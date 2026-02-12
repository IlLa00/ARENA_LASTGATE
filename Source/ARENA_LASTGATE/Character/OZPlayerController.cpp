#include "OZPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "OZPlayer.h"
#include "OZPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Instance/OZGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Subsystem/OZOnlineSessionSubsystem.h"
#include "GameState/OZLobbyWaitGameState.h"
#include "GameState/OZInGameGameState.h"
#include "HUD/OZLobbyWaitHUD.h"
#include "HUD/OZInGameHUD.h"
#include "Utils/Util.h"
#include "Tags/OZGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "Ability/OZFireGun.h"
#include "GameMode/OZInGameMode.h"
#include "GameMode/OzBaseGameMode.h"
#include "Data/OZAbilityInputID.h"
#include "Components/OZCameraAimComponent.h"
#include "OZPlayerAttributeSet.h"
#include "Character/Components/OZVisionComponent.h"
#include "Character/Components/OZInventoryComponent.h"
#include "Interface/OZInteractable.h"
#include "Widget/OZInteractionUI.h"
#include "Components/WidgetComponent.h"
#include "Object/OZTower.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"
#include "Abilities/GameplayAbility.h"
#include "TimerManager.h"
#include "UserInterface/OZLobbyWaitRoomUI.h"
#include "Engine/World.h"
#include "HUD/OZPracticeHUD.h"
#include "HUD/OZLobbyWaitHUD.h"
#include "Shop/OZShopManager.h"
#include "Misc/OutputDeviceNull.h"
#include "Subsystem/OZInGameTutorialPopupSubSystem.h"
#include "Subsystem/OZLoadingSubsystem.h"
#include "LevelSequencePlayer.h"
#include "Instance/OZGameInstance.h"
#include "LevelSequenceActor.h"


AOZPlayerController::AOZPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOZPlayerController::BeginPlay()
{
	Super::BeginPlay();

	IOnlineSessionPtr SessionInterface = IOnlineSubsystem::Get()->GetSessionInterface();

	if (HasAuthority())
	{
		UOZGameInstance* gameInstance = Cast<UOZGameInstance>(GetGameInstance());

		if (gameInstance)
		{
			AOZPlayerState* OzplayerState = GetPlayerState<AOZPlayerState>();
			if (OzplayerState)
			{
				OzplayerState->UserName = gameInstance->PlayerName;
				OzplayerState->UserIdentificationCode = gameInstance->GetSubsystem<UOZOnlineSessionSubsystem>()->GetUserIdentifyCode();
				OzplayerState->SetPlayerName(gameInstance->PlayerName);
			}
		}
	}

	else
	{
		if (ClientReadyState.bBeginPlayCalled == false)
		{
			ClientReadyState.bBeginPlayCalled = true;
			CheckClientReady();
		}

	}

	if (!IsLocalController())
		return;

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

void AOZPlayerController::Tick(float Deltatime)
{
	Super::Tick(Deltatime);

	if (!IsLocalController())
		return;

	APawn* OZPawn = GetPawn();
	if (!OZPawn)
		return;

	if (controllerInputMode == EInputMode::UIOnly)
		return;

	FVector MouseWorldLocation, MouseWorldDirection;
	if (!DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
		return;

	const FVector PawnLocation = OZPawn->GetActorLocation();
	const float PawnZ = PawnLocation.Z;

	const float DistanceToPlane = (PawnZ - MouseWorldLocation.Z) / MouseWorldDirection.Z;
	FVector TargetLocation = MouseWorldLocation + MouseWorldDirection * DistanceToPlane;
	TargetLocation.Z = PawnZ;

	const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(PawnLocation, TargetLocation);
	const FRotator NewRot(0.f, LookAtRot.Yaw, 0.f);

	SetControlRotation(NewRot);

	UpdateMouseEdgeAim();
	UpdateHoldInteraction(Deltatime);
}

void AOZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
		{

			EIC->BindAction(MoveAction, ETriggerEvent::Triggered,
				this, &AOZPlayerController::Move);

		}
		if (FireAction)
		{
			EIC->BindAction(FireAction, ETriggerEvent::Started,
				this, &AOZPlayerController::FireStarted);

			EIC->BindAction(FireAction, ETriggerEvent::Completed,
				this, &AOZPlayerController::FireEnded);

			EIC->BindAction(FireAction, ETriggerEvent::Canceled,
				this, &AOZPlayerController::FireEnded);
		}
		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started,
				this, &AOZPlayerController::DashStarted);
		}
		if (AimingAction)
		{
			EIC->BindAction(AimingAction, ETriggerEvent::Started,
				this, &AOZPlayerController::OnAimStarted);

			EIC->BindAction(AimingAction, ETriggerEvent::Completed,
				this, &AOZPlayerController::OnAimCompleted);
		}

		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started,
				this, &AOZPlayerController::OnAbilityPressed, (int32)EOZAbilityInputID::Sprint);

			EIC->BindAction(SprintAction, ETriggerEvent::Completed,
				this, &AOZPlayerController::OnAbilityReleased, (int32)EOZAbilityInputID::Sprint);
		}
		if (OpenShopAction)
		{
			EIC->BindAction(OpenShopAction, ETriggerEvent::Started,
				this, &AOZPlayerController::ShopOpenPressedAndUp);
		}
		if (Interaction)
		{
			EIC->BindAction(Interaction, ETriggerEvent::Started,
				this, &AOZPlayerController::InteractionStarted);
		}
		if (UseItem_1_Action)
		{
			EIC->BindAction(UseItem_1_Action, ETriggerEvent::Started,
				this, &AOZPlayerController::Use_1_Item);
		}
		if (UseItem_2_Action)
		{
			EIC->BindAction(UseItem_2_Action, ETriggerEvent::Started,
				this, &AOZPlayerController::Use_2_Item);
		}
		if (UseItem_3_Action)
		{
			EIC->BindAction(UseItem_3_Action, ETriggerEvent::Started,
				this, &AOZPlayerController::Use_3_Item);
		}
		if (UseItem_4_Action)
		{
			EIC->BindAction(UseItem_4_Action, ETriggerEvent::Started,
				this, &AOZPlayerController::Use_4_Item);
		}
		if (KeyboardTabAction)
		{
			EIC->BindAction(KeyboardTabAction, ETriggerEvent::Started,
				this, &AOZPlayerController::ToggleMinimap);
		}
		if (KeyboardEnterAction)
		{
			EIC->BindAction(KeyboardEnterAction, ETriggerEvent::Started,
				this, &AOZPlayerController::PressEnter);
		}
		if (KeyboardESCAction)
		{
			EIC->BindAction(KeyboardESCAction, ETriggerEvent::Started,
				this, &AOZPlayerController::PressESC);
		}
	}

}

void AOZPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AOZPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (HasAuthority())
		return;

	UOZGameInstance* gameInstance = Cast<UOZGameInstance>(GetGameInstance());

	if (gameInstance)
	{
		AOZPlayerState* OzplayerState = GetPlayerState<AOZPlayerState>();
		if (OzplayerState)
		{
			FString name = gameInstance->PlayerName;
			FString Id = gameInstance->GetSubsystem<UOZOnlineSessionSubsystem>()->GetUserIdentifyCode();
			Server_SetPlayerInfo(name, Id);
		}
	}

	if (ClientReadyState.bPlayerStateReplicated == false)
	{
		ClientReadyState.bPlayerStateReplicated = true;
		CheckClientReady();
	}
}

void AOZPlayerController::OnUINativeConstructClear()
{
	bUIReady = true;

	if (HasAuthority())
	{
		if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
		{
			gameState->RegisterPlayer(this->GetPlayerState<AOZPlayerState>());

			if (AOZLobbyWaitHUD* HUD = GetHUD<AOZLobbyWaitHUD>())
			{
				HUD->BindGameState(gameState);

				SendPlayerEnrollMsg();

				HUD->RefreshFromGameState();
			}



		}
	}

	else
	{
		if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
		{
			TryEnrollplayerStateAtLobbyWait();
		}


		if (ClientReadyState.bUIConstructed == false)
		{
			ClientReadyState.bUIConstructed = true;
			CheckClientReady();
		}

	}
}

void AOZPlayerController::TryEnrollplayerStateAtLobbyWait()
{
	AOZPlayerState* OZPlayerState = this->GetPlayerState<AOZPlayerState>();

	if (OZPlayerState == nullptr)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AOZPlayerController::TryEnrollplayerStateAtLobbyWait);
		return;
	}


	else
	{
		Server_RequestEnroll(this->GetPlayerState<AOZPlayerState>());

		if (AOZLobbyWaitHUD* HUD = GetHUD<AOZLobbyWaitHUD>())
		{
			HUD->BindGameState(Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()));

			SendPlayerEnrollMsg();
		}
	}
}

void AOZPlayerController::Server_ReportcombatResultRecived_Implementation()
{
	if (AOZInGameMode* GM = GetWorld()->GetAuthGameMode<AOZInGameMode>())
	{
		GM->RecievedCombatResultRep();
	}
}

void AOZPlayerController::OnAbilityPressed(const FInputActionValue& Value, int32 InputID)
{
	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputPressed(InputID);
		}
	}
}

void AOZPlayerController::OnAbilityReleased(const FInputActionValue& Value, int32 InputID)
{
	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputReleased(InputID);
		}
	}
}

void AOZPlayerController::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	float MovementVectorSize = 1.0f;
	float MovementVectorSizeSquared = MovementVector.SquaredLength();

	if (MovementVectorSizeSquared > 1.0f)
	{
		MovementVectorSizeSquared = 1.0f;
		MovementVector.Normalize();
	}

	FVector MoveDirection = FVector{ MovementVector.Y, MovementVector.X, 0.0f };

	APawn* MyPawn = GetPawn();

	if (MyPawn)
	{
		MyPawn->AddMovementInput(MoveDirection, MovementVectorSizeSquared);
	}

}

