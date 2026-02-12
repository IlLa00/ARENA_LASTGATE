#include "OZPlayer.h"
#include "OZPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Character/Components/OZVisionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OZPlayerAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "Weapon/OZWeaponAttributeSet.h"
#include "Widget/OZWidgetComponent.h"
#include "Widget/OZPlayerStatusBarWidget.h"
#include "UserInterface/OZRoundInComBatUI.h"
#include "GameMode/OZInGameMode.h"
#include "HUD/OZInGameHUD.h"
#include "Character/OZPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/Weapon/OZBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Tags/OZGameplayTags.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Object/OZPuppet.h"
#include "Interface/OZInteractable.h"
#include "Widget/OZInteractionUI.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "OZAnimInstance.h"
#include "Object/OZTower.h"
#include "Subsystem/OZInGameTutorialPopupSubSystem.h"

AOZPlayer::AOZPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->bUseControllerDesiredRotation = true;
		MoveComp->RotationRate = FRotator(0.f, 1000.f, 0.f);
	}

	VisionComp = CreateDefaultSubobject<UOZVisionComponent>(TEXT("VisionComponent"));
	CameraAimComp = CreateDefaultSubobject<UOZCameraAimComponent>(TEXT("CameraAimComp"));

	InteractionRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionRangeSphere"));
	InteractionRangeSphere->SetupAttachment(RootComponent);
	InteractionRangeSphere->SetSphereRadius(InteractionRange);
	InteractionRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionRangeSphere->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	InteractionRangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	InteractionRangeSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECollisionResponse::ECR_Overlap); // Interactable channel
	InteractionRangeSphere->SetGenerateOverlapEvents(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);

	static ConstructorHelpers::FClassFinder <UOZPlayerStatusBarWidget> UserStatusBarWidget(TEXT("/Game/UI/Widget/WBP_PlayerStatusBarWidget.WBP_PlayerStatusBarWidget_C"));
	StatusWidgetComp = CreateDefaultSubobject<UOZWidgetComponent>(TEXT("StatusWidgetComponent"));
	if (StatusWidgetComp && UserStatusBarWidget.Class != nullptr)
	{
		StatusWidgetComp->SetupAttachment(RootComponent);
		StatusWidgetComp->SetRelativeLocation(FVector(0, 0, 220));
		StatusWidgetComp->SetWidgetClass(UserStatusBarWidget.Class);
		StatusWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
		StatusWidgetComp->SetDrawSize(FVector2D(200, 70));
		StatusWidgetComp->SetDrawAtDesiredSize(false);
	}

	WeaponMesh->SetIsReplicated(true);

	SoundComp = CreateDefaultSubobject<UOZPlayerSoundComponent>(TEXT("SoundComp"));

	// Shield Effect Component
	ShieldEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ShieldEffectComponent"));
	ShieldEffectComponent->SetupAttachment(RootComponent);
	ShieldEffectComponent->SetAutoActivate(false);
}

UAbilitySystemComponent* AOZPlayer::GetAbilitySystemComponent() const
{
	if (AOZPlayerState* OZPlayerState = GetPlayerState<AOZPlayerState>())
	{
		return OZPlayerState->GetAbilitySystemComponent();
	}
	return nullptr;
}

void AOZPlayer::BeginPlay()
{
	Super::BeginPlay();

	InitASCAndAttributes();

	if (IsLocallyControlled())
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
		}

		if (!ScreenEffectComponent)
		{
			TArray<UNiagaraComponent*> NiagaraComponents;
			GetComponents<UNiagaraComponent>(NiagaraComponents);
			for (UNiagaraComponent* Comp : NiagaraComponents)
			{
				if (Comp && Comp != ShieldEffectComponent)
				{
					ScreenEffectComponent = Comp;
					break;
				}
			}
		}
	}

	if (HasAuthority())
	{
		EVisibilityBasedAnimTickOption AnimOption = GetMesh()->VisibilityBasedAnimTickOption;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	if (InteractionRangeSphere)
	{
		InteractionRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AOZPlayer::OnInteractionRangeBeginOverlap);
		InteractionRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AOZPlayer::OnInteractionRangeEndOverlap);
	}

	if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(
				UOZPlayerAttributeSet::GetMoveSpeedAttribute()
			).AddUObject(this, &AOZPlayer::OnMoveSpeedChanged);

			PS->SetAttributes();
		}
	}

	if (ShieldEffectComponent && ShieldEffectComponent->GetAsset())
	{
		if (AOZPlayerState* PS = GetPlayerState<AOZPlayerState>())
		{
			if (PS->PlayerAttributes)
			{
				const bool bHasShield = PS->PlayerAttributes->GetShield() > 0.f;

				if (HasAuthority())
				{
					Multicast_UpdateShieldEffect(bHasShield);
				}
				else
				{
					UpdateShieldEffectLocal(bHasShield);
				}
			}
		}
	}

}

void AOZPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (AActor* Actor : InteractableActorsInRange)
	{
		if (Actor)
		{
			TArray<UPrimitiveComponent*> PrimitiveComponents;
			Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

			for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
			{
				if (PrimComp)
				{
					PrimComp->SetRenderCustomDepth(false);
				}
			}
		}
	}

	InteractableActorsInRange.Empty();

	Super::EndPlay(EndPlayReason);
}

void AOZPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitASCAndAttributes();
}
void AOZPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitASCAndAttributes();
}

void AOZPlayer::OnRep_Controller()
{
	Super::OnRep_Controller();

	InitStaminaAttributesForUI();
}

void AOZPlayer::InitASCAndAttributes()
{
	AOZPlayerState* PS = GetPlayerState<AOZPlayerState>();
	if (!PS) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC) return;

	ASC->InitAbilityActorInfo(PS, this);

	PAS = PS->PlayerAttributes;
	if (!PAS) return;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		const float NewSpeed = PAS->GetMoveSpeed();
		MoveComp->MaxWalkSpeed = NewSpeed;
	}
	if (!bMoveSpeedDelegateBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			PAS->GetMoveSpeedAttribute())
			.AddUObject(this, &AOZPlayer::OnMoveSpeedChanged);

		bMoveSpeedDelegateBound = true;
	}

	if (!bWalkSpeedDelegateBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			PAS->GetWalkSpeedAttribute())
			.AddUObject(this, &AOZPlayer::OnWalkSpeedChanged);

		bWalkSpeedDelegateBound = true;
	}

	if (CameraAimComp)
	{
		const float InitialAimingDistance = PAS->GetAimingDistance();

		if (InitialAimingDistance >= 0.f && InitialAimingDistance != FLT_MIN)
		{
			CameraAimComp->SetAdditionalCameraDistance(InitialAimingDistance);
		}
	}
	if (!bAimingDistanceDelegateBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			PAS->GetAimingDistanceAttribute())
			.AddUObject(this, &AOZPlayer::OnAimingDistanceChanged);

		bAimingDistanceDelegateBound = true;
	}

	if (CameraAimComp)
	{
		const float InitialCameraDepth = PAS->GetCameraDepth();

		if (InitialCameraDepth > 0.f)
		{
			CameraAimComp->SetDefaultTargetArmLength(InitialCameraDepth);
		}
	}
	if (!bCameraDepthDelegateBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			PAS->GetCameraDepthAttribute())
			.AddUObject(this, &AOZPlayer::OnCameraDepthChanged);

		bCameraDepthDelegateBound = true;
	}

	if (!bViewingAngleDelegateBound)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			PAS->GetViewingAngleAttribute())
			.AddUObject(this, &AOZPlayer::OnViewingAngleChanged);

		bViewingAngleDelegateBound = true;
	}

	InitStaminaAttributesForUI();

	// Shield Effect 초기화 (서버에서 Multicast, 클라이언트에서 로컬 업데이트)
	float CurrentShield = PAS->GetShield();
	if (HasAuthority())
	{
		Multicast_UpdateShieldEffect(CurrentShield > 0.f);
	}
	else
	{
		UpdateShieldEffectLocal(CurrentShield > 0.f);
	}
}
void AOZPlayer::InitStaminaAttributesForUI()
{
	AOZPlayerState* PS = GetPlayerState<AOZPlayerState>();
	if (!PS) return;

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC) return;

	if(IsLocallyControlled() == true)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UOZPlayerAttributeSet::GetCurrentStaminaAttribute())
			.AddUObject(this, &AOZPlayer::OnCurrentStaminaChanged);

		/*ASC->GetGameplayAttributeValueChangeDelegate(
			UOZPlayerAttributeSet::GetMaxStaminaAttribute())
			.AddUObject(this, &AOZPlayer::OnMaxStaminaChanged);*/
	}
}
void AOZPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOZPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AOZPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZPlayer, bIsDead);
	DOREPLIFETIME(AOZPlayer, BandanaTexture);
	DOREPLIFETIME(AOZPlayer, BandanaColor);
}