void AOZPlayerController::FireStarted(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	if (controllerInputMode == EInputMode::UIOnly)
		return;

	if (bIsHoldingFire)
		return;

	if (bBlockFireByUI)
		return;

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer)
		return;

	UAbilitySystemComponent* ASC = OZPlayer->GetAbilitySystemComponent();
	if (!ASC)
		return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(OZGameplayTags::Player_Ability_Fire);
	ASC->TryActivateAbilitiesByTag(TagContainer);

	bIsHoldingFire = true;

	GetWorldTimerManager().SetTimer(
		AutoFireTimerHandle,
		this,
		&AOZPlayerController::AutoFireTick,
		AutoFirePollInterval,
		true,
		AutoFirePollInterval
	);
}

void AOZPlayerController::FireEnded(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	bIsHoldingFire = false;

	if (GetWorldTimerManager().IsTimerActive(AutoFireTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}

void AOZPlayerController::AutoFireTick()
{
	if (!bIsHoldingFire)
	{
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
		return;
	}

	if (controllerInputMode == EInputMode::UIOnly)
		return;

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer)
	{
		bIsHoldingFire = false;
		GetWorldTimerManager().ClearTimer(AutoFireTimerHandle);
		return;
	}

	UAbilitySystemComponent* ASC = OZPlayer->GetAbilitySystemComponent();
	if (!ASC)
		return;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(OZGameplayTags::Player_Ability_Fire);
	ASC->TryActivateAbilitiesByTag(TagContainer);
}

void AOZPlayerController::SendPlayerEnrollMsg()
{
	UOZGameInstance* ozGameInstance = Cast<UOZGameInstance>(GetWorld()->GetGameInstance());
	FString EnrollMessage = FString::Printf(TEXT(" \"%s\" In Engage"), *ozGameInstance->PlayerName);

	Call_SendChattingMsg(EnrollMessage, true);
}

void AOZPlayerController::DashStarted(const FInputActionValue& Value)
{
	AOZPlayerState* PS = GetPlayerState<AOZPlayerState>();
	if (PS)
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputPressed((int32)EOZAbilityInputID::Dash);
		}
	}
}
void AOZPlayerController::ShopOpenPressedAndUp(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	AHUD* hud = GetHUD();

	if (AOZInGameHUD* inGameHUD = Cast<AOZInGameHUD>(hud))
	{
		AOZInGameGameState* InGameState = GetWorld()->GetGameState<AOZInGameGameState>();

		if (InGameState == nullptr)
			return;

		if (InGameState->CurrentRound == 1 && !bFirstShopOpenTriggered)
		{
			bFirstShopOpenTriggered = true;
			if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
			{
				TutorialSubsystem->CallTutorialPopupByID(3, 5.0f);
			}
		}

		if (InGameState->CurrentRoundState == EGameStateType::STATE_PREP)
		{
			inGameHUD->ToggleShopUI();
		}
	}

	else if (AOZPracticeHUD* practiceHUD = Cast<AOZPracticeHUD>(hud))
	{
		practiceHUD->ToggleCustomConverterUI();
	}
}

void AOZPlayerController::OnAimStarted(const FInputActionInstance& Instance)
{
	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer) return;

	OZPlayer->VisionComp->OnAiming();

	AimStartedInternal();

	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputReleased((int32)EOZAbilityInputID::Sprint);
		}
	}

	OnAbilityPressed(Instance.GetValue(), (int32)EOZAbilityInputID::Aiming);
}

void AOZPlayerController::OnAimCompleted(const FInputActionInstance& Instance)
{
	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer) return;

	OZPlayer->VisionComp->OffAiming();

	AimCompletedInternal();
	OnAbilityReleased(Instance.GetValue(), (int32)EOZAbilityInputID::Aiming);
}

void AOZPlayerController::AimStartedInternal()
{
	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer) return;

	if (OZPlayer->CameraAimComp)
	{
		OZPlayer->CameraAimComp->StartAim();
	}
}

void AOZPlayerController::AimCompletedInternal()
{
	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer) return;

	if (OZPlayer->CameraAimComp)
	{
		OZPlayer->CameraAimComp->StopAim();
	}
}

void AOZPlayerController::Use_1_Item()
{
	SendBattleItemThrowDistanceToServer(10000.f);
	Server_UseItemAtSlot(0);
}

void AOZPlayerController::Use_2_Item()
{
	SendBattleItemThrowDistanceToServer(10000.f);
	Server_UseItemAtSlot(1);
}

void AOZPlayerController::Use_3_Item()
{
	SendBattleItemThrowDistanceToServer(10000.f);
	Server_UseItemAtSlot(2);
}

void AOZPlayerController::Use_4_Item()
{
	SendBattleItemThrowDistanceToServer(10000.f);
	Server_UseItemAtSlot(3);
}

void AOZPlayerController::ToggleMinimap(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	AHUD* hud = GetHUD();

	if (AOZInGameHUD* inGameHUD = Cast<AOZInGameHUD>(hud))
	{
		AOZInGameGameState* InGameState = GetWorld()->GetGameState<AOZInGameGameState>();

		if (InGameState == nullptr)
			return;

		if (InGameState->CurrentRoundState == EGameStateType::STATE_COMBAT)
		{
			inGameHUD->ToggleMinimapUI();
		}
	}
}

void AOZPlayerController::PressEnter(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	AHUD* hud = GetHUD();

	if (AOZLobbyWaitHUD* lobbyWait = Cast<AOZLobbyWaitHUD>(hud))
	{
		lobbyWait->OnKeyboardEnterPressed();
	}
}

void AOZPlayerController::PressESC(const FInputActionValue& Value)
{
	if (!IsLocalController())
		return;

	AHUD* hud = GetHUD();

	if (AOZLobbyWaitHUD* lobbyWait = Cast<AOZLobbyWaitHUD>(hud))
	{
		lobbyWait->OnKeyboardESCPressed();
	}
}

void AOZPlayerController::Server_UseItemAtSlot_Implementation(int32 SlotIndex)
{
	AOZPlayerState* PS = GetPlayerState<AOZPlayerState>();
	if (!PS || !PS->InventoryComp)
		return;

	FOZInventorySlot Slot = PS->InventoryComp->GetSlot(SlotIndex);
	if (Slot.IsEmpty())
		return;

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
		return;

	UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
	if (!ItemSubsystem)
		return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
		return;

	TSoftClassPtr<UGameplayAbility> SoftAbilityClass = nullptr;

	if (Slot.ItemType == EOZItemType::Battle)
	{
		if (FOZBattleItemData* ItemData = ItemSubsystem->GetBattleItemData(Slot.ItemID))
		{
			SoftAbilityClass = ItemData->ItemAbilityClass;
		}
	}
	else if (Slot.ItemType == EOZItemType::Buff)
	{
		if (FOZBuffItemData* ItemData = ItemSubsystem->GetBuffItemData(Slot.ItemID))
		{
			SoftAbilityClass = ItemData->ItemAbilityClass;
		}
	}

	if (!SoftAbilityClass.IsNull())
	{
		TSubclassOf<UGameplayAbility> AbilityClass = SoftAbilityClass.LoadSynchronous();

		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);

			if (SpecHandle.IsValid())
			{
				bool bActivated = ASC->TryActivateAbility(SpecHandle);

				if (bActivated)
				{
					PS->InventoryComp->RemoveItem(SlotIndex, 1);

					// 어빌리티 종료 시 ClearAbility 호출
					FDelegateHandle DelegateHandle;
					TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
					FGameplayAbilitySpecHandle CapturedHandle = SpecHandle;

					DelegateHandle = ASC->OnAbilityEnded.AddLambda(
						[WeakASC, CapturedHandle, DelegateHandle](const FAbilityEndedData& EndedData) mutable
						{
							if (EndedData.AbilitySpecHandle == CapturedHandle)
							{
								if (WeakASC.IsValid())
								{
									WeakASC->OnAbilityEnded.Remove(DelegateHandle);
									WeakASC->ClearAbility(CapturedHandle);
								}
							}
						});
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to activate item ability at slot %d"), SlotIndex);
					ASC->ClearAbility(SpecHandle);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to give ability for item at slot %d"), SlotIndex);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load ability class from path: %s"), *SoftAbilityClass.ToString());
		}
	}
}

UOZInteractionUI* GetInteractionUIFromActor(AActor* Actor)
{
	if (!Actor)
		return nullptr;

	if (UWidgetComponent* WidgetComp = Actor->FindComponentByClass<UWidgetComponent>())
	{
		return Cast<UOZInteractionUI>(WidgetComp->GetUserWidgetObject());
	}
	return nullptr;
}

void AOZPlayerController::InteractionStarted()
{
	if (bIsHoldingInteraction)
		return;

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer)
		return;

	AActor* ClosestActor = OZPlayer->GetClosestInteractableActor();

	if (!ClosestActor || !ClosestActor->Implements<UOZInteractable>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerController] No interactable actor in range"));
		return;
	}

	// 타워인 경우 2초 게이지 후 발동
	if (ClosestActor->IsA<AOZTower>())
	{
		bIsHoldingInteraction = true;
		InteractionHoldTime = 0.f;
		CurrentInteractionTarget = ClosestActor;

		if (UOZInteractionUI* InteractionUI = GetInteractionUIFromActor(ClosestActor))
		{
			InteractionUI->SetGaugePercent(0.f);
		}

		if (TowerInteractionStartSound)
		{
			TowerInteractionAudioComp = UGameplayStatics::SpawnSound2D(this, TowerInteractionStartSound);

			// 2초 후 사운드 정지
			GetWorldTimerManager().SetTimer(
				TowerInteractionSoundTimerHandle,
				[this]()
				{
					if (TowerInteractionAudioComp)
					{
						TowerInteractionAudioComp->Stop();
						TowerInteractionAudioComp = nullptr;
					}
				},
				2.0f,
				false
			);
		}
	}
	// 그 외 즉시 발동
	else
	{
		IOZInteractable::Execute_OnInteract(ClosestActor, OZPlayer);
	}
}