void AOZPlayer::BroadCastUpdateCharacterSheildHPRatio_Implementation(float maxSheild, float maxHP)
{
	UUserWidget* WidgetInstance = StatusWidgetComp->GetWidget();

	if (UOZPlayerStatusBarWidget* statusBarWidget = Cast<UOZPlayerStatusBarWidget>(WidgetInstance))
	{
		/*float sheildSizeAmountRatio = maxSheild / maxHP;
		statusBarWidget->SetSheildAmountRatio(sheildSizeAmountRatio);*/
		statusBarWidget->UpdateSheildVolumnRatio(maxSheild, maxHP);
		
	}
}

void AOZPlayer::SetHideUI(bool bHide)
{
	if (!StatusWidgetComp)
		return;

	UUserWidget* Widget = StatusWidgetComp->GetUserWidgetObject();
	if (!Widget)
		return;

	if (UTextBlock* HideText =
		Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("HiddingText"))))
	{
		HideText->SetVisibility(bHide ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (UImage* HideIcon = Cast<UImage>(Widget->GetWidgetFromName(TEXT("HideIcon"))))
	{
		HideIcon->SetVisibility(bHide ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

}

//void AOZPlayer::OnCharacterMaxSheildChanged_Implementation(float maxSheild, float maxHP)
//{
//	UUserWidget* WidgetInstance = StatusWidgetComp->GetWidget();
//
//	if (UOZPlayerStatusBarWidget* statusBarWidget = Cast<UOZPlayerStatusBarWidget>(WidgetInstance))
//	{
//		float sheildSizeAmountRatio = maxSheild / maxHP;
//		statusBarWidget->SetSheildAmountRatio(sheildSizeAmountRatio);
//	}
//}

void AOZPlayer::OnCharacterSheildChanged_Implementation(float currSheildRatio)
{
	UUserWidget* WidgetInstance = StatusWidgetComp->GetWidget();

	if (UOZPlayerStatusBarWidget* statusBarWidget = Cast<UOZPlayerStatusBarWidget>(WidgetInstance))
	{
		statusBarWidget->SetSheildRatio(currSheildRatio);
	}
}

void AOZPlayer::OnWeaponHeatChanged_Implementation(float heatRatio)
{
	if (!IsLocallyControlled())
		return;

	AOZPlayerController* PC = Cast<AOZPlayerController>(GetController());

	if (AOZInGameHUD* ingameHud = PC->GetHUD<AOZInGameHUD>())
	{
		UOZRoundInComBatUI* roundInCombatUI = Cast<UOZRoundInComBatUI>(ingameHud->InGameUIArray[3]);
		if (roundInCombatUI)
		{
			roundInCombatUI->SetAimHeatRatio(heatRatio);
		}
	}
}

void AOZPlayer::OnCharacterHPChanged_Implementation(float currHP)
{
	UUserWidget* WidgetInstance = StatusWidgetComp->GetWidget();

	if (UOZPlayerStatusBarWidget* statusBarWidget = Cast<UOZPlayerStatusBarWidget>(WidgetInstance))
	{
		statusBarWidget->SetCurrentHP(currHP);
	}
}

void AOZPlayer::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	const float NewSpeed = Data.NewValue;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = NewSpeed;
	}
}

void AOZPlayer::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (!PAS) return;

	const float CurrentMoveSpeed = PAS->GetMoveSpeed();
	const float SprintSpeed = PAS->GetSprintSpeed();

	// Sprint 중이 아닐 때만 MoveSpeed 업데이트
	if (!FMath::IsNearlyEqual(CurrentMoveSpeed, SprintSpeed))
	{
		PAS->SetMoveSpeed(Data.NewValue);
	}
}