void AOZPlayerController::InteractionCompleted()
{
	if (!bIsHoldingInteraction)
		return;

	// 게이지 리셋
	if (CurrentInteractionTarget.IsValid())
	{
		if (UOZInteractionUI* InteractionUI = GetInteractionUIFromActor(CurrentInteractionTarget.Get()))
		{
			InteractionUI->SetGaugePercent(0.f);
		}
	}

	// 타워 상호작용 사운드 정지
	GetWorldTimerManager().ClearTimer(TowerInteractionSoundTimerHandle);
	if (TowerInteractionAudioComp)
	{
		TowerInteractionAudioComp->Stop();
		TowerInteractionAudioComp = nullptr;
	}

	bIsHoldingInteraction = false;
	InteractionHoldTime = 0.f;
	CurrentInteractionTarget = nullptr;
}

void AOZPlayerController::UpdateHoldInteraction(float DeltaTime)
{
	if (!bIsHoldingInteraction)
		return;

	// 타겟이 유효하지 않으면 상호작용 취소
	if (!CurrentInteractionTarget.IsValid())
	{
		InteractionCompleted();
		return;
	}

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer)
	{
		InteractionCompleted();
		return;
	}

	// 타겟이 여전히 범위 내에 있는지 확인
	AActor* ClosestActor = OZPlayer->GetClosestInteractableActor();
	if (ClosestActor != CurrentInteractionTarget.Get())
	{
		// 타겟이 바뀌었거나 범위를 벗어남
		InteractionCompleted();
		return;
	}

	// 시간 누적
	InteractionHoldTime += DeltaTime;

	// 게이지 퍼센트 계산 및 UI 업데이트
	float Percent = FMath::Clamp(InteractionHoldTime / InteractionRequiredTime, 0.f, 1.f);

	if (UOZInteractionUI* InteractionUI = GetInteractionUIFromActor(CurrentInteractionTarget.Get()))
	{
		InteractionUI->SetGaugePercent(Percent);
	}

	// 게이지가 꽉 차면 상호작용 실행
	if (InteractionHoldTime >= InteractionRequiredTime)
	{
		AActor* Target = CurrentInteractionTarget.Get();

		// 상호작용 상태 초기화 (OnInteract 호출 전에 리셋)
		bIsHoldingInteraction = false;
		InteractionHoldTime = 0.f;
		CurrentInteractionTarget = nullptr;

		// 상호작용 실행
		if (Target && Target->Implements<UOZInteractable>())
		{
			IOZInteractable::Execute_OnInteract(Target, OZPlayer);
		}

		// UI 게이지 리셋
		if (UOZInteractionUI* InteractionUI = GetInteractionUIFromActor(Target))
		{
			InteractionUI->SetGaugePercent(0.f);
		}
	}
}

void AOZPlayerController::InitializeInGameMinimap(FOZWorldBoundInfo worldBound)
{
	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();

	if (ozIngameHUD == nullptr)
		return;

	if (ozIngameHUD->InGameUIArray.IsEmpty())
	{
		FTimerDelegate WaitForGameUIDelegate;
		WaitForGameUIDelegate.BindUObject(
			this,
			&AOZPlayerController::InitializeInGameMinimap,
			worldBound
		);

		GetWorld()->GetTimerManager().SetTimerForNextTick(WaitForGameUIDelegate);
	}

	else
	{
		ozIngameHUD->InitCombatMinimap(worldBound.Center, worldBound.Size);

	}


}

void AOZPlayerController::StartDarkSmogeShrink_Implementation(float shrinkingTime)
{
	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();

	if (ozIngameHUD == nullptr)
		return;

	ozIngameHUD->ShowBlackSmogeWarning();
	ozIngameHUD->SetBlackSmogeShrinking(shrinkingTime);

	PlayFogShrinkStartSfxRepeated();
}

void AOZPlayerController::ShowKillLog_Implementation(const FString& KillerName, const FString& DeadName, int numSurvivalPlayers)
{
	AOZPlayer* OZPlayerCharacter = Cast<AOZPlayer>(GetCharacter());

	if (OZPlayerCharacter == nullptr)
		return;

	if (OZPlayerCharacter->GetIsPlayerDead() == true)
		return;

	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();

	if (ozIngameHUD == nullptr)
		return;

	ozIngameHUD->ShowCombatKillLog(KillerName, DeadName, numSurvivalPlayers);

}