void AOZPlayer::OnCurrentStaminaChanged(const FOnAttributeChangeData& Data)
{
	AOZPlayerController* PC = Cast<AOZPlayerController>(GetController());

	if (PC == nullptr)
		return;

	const float NewStamina = Data.NewValue;
	float CurrentMaxStamina = 100.f;

	if (AOZPlayerState* OZPS = GetPlayerState<AOZPlayerState>())
	{
		CurrentMaxStamina = OZPS->PlayerAttributes->GetMaxStamina();
	}

	PC->OnCurrentStaminaChanged.Broadcast(NewStamina / CurrentMaxStamina);

}

void AOZPlayer::OnAimingDistanceChanged(const FOnAttributeChangeData& Data)
{
	const float NewDistance = Data.NewValue;
	if (CameraAimComp)
	{
		CameraAimComp->SetAdditionalCameraDistance(NewDistance);
	}
}

void AOZPlayer::OnCameraDepthChanged(const FOnAttributeChangeData& Data)
{
	const float NewDepth = Data.NewValue;

	if (CameraAimComp)
	{
		CameraAimComp->SetDefaultTargetArmLength(NewDepth);
	}
}

void AOZPlayer::OnViewingAngleChanged(const FOnAttributeChangeData& Data)
{
	if (VisionComp)
	{
		VisionComp->UpdateVisionParameters();
	}
}

void AOZPlayer::HandleOutOfHealth()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (AOZPlayerState* OZPS = GetPlayerState<AOZPlayerState>())
	{
		if (UAbilitySystemComponent* ASC = OZPS->GetAbilitySystemComponent())
		{
			ASC->AddLooseGameplayTag(OZGameplayTags::Player_State_Dead);
			ASC->CancelAllAbilities();
		}

		// 죽으면 부시 상태 해제
		OZPS->SetInBush_Server(false);
		OZPS->SetCurrentBushID(INDEX_NONE);
	}

	// 모든 나이아가라 이펙트 비활성화
	Multicast_DeactivateAllNiagaraEffects();

	MulticastPlayDeathMontage();

	UE_LOG(LogTemp, Warning, TEXT("Player %s is Dead!"), *GetName());

	OnRep_IsDead();

	SetLifeSpan(3.0f);
}

void AOZPlayer::Input_Move(const FInputActionValue& Value)
{
	const FVector2D MoveAxis = Value.Get<FVector2D>();
	LastMoveInput = MoveAxis;

	AddMovementInput(GetActorForwardVector(), MoveAxis.Y);
	AddMovementInput(GetActorRightVector(), MoveAxis.X);
}

void AOZPlayer::OnRep_IsDead()
{
	bool isLocallyControlled = false;

	if (bIsDead)
	{
		if (AOZPlayerController* PC = Cast<AOZPlayerController>(GetController()))
		{
			if (PC->IsLocalController())
			{
				isLocallyControlled = true;
				DisableInput(PC);

				if (VisionComp)
					VisionComp->DisableVisionAndShowAllPlayers();
			}
			PC->BP_OnLocalPlayerDead();


			if (AOZInGameHUD* ingameHud = PC->GetHUD<AOZInGameHUD>())
			{
				ingameHud->OnPlayerDie();
			}
		}
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->DisableMovement();
		}

	}

	if (isLocallyControlled == false)
		return;

	if (AOZPlayerState* OZPlayerState = GetPlayerState<AOZPlayerState>())
	{
		Server_ReportPlayerDead();
	}


}

void AOZPlayer::Server_ReportPlayerDead_Implementation()
{
	AGameModeBase* gameMode = GetWorld()->GetAuthGameMode();

	if (AOZInGameMode* OZingameMode = Cast<AOZInGameMode>(gameMode))
	{
		OZingameMode->OnInCombatStatePlayerDead(this);
	}
}

void AOZPlayer::MulticastPlayDeathMontage_Implementation()
{
	if (!DeathMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeathMontage is NULL on %s"), *GetName());
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Error, TEXT("MulticastPlayDeathMontage: Mesh is NULL on %s"), *GetName());
		return;
	}

	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (!AnimInst)
	{
		UE_LOG(LogTemp, Error, TEXT("MulticastPlayDeathMontage: AnimInstance is NULL on %s"), *GetName());
		return;
	}

	const float Duration = AnimInst->Montage_Play(DeathMontage, 1.0f);

	UE_LOG(LogTemp, Warning, TEXT("MulticastPlayDeathMontage: %s play death montage. Duration = %f"),
		*GetName(), Duration);

}

void AOZPlayer::HandleProjectileHit(AActor* HitActor, const FHitResult& Hit, AOZBullet* Bullet)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!HitActor || HitActor == this)
	{
		return;
	}

	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor);
	if (!TargetASI)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();

	if (!SourceASC || !TargetASC || !DamageEffectClass)
	{
		return;
	}

	float Damage = 0.f;
	float KnockbackImpulse = 0.f;

	if (const UOZWeaponAttributeSet* WeaponAS = SourceASC->GetSet<UOZWeaponAttributeSet>())
	{
		KnockbackImpulse = WeaponAS->GetBaseKnockbackPower();
	}

	if (Bullet)
	{
		Damage = Bullet->GetCurrentDamage();
	}

	if (Damage <= 0.f)
	{
		return;
	}

	ApplyDamageToASC(SourceASC, TargetASC, Damage, &Hit, Bullet ? (UObject*)Bullet : (UObject*)this);

	if (KnockbackImpulse > 0.f)
	{
		ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
		if (HitCharacter)
		{
			FVector KnockDir = (HitActor->GetActorLocation() - GetActorLocation());
			KnockDir.Z = 0.f;
			KnockDir.Normalize();

			FVector Impulse = KnockDir * KnockbackImpulse;

			if (UCharacterMovementComponent* MoveComp = HitCharacter->GetCharacterMovement())
			{
				MoveComp->AddImpulse(Impulse, true);
			}
		}
	}

	APawn* InstPawn = Bullet ? Cast<APawn>(Bullet->GetInstigator()) : nullptr;
	APawn* HitPawn = Cast<APawn>(HitActor);

	if (InstPawn && HitPawn)
	{
		RecordHit(HitPawn, InstPawn, Damage);
	}

	UE_LOG(LogTemp, Log, TEXT("[Damage] %s hit %s for %.1f"),
		*GetNameSafe(this), *GetNameSafe(HitActor), Damage);
}

void AOZPlayer::RecordHit(TObjectPtr<APawn> hittedPawn, TObjectPtr<APawn> instigatorPawn, float damage)
{
	AOZPlayerState* hittedPawnState = Cast<AOZPlayerState>(hittedPawn->GetPlayerState());
	AOZPlayerState* damageCursurState = Cast<AOZPlayerState>(instigatorPawn->GetPlayerState());

	hittedPawnState->Round_DamagedAmount += damage;
	hittedPawnState->Total_DamagedAmount += damage;
	hittedPawnState->LastDamageInstigatorState = damageCursurState;

	damageCursurState->Round_DamageAmount += damage;
	damageCursurState->Total_DamageAmount += damage;
}


void AOZPlayer::ApplyDamageToASC(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, float Damage, const FHitResult* OptionalHit, UObject* OptionalSourceObject)
{
	if (!HasAuthority()) return;
	if (!SourceASC || !TargetASC) return;
	if (Damage <= 0.f) return;

	TSubclassOf<UGameplayEffect> SelectedGE = DamageEffectClass;
	AActor* TargetOwner = TargetASC->GetOwner();
	if (TargetOwner && TargetOwner->IsA<AOZPuppet>())
	{
		SelectedGE = PuppetDamageEffectClass ? PuppetDamageEffectClass : DamageEffectClass;

		// 처음 Puppet 데미지 시 튜토리얼 팝업 호출
		if (!bFirstPuppetDamageTriggered)
		{
			bFirstPuppetDamageTriggered = true;
			Client_OnFirstPuppetDamage();
		}
	}

	if (!SelectedGE) return;

	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();

	if (OptionalHit)
	{
		ContextHandle.AddHitResult(*OptionalHit);
	}

	if (OptionalSourceObject)
	{
		ContextHandle.AddSourceObject(OptionalSourceObject);
	}
	else
	{
		ContextHandle.AddSourceObject(this);
	}

	FGameplayEffectSpecHandle SpecHandle =
		SourceASC->MakeOutgoingSpec(SelectedGE, 1.f, ContextHandle);

	if (!SpecHandle.IsValid()) return;

	const FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Player.Data.Damage"));
	SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, Damage);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	//아이템 데미지 기록
	{
		APawn* HittedPawn = nullptr;

		if (TargetASC)
		{
			HittedPawn = Cast<APawn>(TargetASC->GetAvatarActor());
		}

		APawn* InstigatorPawn = nullptr;

		if (SourceASC)
		{
			InstigatorPawn = Cast<APawn>(SourceASC->GetAvatarActor());
		}

		if (HittedPawn != nullptr && InstigatorPawn != nullptr)
		{
			RecordHit(HittedPawn, InstigatorPawn, Damage);
		}
		
	}
	
}