void AOZPlayerController::UpdateMouseEdgeAim()
{
	if (!IsLocalController())
		return;

	float MouseX = 0.f, MouseY = 0.f;
	if (!GetMousePosition(MouseX, MouseY))
	{
		if (bEdgeAimActive)
		{
			bEdgeAimActive = false;
			AimCompletedInternal();
		}
		return;
	}

	int32 ViewSizeX = 0, ViewSizeY = 0;
	GetViewportSize(ViewSizeX, ViewSizeY);

	const float Left = EdgeBorderThickness;
	const float Right = ViewSizeX - EdgeBorderThickness;
	const float Top = EdgeBorderThickness;
	const float Bottom = ViewSizeY - EdgeBorderThickness;

	const bool bIsInEdgeRegion =
		(MouseX <= Left) ||
		(MouseX >= Right) ||
		(MouseY <= Top) ||
		(MouseY >= Bottom);

	if (bIsInEdgeRegion && !bEdgeAimActive)
	{
		bEdgeAimActive = true;
		AimStartedInternal();
	}
	else if (!bIsInEdgeRegion && bEdgeAimActive)
	{
		bEdgeAimActive = false;
		AimCompletedInternal();
	}
}

void AOZPlayerController::Server_SetPlayerInfo_Implementation(const FString& name, const FString& ID)
{
	UOZGameInstance* gameInstance = Cast<UOZGameInstance>(GetGameInstance());

	if (gameInstance)
	{
		AOZPlayerState* OzplayerState = GetPlayerState<AOZPlayerState>();
		if (OzplayerState)
		{
			OzplayerState->UserName = name;
			OzplayerState->UserIdentificationCode = ID;
			OzplayerState->SetPlayerName(name);
		}
	}
}

void AOZPlayerController::CheckClientReady()
{
	if (ClientReadyState.IsReady())
	{
		if (HasAuthority())
			return;

		Server_ReportClientReady();
	}
}

void AOZPlayerController::ShowCombatResultPopup_Implementation(bool bIsWin)
{
	AOZPlayer* OZPlayerCharacter = Cast<AOZPlayer>(GetCharacter());

	if (OZPlayerCharacter == nullptr)
		return;

	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();

	ozIngameHUD->ShowCombatResultPopupWidget(bIsWin);
}

int AOZPlayerController::GetIsPlayerReadyState()
{
	if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
	{
		int playerReadyState = gameState->GetIsPlayerReady(GetPlayerState<AOZPlayerState>());

		if (playerReadyState == 1 || playerReadyState == -1)
			return playerReadyState;
	}

	return 0;
}

void AOZPlayerController::ChangeLobbyWaitState(TObjectPtr<class UOZLobbyWaitRoomUI> lobbyWaitRoomUI)
{
	if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
	{
		int assingedIndex = gameState->GetAssingedIndex(GetPlayerState<AOZPlayerState>());

		if (assingedIndex == -1)
			return;

		int bIsReadyState = lobbyWaitRoomUI->ChangeUserInfoReadyState(assingedIndex);

		switch (bIsReadyState)
		{
		case 1:
			Server_RequestChangePlayerState(assingedIndex, true);
			break;

		case -1:
			Server_RequestChangePlayerState(assingedIndex, false);
			break;

		case 0:
			break;

		default:
			break;
		}
	}
}

void AOZPlayerController::Server_RequestChangePlayerState_Implementation(int playerIndex, bool bIsReadyState)
{
	if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
	{
		gameState->ChangePlayerReadyState(playerIndex, bIsReadyState);
	}
}

void AOZPlayerController::Server_RequestEnroll_Implementation(AOZPlayerState* NewPlayerState)
{
	if (AOZLobbyWaitGameState* gameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState()))
	{
		gameState->RegisterPlayer(NewPlayerState);
		gameState->OnLobbyWaitSlotChanged.Broadcast();
	}
}

void AOZPlayerController::Server_ReportClientReady_Implementation()
{
	if (!HasAuthority())
		return;

	if (AOzBaseGameMode* OzGameMode = Cast< AOzBaseGameMode>(GetWorld()->GetAuthGameMode()))
	{
		OzGameMode->AddPlayerReady(this);
	}
}

void AOZPlayerController::Server_PurchaseItem_Implementation(int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
	if (!HasAuthority())
		return;

	AOZInGameGameState* GameState = GetWorld()->GetGameState<AOZInGameGameState>();
	if (!GameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_PurchaseItem: GameState is NULL!"));
		return;
	}

	AOZShopManager* ShopManager = GameState->GetShopManager();
	if (!ShopManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_PurchaseItem: ShopManager is NULL!"));
		return;
	}

	AOZPlayerState* OZPlayerState = GetPlayerState<AOZPlayerState>();
	if (!OZPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_PurchaseItem: PlayerState is NULL!"));
		return;
	}

	ShopManager->Server_PurchaseItem(OZPlayerState, ItemID, ItemType, Quantity);
}

void AOZPlayerController::Server_SellItem_Implementation(int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
	if (!HasAuthority())
		return;

	AOZInGameGameState* GameState = GetWorld()->GetGameState<AOZInGameGameState>();
	if (!GameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_SellItem: GameState is NULL!"));
		return;
	}

	AOZShopManager* ShopManager = GameState->GetShopManager();
	if (!ShopManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_SellItem: ShopManager is NULL!"));
		return;
	}

	AOZPlayerState* OZPlayerState = GetPlayerState<AOZPlayerState>();
	if (!OZPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController] Server_SellItem: PlayerState is NULL!"));
		return;
	}

	ShopManager->Server_SellItem(OZPlayerState, ItemID, ItemType, Quantity);
}