void AOZPlayer::Multicast_SetBandanaTexture_Implementation(UTexture2D* NewBandanaTexture, FLinearColor NewBandanaColor)
{
	// 멤버 변수에 저장
	BandanaTexture = NewBandanaTexture;
	BandanaColor = NewBandanaColor;

	USkeletalMeshComponent* ChrMesh = GetMesh();
	if (!ChrMesh)
		return;

	UMaterialInterface* OriginalMaterial = ChrMesh->GetMaterial(0);
	if (!OriginalMaterial)
		return;

	UMaterialInstanceDynamic* DynamicMaterial = ChrMesh->CreateDynamicMaterialInstance(0, OriginalMaterial);
	if (!DynamicMaterial)
		return;

	if (NewBandanaTexture)
	{
		DynamicMaterial->SetTextureParameterValue(FName("BandanaTextureParameter"), NewBandanaTexture);
	}
}

void AOZPlayer::Multicast_DisableVision_Implementation(float Duration)
{
	if (!VisionComp)
		return;

	if (IsLocallyControlled())
	{
		VisionComp->bFlashbanged = true;

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this]()
			{
				if (VisionComp && IsValid(VisionComp))
				{
					VisionComp->bFlashbanged = false;
				}
			});

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);

		UE_LOG(LogTemp, Log, TEXT("[FlashBang] %s vision cone disabled for %.1f seconds"), *GetName(), Duration);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[FlashBang] %s is not locally controlled, skipping"), *GetName());
	}
}

void AOZPlayer::Multicast_PlayGunFireSound_Implementation(const FVector& Location, const FOZGunSoundParams& Params)
{
	if (SoundComp)
	{
		SoundComp->PlayGunFireSound(Location, Params);
	}
}

void AOZPlayer::Client_PlayFogDamageSound_Implementation()
{
	if (SoundComp)
	{
		SoundComp->PlayFogDamageSound_Local();
	}
}

void AOZPlayer::Client_PlayBattleItemThrowSound_Implementation(float ThrowDistanceCm)
{
	if (SoundComp)
	{
		SoundComp->PlayBattleItemThrowSound_Local(ThrowDistanceCm);
	}
}

void AOZPlayer::Server_PlayFireMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	Multicast_PlayFireMontage(Montage, PlayRate);
}

void AOZPlayer::Multicast_PlayFireMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage) return;
	if (UOZAnimInstance* Anim = GetMesh() ? Cast<UOZAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
	{
		bool JumpToLoop = Anim->IsCombat;

		Anim->IsCombat = true;
		GetWorld()->GetTimerManager().ClearTimer(CombatStateCheckTimer);
		GetWorld()->GetTimerManager().SetTimer(CombatStateCheckTimer, this, &AOZPlayer::ResetCombatState, 5.f, false);
		Anim->Montage_Play(Montage, PlayRate);

		if (JumpToLoop)
		{
			Anim->Montage_JumpToSection(TEXT("Loop"), Montage);
		}
		else
		{
			Anim->Montage_JumpToSection(TEXT("Start"), Montage);
		}
	}
}

void AOZPlayer::ResetCombatState()
{
	if (UOZAnimInstance* Anim = GetMesh() ? Cast<UOZAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr)
	{
		Anim->IsCombat = false;
	}
}

void AOZPlayer::OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	// 로컬 플레이어만 UI/아웃라인 표시
	if (!IsLocallyControlled())
		return;

	if (OtherActor->Implements<UOZInteractable>())
	{
		InteractableActorsInRange.AddUnique(OtherActor);

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GI->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
			{
				if (OtherActor->IsA<AOZTower>())
				{
					if (!bFirstTowerOverlapTriggered)
					{
						bFirstTowerOverlapTriggered = true;
						TutorialSubsystem->CallTutorialPopupByID(9, 5.0f);
					}
				}
				else
				{
					if (!bFirstSlotMachineOverlapTriggered)
					{
						bFirstSlotMachineOverlapTriggered = true;
						TutorialSubsystem->CallTutorialPopupByID(4, 5.0f);
					}
				}
			}
		}

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		OtherActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
		{
			if (PrimComp)
			{
				PrimComp->SetRenderCustomDepth(true);
				PrimComp->SetCustomDepthStencilValue(InteractableOutlineStencilValue);
			}
		}

		UWidgetComponent* WidgetComp = OtherActor->FindComponentByClass<UWidgetComponent>();

		if (WidgetComp)
		{
			UOZInteractionUI* InteractionUI = Cast<UOZInteractionUI>(WidgetComp->GetUserWidgetObject());

			if (InteractionUI)
			{
				InteractionUI->SetTextVisibility(OtherActor);
				InteractionUI->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void AOZPlayer::OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	// 로컬 플레이어만 UI/아웃라인 처리
	if (!IsLocallyControlled())
		return;

	if (InteractableActorsInRange.Contains(OtherActor))
	{
		InteractableActorsInRange.Remove(OtherActor);

		TArray<UPrimitiveComponent*> PrimitiveComponents;
		OtherActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

		for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
		{
			if (PrimComp)
			{
				PrimComp->SetRenderCustomDepth(false);
			}
		}

		UWidgetComponent* WidgetComp = OtherActor->FindComponentByClass<UWidgetComponent>();

		if (WidgetComp)
		{
			UOZInteractionUI* InteractionUI = Cast<UOZInteractionUI>(WidgetComp->GetUserWidgetObject());

			if (InteractionUI)
			{
				InteractionUI->SetTextVisibility(OtherActor);
				InteractionUI->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

AActor* AOZPlayer::GetClosestInteractableActor() const
{
	if (InteractableActorsInRange.IsEmpty())
		return nullptr;

	AActor* ClosestActor = nullptr;
	float MinDistSq = FLT_MAX;
	FVector MyLocation = GetActorLocation();

	for (AActor* Actor : InteractableActorsInRange)
	{
		if (!Actor)
			continue;

		float DistSq = FVector::DistSquared(MyLocation, Actor->GetActorLocation());
		if (DistSq < MinDistSq)
		{
			MinDistSq = DistSq;
			ClosestActor = Actor;
		}
	}

	return ClosestActor;
}

void AOZPlayer::EnterSlotMachineView()
{
	if (!IsLocallyControlled())
		return;

	if (StatusWidgetComp)
		StatusWidgetComp->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Hidden);

	if (GetMesh())
		GetMesh()->SetHiddenInGame(true);

	if (WeaponMesh)
		WeaponMesh->SetHiddenInGame(true);

	if (VisionComp)
		VisionComp->IsActivate = false;

	if (ShieldEffectComponent)
	{
		ShieldEffectComponent->Deactivate();
		ShieldEffectComponent->SetVisibility(false, true);
		ShieldEffectComponent->SetHiddenInGame(true, true);
	}
}

void AOZPlayer::ExitSlotMachineView()
{
	if (!IsLocallyControlled())
		return;

	if (StatusWidgetComp)
		StatusWidgetComp->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Visible);

	if (GetMesh())
		GetMesh()->SetHiddenInGame(false);

	if (WeaponMesh)
		WeaponMesh->SetHiddenInGame(false);

	if (VisionComp)
		VisionComp->IsActivate = true;

	if (ShieldEffectComponent)
	{
		ShieldEffectComponent->Deactivate();
		ShieldEffectComponent->SetVisibility(true, true);
		ShieldEffectComponent->SetHiddenInGame(false, true);
	}
}

void AOZPlayer::Client_StartEffect_Implementation(EPlayerEffectType EffectType)
{
	if (EffectType == EPlayerEffectType::None)
		return;

	if (!ScreenEffectComponent)
		return;

	TObjectPtr<UNiagaraSystem>* FoundSystem = EffectSystemMap.Find(EffectType);
	if (!FoundSystem || !(*FoundSystem))
		return;

	if (CurrentEffectType == EffectType && ScreenEffectComponent->IsActive())
		return;

	if (ScreenEffectComponent->IsActive())
	{
		ScreenEffectComponent->Deactivate();
	}

	ScreenEffectComponent->SetAsset(FoundSystem->Get());
	ScreenEffectComponent->Activate(true);
	CurrentEffectType = EffectType;
}

void AOZPlayer::Client_StopEffect_Implementation()
{
	if (!ScreenEffectComponent)
		return;

	if (ScreenEffectComponent->IsActive())
	{
		ScreenEffectComponent->Deactivate();
	}
	CurrentEffectType = EPlayerEffectType::None;
}
void AOZPlayer::UpdateBushVision()
{
	AOZPlayerState* MyPS = GetPlayerState<AOZPlayerState>();
	if (!MyPS) return;

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->IsLocalController())
		return;

	AOZPlayer* LocalPlayer = Cast<AOZPlayer>(LocalPC->GetPawn());
	if (!LocalPlayer)
		return;

	AOZPlayerState* LocalPS = LocalPlayer->GetPlayerState<AOZPlayerState>();
	if (!LocalPS)
		return;

	const int32 MyBushID = MyPS->CurrentBushID;
	const int32 LocalBushID = LocalPS->CurrentBushID;

	bool bShouldBeVisible = false;

	if (MyBushID == INDEX_NONE && LocalBushID == INDEX_NONE)
	{
		bShouldBeVisible = true;
	}
	else if (MyBushID != INDEX_NONE && MyBushID == LocalBushID)
	{
		bShouldBeVisible = true;
	}
	else
	{
		bShouldBeVisible = false;
	}

	if (IsLocallyControlled())
	{
		SetBushVisibility(true);
		return;
	}

	SetBushVisibility(bShouldBeVisible);
}

void AOZPlayer::SetBushVisibility(bool bVisible)
{
	if (USkeletalMeshComponent* PlayerMesh = GetMesh())
	{
		PlayerMesh->SetVisibility(bVisible, true);
		PlayerMesh->SetCastShadow(bVisible);
	}
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(bVisible, true);
	}
	if (StatusWidgetComp)
	{
		StatusWidgetComp->SetVisibility(bVisible, true);
	}
}

void AOZPlayer::Multicast_UpdateShieldEffect_Implementation(bool bHasShield)
{
	UpdateShieldEffectLocal(bHasShield);
}

void AOZPlayer::UpdateShieldEffectLocal(bool bHasShield)
{
	// 플래그 업데이트 (VisionComponent에서 참조)
	bShieldEffectActive = bHasShield;

	if (!ShieldEffectComponent)
		return;

	if (bHasShield)
	{
		if (!ShieldEffectComponent->IsActive())
		{
			ShieldEffectComponent->Activate(true);
		}
		// 로컬 플레이어는 VisionComponent가 관리하지 않으므로 직접 Visibility 설정
		if (IsLocallyControlled())
		{
			ShieldEffectComponent->SetVisibility(true);
		}
		// 다른 플레이어의 경우 VisionComponent에서 시야에 따라 관리
	}
	else
	{
		// 실드가 없으면 Deactivate + Visibility 강제 false
		ShieldEffectComponent->Deactivate();
		ShieldEffectComponent->SetVisibility(false);
	}
}

void AOZPlayer::Multicast_DeactivateAllNiagaraEffects_Implementation()
{
	TArray<UNiagaraComponent*> NiagaraComponents;
	GetComponents<UNiagaraComponent>(NiagaraComponents);

	for (UNiagaraComponent* Comp : NiagaraComponents)
	{
		if (Comp && Comp->IsActive())
		{
			Comp->Deactivate();
			Comp->SetVisibility(false);
		}
	}

	// 플래그도 업데이트
	bShieldEffectActive = false;
	CurrentEffectType = EPlayerEffectType::None;
}

void AOZPlayer::Client_OnFirstPuppetDamage_Implementation()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOZInGameTutorialPopupSubSystem* TutorialSubsystem = GI->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
		{
			TutorialSubsystem->CallTutorialPopupByID(6, 5.0f);
		}
	}
}

void AOZPlayer::Client_PlayFlashBangHitEffect_Implementation(UNiagaraSystem* EffectSystem)
{
	if (!EffectSystem)
		return;

	const FVector Location = GetActorLocation();
	const FRotator Rotation = FRotator::ZeroRotator;
	const FVector Scale = FVector(1.f);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		EffectSystem,
		Location,
		Rotation,
		Scale,
		true,
		true,
		ENCPoolMethod::AutoRelease
	);
}