void AOZPlayerController::SendBattleItemThrowDistanceToServer(float MaxRangeCm, float FallbackDistanceCm)
{
	if (!IsLocalController())
		return;

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
		return;

	const FVector Start = MyPawn->GetActorLocation();

	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		Hit
	);

	FVector Point;
	if (bHit)
	{
		Point = Hit.ImpactPoint;
	}
	else
	{
		FVector CamLoc;
		FRotator CamRot;
		GetPlayerViewPoint(CamLoc, CamRot);
		Point = CamLoc + CamRot.Vector() * FallbackDistanceCm;
	}

	FVector Delta = Point - Start;
	float Dist2D = FVector(Delta.X, Delta.Y, 0.f).Size();

	float Clamped = FMath::Clamp(Dist2D, 0.f, MaxRangeCm);

	Server_SetBattleItemThrowDistance(Clamped);
}

void AOZPlayerController::ReportSessionExit_Implementation(AOZPlayerController* exitPlayercontroller)
{
	if (HasAuthority() == false)
		return;

	AOZLobbyWaitGameState* ozLobbyWaitGameSTate = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState());

	if (ozLobbyWaitGameSTate == nullptr)
		return;

	ozLobbyWaitGameSTate->UnregisterPlayer(GetPlayerState<AOZPlayerState>());
}

void AOZPlayerController::ReportRoundCombatEnd_Implementation()
{
	if (AGameModeBase* gameMode = this->GetWorld()->GetAuthGameMode())
	{
		AOZInGameMode* ozInGameMode = Cast<AOZInGameMode>(gameMode);

		if (ozInGameMode == nullptr)
			return;

		ozInGameMode->RoundCombatEnd();
	}
}

void AOZPlayerController::Server_SetBattleItemThrowDistance_Implementation(float InDistanceCm)
{
	LastBattleItemThrowDistanceCm = FMath::Max(0.f, InDistanceCm);
	LastBattleItemThrowTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void AOZPlayerController::Client_ForceCloseSlotUIAndReturnCamera_Implementation()
{
	TArray<UUserWidget*> Widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UUserWidget::StaticClass(), false);

	for (UUserWidget* W : Widgets)
	{
		if (!W) continue;

		if (W->GetName().Contains(TEXT("WBP_SlotMachine")))
		{
			W->RemoveFromParent();
		}
	}

	// 2) 카메라 복귀
	if (APawn* P = GetPawn())
	{
		SetViewTargetWithBlend(P, 0.2f);
	}

	AOZPlayer* OZPlayerCharacter = Cast<AOZPlayer>(GetCharacter());

	OZPlayerCharacter->ExitSlotMachineView();
}

void AOZPlayerController::PlayFogShrinkStartSfxOnce()
{
	if (!FogShrinkStartSfx)
		return;

	UGameplayStatics::PlaySound2D(this, FogShrinkStartSfx);

	FogSfxRepeatCount++;

	/*if (FogSfxRepeatCount >= 3)
	{
		
	}*/

	GetWorldTimerManager().ClearTimer(FogSfxRepeatTimer);
}

void AOZPlayerController::PlayFogShrinkStartSfxRepeated()
{
	if (!FogShrinkStartSfx)
		return;

	FogSfxRepeatCount = 0;

	// 첫 1회 즉시 재생
	PlayFogShrinkStartSfxOnce();

	//// 이후 타이머로 반복
	//GetWorldTimerManager().SetTimer(
	//	FogSfxRepeatTimer,
	//	this,
	//	&AOZPlayerController::PlayFogShrinkStartSfxOnce,
	//	1.5f,   // 반복 간격(원하는 값으로 조절)
	//	true
	//);
}

void AOZPlayerController::ReleaseAbilityInput(int32 InputID)
{
	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityLocalInputReleased(InputID);
		}
	}
}

void AOZPlayerController::Call_SendChattingMsg(const FString& Message, bool bIsServerMSG)
{
	if (Message.IsEmpty())
		return;

	FChatMessage ChatiingMessage;

	if (bIsServerMSG)
	{
		ChatiingMessage.Sender = FString::Printf(TEXT("System"));
	}

	else
	{
		ChatiingMessage.Sender = GetPlayerState<AOZPlayerState>()->UserName;
	}

	ChatiingMessage.Message = Message;

	Server_SendChat(ChatiingMessage);
}

void AOZPlayerController::OnTowerActivationChanged_Implementation(bool bIsActivated)
{
	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();

	if (ozIngameHUD == nullptr)
		return;

	ozIngameHUD->SetActivateTowerVision(bIsActivated);

	// 처음 타워 활성화 시 튜토리얼 팝업 호출
	if (bIsActivated && !bFirstTowerActivationTriggered)
	{
		bFirstTowerActivationTriggered = true;
		if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
		{
			TutorialSubsystem->CallTutorialPopupByID(10, 5.0f);
		}
	}
}

void AOZPlayerController::Server_SendChat_Implementation(FChatMessage ChattingMSGInfo)
{
	Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState())->AddChatMessage(ChattingMSGInfo);
}

void AOZPlayerController::Server_ReportCombatBonusRewardPopupEnd_Implementation()
{
	if (HasAuthority() == false)
		return;


	AOZInGameMode* inGameMode = Cast< AOZInGameMode>(GetWorld()->GetAuthGameMode());

	if (inGameMode == nullptr)
		return;

	inGameMode->RevievedCombatPopupEnd();
}

void AOZPlayerController::OnRoundStart_Implementation()
{
	if (!IsLocalController())
		return;

	ReleaseAbilityInput(3);

	Server_UpdateCharacterHealthWidget();

	// 1라운드 첫 정비시간에 튜토리얼 팝업 호출
	AOZInGameGameState* InGameState = GetWorld()->GetGameState<AOZInGameGameState>();
	if (InGameState && InGameState->CurrentRound == 1)
	{
		if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
		{
			TutorialSubsystem->CallTutorialPopupByID(2, 5.0f);
		}
	}
}

void AOZPlayerController::Server_ActivateTower_Implementation(AOZTower* Tower)
{
	if (!Tower)
		return;

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(GetPawn());
	if (!OZPlayer)
		return;

	Tower->ActivateTower(OZPlayer);
}

void AOZPlayerController::Server_UpdateCharacterHealthWidget_Implementation()
{
	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		float maxShieldAmount = PS->PlayerAttributes->GetMaxShield();
		float maxHPAmount = PS->PlayerAttributes->GetMaxHealth();

		APawn* pawn = GetPawn();
		if (pawn == nullptr)
			return;

		AOZPlayer* ozPlayerCharacter = Cast<AOZPlayer>(pawn);
		if (ozPlayerCharacter == nullptr)
			return;

		ozPlayerCharacter->BroadCastUpdateCharacterSheildHPRatio(maxShieldAmount, maxHPAmount);
	}
}

void AOZPlayerController::Client_PlayOpeningCutscene_Implementation()
{
	if (!IsLocalController())
		return;

	// 컷신 재생 전 로딩 화면이 남아있으면 제거 (안전장치)
	if (UOZLoadingSubsystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UOZLoadingSubsystem>())
	{
		LoadingSubsystem->EraseLoadingScene();
	}

	// 1) 시퀀서 액터 찾기
	ALevelSequenceActor* SequenceActor =
		Cast<ALevelSequenceActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ALevelSequenceActor::StaticClass()));

	if (!SequenceActor || !SequenceActor->GetSequencePlayer())
	{
		// 시퀀스가 없으면 그냥 완료 보고
		Server_ReportCutsceneFinished();

		Cast<UOZGameInstance>(GetGameInstance())->PrintLog(TEXT("ReportCutSceneFinished"), FColor::Green, 0.5f);
		return;
	}

	ULevelSequencePlayer* SeqPlayer = SequenceActor->GetSequencePlayer();

	// 2) 중복 바인딩 방지
	SeqPlayer->OnFinished.RemoveDynamic(this, &AOZPlayerController::OnOpeningCutsceneFinished_Local);
	SeqPlayer->OnFinished.AddDynamic(this, &AOZPlayerController::OnOpeningCutsceneFinished_Local);

	// 3) 입력 잠금
	//SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = false; 

	// 4) 재생
	SeqPlayer->Play();

	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();
	if (ozIngameHUD == nullptr)
		return;

	ozIngameHUD->SetIsOnCutScene(true);

	if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
	{
		TutorialSubsystem->CallTutorialPopupByID(1, 10.0f);
	}


	Cast<UOZGameInstance>(GetGameInstance())->PrintLog(TEXT("Received_PlayOpeningCutscene"), FColor::Green, 0.5f);
}

void AOZPlayerController::Server_ReportCutsceneFinished_Implementation()
{
	if (AOZInGameMode* GM = Cast<AOZInGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->NotifyPlayerFinishedCutscene(this);
	}
}

void AOZPlayerController::Client_RecievedCommonTutorialPopupCall_Implementation(int index)
{
	if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
	{
		TutorialSubsystem->CallTutorialPopupByID(index, 5.0f);
	}
}

void AOZPlayerController::OnOpeningCutsceneFinished_Local()
{
	FInputModeGameOnly Mode;
	//SetInputMode(Mode);
	bShowMouseCursor = false;

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);

	// 서버에 보고
	Server_ReportCutsceneFinished();

	AOZInGameHUD* ozIngameHUD = GetHUD<AOZInGameHUD>();
	if (ozIngameHUD == nullptr)
		return;

	ozIngameHUD->SetIsOnCutScene(false);
}

void AOZPlayerController::Client_PlayMineInstallSound_Implementation(USoundBase* Sound, FVector Location)
{
	if (!Sound)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
}
