#include "GameState/OZInGameGameState.h"
#include "HUD/OZInGameHUD.h"
#include "Subsystem/OZLoadingSubsystem.h"
#include "Utils/Util.h"
#include "UserInterface/OZRoundPrepareUI.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/OZInGameMode.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerController.h"
#include "Shop/OZShopManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Subsystem/OZModuleSubsystem.h"
#include "Instance/OZGameInstance.h"
#include "Object/OZFogVisualActor.h"
#include "NiagaraComponent.h"
#include "Character/OZPlayer.h"
#include "Widget/OZWidgetComponent.h"
#include "Subsystem/OZSoundSubsystem.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

void AOZInGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZInGameGameState, CurrentRoundState);
	DOREPLIFETIME(AOZInGameGameState, prepareStateTime);
	DOREPLIFETIME(AOZInGameGameState, combatStateTime);
	DOREPLIFETIME(AOZInGameGameState, resultStateTime);
	DOREPLIFETIME(AOZInGameGameState, bIsAvilityRound);
	DOREPLIFETIME(AOZInGameGameState, roundCombatPlayerState);

	DOREPLIFETIME(AOZInGameGameState, roundCombatBonus);
	DOREPLIFETIME(AOZInGameGameState, bonusePlayerNames);
	DOREPLIFETIME(AOZInGameGameState, bIsCombatResultSetted);
	
	
	DOREPLIFETIME(AOZInGameGameState, worldBound);

	DOREPLIFETIME(AOZInGameGameState, ShopManager);
	DOREPLIFETIME(AOZInGameGameState, CurrentConvertGrade);

	DOREPLIFETIME(AOZInGameGameState, RepFogState);
	DOREPLIFETIME(AOZInGameGameState, RepFogPhaseIndex);
	DOREPLIFETIME(AOZInGameGameState, RepFogCenter);
	DOREPLIFETIME(AOZInGameGameState, RepFogSafeRadiusCm);
	DOREPLIFETIME(AOZInGameGameState, RepFogInitialMapRadiusCm);

	DOREPLIFETIME(AOZInGameGameState, RepFogPrevCenter);
	DOREPLIFETIME(AOZInGameGameState, RepFogPrevRadiusCm);
	DOREPLIFETIME(AOZInGameGameState, RepFogNextCenter);
	DOREPLIFETIME(AOZInGameGameState, RepFogNextRadiusCm);

	DOREPLIFETIME(AOZInGameGameState, TargetShrinkingTime);

	DOREPLIFETIME(AOZInGameGameState, totalPlayerNum);
}

void AOZInGameGameState::Multicast_OnStartRound_Implementation(double ServerTime)
{
	RoundStartTime = ServerTime;
	CurrentRound += 1;

	if (!HasAuthority())
	{
		ClientServerRoundStartDelayTime = ServerTime - GetWorld()->GetTimeSeconds();
	}
}

void AOZInGameGameState::Multicast_OnStartCombat_Implementation(double ServerTime)
{
	CombatStartTime = ServerTime;

	if (!HasAuthority())
	{
		ClientServerCombatStartDelayTime = ServerTime - GetWorld()->GetTimeSeconds();
	}
}

void AOZInGameGameState::Multicast_OnStartResult_Implementation(double ServerTime)
{
	ResultStartTime = ServerTime;

	if (!HasAuthority())
	{
		ClientServerResultDelayTime = ServerTime - GetWorld()->GetTimeSeconds();
	}
}

void AOZInGameGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ShopManager = GetWorld()->SpawnActor<AOZShopManager>(AOZShopManager::StaticClass(), SpawnParams);
	}

	// 서버/클라이언트 모두 원본 PrepareActor 파괴 (서버는 스폰 정보도 저장)
	InitializePrepareStateActors();

	if (!HasAuthority())
	{
		UpdateFogVisual_Local();

		// 클라이언트가 레벨에 완전히 진입했을 때 로딩 화면 숨기기
		if (UOZLoadingSubsystem* LoadingSubsystem = GetGameInstance()->GetSubsystem<UOZLoadingSubsystem>())
		{
			LoadingSubsystem->EraseLoadingScene();
		}
	}
}

void AOZInGameGameState::HidLoadingScreen_Implementation()
{
	UOZLoadingSubsystem* loadingSubsystem = GetGameInstance()->GetSubsystem<UOZLoadingSubsystem>();

	Cast<UOZGameInstance>(GetGameInstance())->PrintLog(TEXT("Received_HideLoadingScreen"), FColor::Green, 1.f);

	if (loadingSubsystem != nullptr)
	{
		loadingSubsystem->EraseLoadingScene();
	}

}

void AOZInGameGameState::OnRep_TargetShrinkingTimeSetted()
{

}

void AOZInGameGameState::OnRep_CurrentRoundState()
{
	FString StateName;

	switch (CurrentRoundState)
	{
	case EGameStateType::STATE_PREP:
		StateName = TEXT("Prepare State");
		EnterPrepareState();
		break;
	case EGameStateType::STATE_COMBAT:
		StateName = TEXT("Battle State");
		EnterComBatState();
		break;
	case EGameStateType::STATE_RESULT:
		StateName = TEXT("Result State");
		EnterResultState();
		break;
	case EGameStateType::STATE_GAMEEND:
		StateName = TEXT("Game End");
		EnterGameEndState();
		break;
	default:
		StateName = TEXT("Unknown State");
		break;
	}
}

//void AOZInGameGameState::OnRep_CombatResultPlayerStates()
//{
//	if (roundCombatPlayerState.IsEmpty())
//		return;
//
//	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AOZInGameGameState::CheckCombatResultfullReplicated);
//}

void AOZInGameGameState::OnRep_FogSnapshot()
{
	UpdateFogVisual_Local();


}

void AOZInGameGameState::UpdateFogVisual_Local()
{
	AOZFogVisualActor* FogVis = GetFogVisualActor();
	if (!FogVis)
	{
		return;
	}

	FogVis->ApplyFogSnapshot(
		RepFogState,
		RepFogPhaseIndex,
		RepFogCenter,
		RepFogSafeRadiusCm,
		RepFogInitialMapRadiusCm
	);
}

void AOZInGameGameState::OnRep_CombatResultSetted()
{
	if (bIsCombatResultSetted == false)
		return;

	CheckCombatResultfullReplicated();
}

void AOZInGameGameState::SetWorldBounds(const FVector& Center, const FVector& Extent)
{
	if (!HasAuthority())
		return;

	worldBound.Center = Center;
	worldBound.Size = Extent;

	On_worldBoundRep();
}

void AOZInGameGameState::DeActivatePlayerWidgetComponent_Implementation()
{
	APlayerController* LocalPC = GetLocallPlayercontroller();
	AOZPlayer* LocalPlayer = nullptr;

	if (LocalPC)
	{
		LocalPlayer = Cast<AOZPlayer>(LocalPC->GetPawn());
	}

	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZPlayer::StaticClass(), AllPlayers);

	for (AActor* Actor : AllPlayers)
	{
		AOZPlayer* OtherPlayer = Cast<AOZPlayer>(Actor);

		if (OtherPlayer && OtherPlayer != LocalPlayer)
		{
			if (UOZWidgetComponent* WidgetComp = OtherPlayer->StatusWidgetComp)
			{
				if (UUserWidget* Widget = WidgetComp->GetUserWidgetObject())
				{
					Widget->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}
}

void AOZInGameGameState::On_worldBoundRep()
{
	AOZPlayerController* OzPlayerController = Cast<AOZPlayerController>(GetLocallPlayercontroller());

	if (OzPlayerController)
	{
		OzPlayerController->InitializeInGameMinimap(worldBound);
	}
}

AOZFogVisualActor* AOZInGameGameState::GetFogVisualActor()
{
	if (CachedFogVisual)
		return CachedFogVisual;

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), AOZFogVisualActor::StaticClass());
	CachedFogVisual = Cast<AOZFogVisualActor>(Found);
	return CachedFogVisual;
}

void AOZInGameGameState::Multicast_CommonTutorialMsg_Implementation(int32 index)
{
	if (AOZPlayerController* PC = Cast<AOZPlayerController>(GetLocallPlayercontroller()))
	{
		PC->Client_RecievedCommonTutorialPopupCall(index);
	}
}

void AOZInGameGameState::EnterPrepareState()
{
	if (!GetWorld())
		return;

	UOZSoundSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UOZSoundSubsystem>();
	if (SoundSubsystem && PrepareBGM)
	{
		if (CurrentRound > 1)
		{
			SoundSubsystem->StopBGMAndPlayNext(PrepareBGM, 0.5f);
		}
		else
		{
			SoundSubsystem->PlayBGM(PrepareBGM);
		}
	}

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
		return;

	APlayerController* PC = GetLocallPlayercontroller();
	if (PC == nullptr)
		return;

	//PC->SetInputMode(FInputModeGameOnly());

	roundCombatBonus = ECombatBonus::Max;
	bonusePlayerNames.Empty();

	if (!HasAuthority())
	{
		bPlayedPrep5SecSfx = false;
		PrevPrepRemainTime = 9999.f;

		GetWorld()->GetTimerManager().ClearTimer(PrepCountdownSfxTimer);
		GetWorld()->GetTimerManager().SetTimer(
			PrepCountdownSfxTimer,
			this,
			&AOZInGameGameState::TickPrepCountdownSfx,
			0.1f,
			true
		);
	}

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetShowRoundUI(CurrentRound);

		OZInGameHud->SetFloorUIState(CurrentRoundState);
		//OZInGameHud->SetFloorRoundNum(CurrentRound);
		OZInGameHud->SetInvenEntriesActive(true);
		OZInGameHud->ChangeFloorUISwitcher(true);

		if (bIsAvilityRound)
		{
			OZInGameHud->SetInGameUI(EInGameUIType::AVILITY_UI, true);
		}

	}

}

void AOZInGameGameState::EnterComBatState()
{
	if (!GetWorld())
		return;

	// 전투단계 돌입 시 정비단계 BGM 페이드아웃
	UOZSoundSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UOZSoundSubsystem>();
	if (SoundSubsystem)
	{
		SoundSubsystem->StopBGM(0.5f);
	}

	OnCombatStateEntered.Broadcast();

	if (!HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(PrepCountdownSfxTimer);
	}

	if (!HasAuthority() && CombatStartSfx)
	{
		UGameplayStatics::PlaySound2D(this, CombatStartSfx);
	}

	APlayerController* PC = GetLocallPlayercontroller();

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		PC->GetPlayerState<AOZPlayerState>()->EraserCurrentViewObjects();;

		OZInGameHud->PrepareMinimap(this, totalPlayerNum);

		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetInvenEntriesActive(false);

		if (OZInGameHud->InGameUIArray[2]->GetVisibility() == ESlateVisibility::Visible)
		{

		}

		OZInGameHud->SetInGameUI(EInGameUIType::ROUND_COMBAT, true);

		OZInGameHud->SetFloorUIState(CurrentRoundState, CurrentRound);
		//OZInGameHud->SetRemainTimeCombat();
		OZInGameHud->bIsCanUpdateMinimap = true;
	}
}

void AOZInGameGameState::EnterResultState()
{
	if (!GetWorld())
		return;

	UOZSoundSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UOZSoundSubsystem>();
	if (SoundSubsystem && ResultBGM)
		SoundSubsystem->PlayBGM(ResultBGM);

	APlayerController* PC = GetLocallPlayercontroller();

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetInGameUI(EInGameUIType::ROUND_COMBATRESULT, true);
		OZInGameHud->SetFloorUIState(CurrentRoundState);
		OZInGameHud->DisplayCombatResult();
		OZInGameHud->SetCombatBonusRewardPopup(roundCombatBonus, bonusePlayerNames);
		OZInGameHud->bIsCanUpdateMinimap = false;
	}
}

void AOZInGameGameState::EnterGameEndState()
{
	if (!GetWorld())
		return;

	APlayerController* PC = GetLocallPlayercontroller();

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetInGameUI(EInGameUIType::GAME_END, true);
		OZInGameHud->SetGameResultUserInfo(roundCombatPlayerState);

	}
}

void AOZInGameGameState::CheckCombatResultfullReplicated()
{
	int32 CountSurvivalRankingOne = 0;
	int32 CountBonusRewardPlayers = 0;

	for (AOZPlayerState* PS : roundCombatPlayerState)
	{
		if (PS == nullptr)
			return;

		if (PS->Round_SurvivalRanking == 1)
			CountSurvivalRankingOne++;

		if (PS->RoundCombatBonus != ECombatBonus::Max)
		{
			CountBonusRewardPlayers++;
		}
			
	}

	if (CountSurvivalRankingOne > 1 || CountBonusRewardPlayers == 0 ||
		roundCombatBonus == ECombatBonus::Max || bonusePlayerNames.IsEmpty())
	{
		if (roundCombatBonus == ECombatBonus::Max)
			roundCombatBonus = roundCombatPlayerState[0]->RoundCombatBonus;

		// 아직 배열 내부의 State 복사가 덜 된것
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			this, &AOZInGameGameState::CheckCombatResultfullReplicated);
		return;
	}

	AOZPlayerController* PC = Cast<AOZPlayerController>(GetLocallPlayercontroller());

	if (PC)
	{
		PC->Server_ReportcombatResultRecived();
	}
		
}

APlayerController* AOZInGameGameState::GetLocallPlayercontroller()
{
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
		return nullptr;

	APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
	if (PC == nullptr)
		return nullptr;

	return PC;
}

void AOZInGameGameState::SelectRandomEffects(int32 Count)
{
	// 호환성을 위해 유지 - 첫 번째 로컬 플레이어의 PlayerState 사용
	APlayerController* PC = GetLocallPlayercontroller();
	if (PC)
	{
		AOZPlayerState* PS = PC->GetPlayerState<AOZPlayerState>();
		SelectRandomEffectsForPlayer(PS, Count);
	}
}

void AOZInGameGameState::SelectRandomEffectsForPlayer(AOZPlayerState* TargetPlayerState, int32 Count)
{
	SelectedEffectArray.Empty();
	SlotRerollUsed.Empty();
	SlotRerollUsed.SetNum(Count);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	UOZGameInstance* OZGameInstance = Cast<UOZGameInstance>(GameInstance);
	if (!OZGameInstance)
		return;

	UOZConvertSubsystem* ConvertSubsystem = OZGameInstance->GetSubsystem<UOZConvertSubsystem>();
	if (!ConvertSubsystem)
		return;

	// 서버에서만 등급 결정
	if (HasAuthority())
	{
		float AdjustedLegendary = LegendaryProbability;
		float AdjustedUnique = UniqueProbability;
		float AdjustedRare = RareProbability;

		if (LastGrantedConvertGrade != EConvertGrade::None)
		{
			switch (LastGrantedConvertGrade)
			{
			case EConvertGrade::Legendary:
				AdjustedLegendary -= 20.0f;
				AdjustedUnique += 10.0f;
				AdjustedRare += 10.0f;
				break;

			case EConvertGrade::Unique:
				AdjustedLegendary += 10.0f;
				AdjustedUnique -= 20.0f;
				AdjustedRare += 10.0f;
				break;

			case EConvertGrade::Rare:
				AdjustedLegendary += 10.0f;
				AdjustedUnique += 10.0f;
				AdjustedRare -= 20.0f;
				break;
			}
		}

		float RandomValue = FMath::FRandRange(0.0f, 100.0f);

		if (RandomValue < AdjustedLegendary)
		{
			CurrentConvertGrade = EConvertGrade::Legendary;
		}
		else if (RandomValue < AdjustedLegendary + AdjustedUnique)
		{
			CurrentConvertGrade = EConvertGrade::Unique;
		}
		else
		{
			CurrentConvertGrade = EConvertGrade::Rare;
		}

		LastGrantedConvertGrade = CurrentConvertGrade;
	}

	// 제외할 ID 목록 구성
	TArray<int32> ExcludeIDs;

	// 유니크/레전더리 등급일 경우, 해당 플레이어가 이전에 획득한 증강들 제외
	if ((CurrentConvertGrade == EConvertGrade::Unique || CurrentConvertGrade == EConvertGrade::Legendary) && TargetPlayerState)
	{
		ExcludeIDs.Append(TargetPlayerState->GetUniqueConvertHistory());
	}

	for (int i = 0; i < Count; i++)
	{
		bool bSuccess = false;
		FOZConvertData SelectedConvert = ConvertSubsystem->GetRandomConvertByGrade(CurrentConvertGrade, ExcludeIDs, bSuccess);

		if (bSuccess)
		{
			SelectedEffectArray.Add(SelectedConvert);

			// 모든 등급에서 현재 선택지 내 중복 방지
			ExcludeIDs.Add(SelectedConvert.ID);

			SlotRerollUsed[i] = false;
		}
	}
}

bool AOZInGameGameState::RerollSelectedEffect(int32 SlotIndex)
{
	if (!SelectedEffectArray.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedEffect: Invalid slot index %d"), SlotIndex);
		return false;
	}

	if (SlotRerollUsed.IsValidIndex(SlotIndex) && SlotRerollUsed[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedEffect: Slot %d already used reroll"), SlotIndex);
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return false;

	UOZGameInstance* OZGameInstance = Cast<UOZGameInstance>(GameInstance);
	if (!OZGameInstance)
		return false;

	UOZConvertSubsystem* ConvertSubsystem = OZGameInstance->GetSubsystem<UOZConvertSubsystem>();
	if (!ConvertSubsystem)
		return false;

	TArray<int32> ExcludeIDs;

	// 유니크/레전더리 등급일 경우, 해당 플레이어가 이전에 획득한 증강들 제외
	if (CurrentConvertGrade == EConvertGrade::Unique || CurrentConvertGrade == EConvertGrade::Legendary)
	{
		APlayerController* PC = GetLocallPlayercontroller();
		if (PC)
		{
			AOZPlayerState* PS = PC->GetPlayerState<AOZPlayerState>();
			if (PS)
			{
				ExcludeIDs.Append(PS->GetUniqueConvertHistory());
			}
		}
	}

	// 현재 선택지에 있는 모든 증강 제외
	for (int32 i = 0; i < SelectedEffectArray.Num(); i++)
	{
		ExcludeIDs.AddUnique(SelectedEffectArray[i].ID);
	}

	bool bSuccess = false;
	FOZConvertData NewConvert = ConvertSubsystem->GetRandomConvertByGrade(CurrentConvertGrade, ExcludeIDs, bSuccess);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedEffect: No available converts for slot %d"), SlotIndex);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Rerolled Slot %d: %s (ID: %d) -> %s (ID: %d)"),
		SlotIndex,
		*SelectedEffectArray[SlotIndex].Name.ToString(), SelectedEffectArray[SlotIndex].ID,
		*NewConvert.Name.ToString(), NewConvert.ID);

	SelectedEffectArray[SlotIndex] = NewConvert;
	SlotRerollUsed[SlotIndex] = true;

	return true;
}

bool AOZInGameGameState::CanRerollSlot(int32 SlotIndex) const
{
	if (!SelectedEffectArray.IsValidIndex(SlotIndex))
		return false;

	if (!SlotRerollUsed.IsValidIndex(SlotIndex))
		return false;

	return !SlotRerollUsed[SlotIndex];
}

void AOZInGameGameState::ResetRerollStates()
{
	SlotRerollUsed.Empty();
	SlotRerollUsed.SetNum(SelectedEffectArray.Num());

	for (int32 i = 0; i < SlotRerollUsed.Num(); i++)
	{
		SlotRerollUsed[i] = false;
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Reroll states reset"));
}

void AOZInGameGameState::Multicast_SyncModuleFinalValues_Implementation(const TArray<int32>& ModuleIDs, const TArray<float>& FinalValues)
{
	// 서버는 이미 계산 완료되어 있으므로 클라이언트만 동기화
	if (HasAuthority())
		return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	UOZModuleSubsystem* ModuleSubsystem = GameInstance->GetSubsystem<UOZModuleSubsystem>();
	if (!ModuleSubsystem)
		return;

	for (int32 i = 0; i < ModuleIDs.Num(); i++)
	{
		if (FinalValues.IsValidIndex(i))
		{
			ModuleSubsystem->SetCachedFinalValue(ModuleIDs[i], FinalValues[i]);
		}
	}
}

void AOZInGameGameState::Multicast_SyncModuleMinValues_Implementation(const TArray<int32>& ModuleIDs, const TArray<float>& MinValues)
{
	// 서버는 이미 계산 완료되어 있으므로 클라이언트만 동기화
	if (HasAuthority())
		return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	UOZModuleSubsystem* ModuleSubsystem = GameInstance->GetSubsystem<UOZModuleSubsystem>();
	if (!ModuleSubsystem)
		return;

	for (int32 i = 0; i < ModuleIDs.Num(); i++)
	{
		if (MinValues.IsValidIndex(i))
		{
			ModuleSubsystem->SetCachedMinValue(ModuleIDs[i], MinValues[i]);
		}
	}
}

void AOZInGameGameState::Multicast_SyncModuleMaxValues_Implementation(const TArray<int32>& ModuleIDs, const TArray<float>& MaxValues)
{
	// 서버는 이미 계산 완료되어 있으므로 클라이언트만 동기화
	if (HasAuthority())
		return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	UOZModuleSubsystem* ModuleSubsystem = GameInstance->GetSubsystem<UOZModuleSubsystem>();
	if (!ModuleSubsystem)
		return;

	for (int32 i = 0; i < ModuleIDs.Num(); i++)
	{
		if (MaxValues.IsValidIndex(i))
		{
			ModuleSubsystem->SetCachedMaxValue(ModuleIDs[i], MaxValues[i]);
		}
	}
}

void AOZInGameGameState::SelectRandomModules(int32 Count)
{
	SelectedModuleArray.Empty();
	ModuleSlotRerollUsed.Empty();
	ModuleSlotRerollUsed.SetNum(Count);
	OriginalModuleIDs.Empty();
	OriginalModuleIDs.SetNum(Count);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	UOZGameInstance* OZGameInstance = Cast<UOZGameInstance>(GameInstance);
	if (!OZGameInstance)
		return;

	UOZModuleSubsystem* ModuleSubsystem = OZGameInstance->GetSubsystem<UOZModuleSubsystem>();
	if (!ModuleSubsystem)
		return;

	// 모듈에는 등급이 없으므로 모든 모듈에서 랜덤 선택
	TArray<int32> ExcludeIDs;

	for (int i = 0; i < Count; i++)
	{
		bool bSuccess = false;
		FOZModuleData SelectedModule = ModuleSubsystem->GetRandomModule(ExcludeIDs, bSuccess);

		if (bSuccess)
		{
			SelectedModuleArray.Add(SelectedModule);

			// 중복 방지
			ExcludeIDs.Add(SelectedModule.Module_ID);

			ModuleSlotRerollUsed[i] = false;
			OriginalModuleIDs[i] = SelectedModule.Module_ID;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] Failed to select module for slot %d"), i);
		}
	}
}

void AOZInGameGameState::InitializePrepareStateActors()
{
	TArray<AActor*> TaggedActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), PrepareStateActorTag, TaggedActors);

	// 서버는 스폰 정보 저장 + 원본 파괴
	// 클라이언트는 원본 파괴만 
	if (HasAuthority())
	{
		PrepareActorSpawnInfos.Reserve(TaggedActors.Num());

		for (AActor* Actor : TaggedActors)
		{
			if (!IsValid(Actor))
				continue;

			FPrepareActorSpawnInfo Info;
			Info.ActorClass = Actor->GetClass();
			Info.SpawnTransform = Actor->GetActorTransform();
			Info.SpawnScale = Actor->GetActorScale3D();
			PrepareActorSpawnInfos.Add(Info);

			Actor->Destroy();
		}
	}
	else
	{
		for (AActor* Actor : TaggedActors)
		{
			if (IsValid(Actor))
			{
				Actor->Destroy();
			}
		}
	}
}

void AOZInGameGameState::SpawnPrepareStateActors()
{
	if (!HasAuthority())
		return;

	for (const FPrepareActorSpawnInfo& Info : PrepareActorSpawnInfos)
	{
		if (!Info.ActorClass)
			continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			Info.ActorClass,
			Info.SpawnTransform,
			SpawnParams
		);

		if (SpawnedActor)
		{
			SpawnedActor->SetActorScale3D(Info.SpawnScale);
			SpawnedPrepareActors.Add(SpawnedActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Spawned %d PrepareStateActors"), SpawnedPrepareActors.Num());
}

void AOZInGameGameState::DestroyPrepareStateActors()
{
	if (!HasAuthority())
		return;

	for (AActor* Actor : SpawnedPrepareActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Destroyed %d PrepareStateActors"), SpawnedPrepareActors.Num());
	SpawnedPrepareActors.Empty();
}

bool AOZInGameGameState::RerollSelectedModule(int32 SlotIndex)
{
	if (!SelectedModuleArray.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedModule: Invalid slot index %d"), SlotIndex);
		return false;
	}

	if (ModuleSlotRerollUsed.IsValidIndex(SlotIndex) && ModuleSlotRerollUsed[SlotIndex])
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedModule: Slot %d already used reroll"), SlotIndex);
		return false;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return false;

	UOZGameInstance* OZGameInstance = Cast<UOZGameInstance>(GameInstance);
	if (!OZGameInstance)
		return false;

	UOZModuleSubsystem* ModuleSubsystem = OZGameInstance->GetSubsystem<UOZModuleSubsystem>();
	if (!ModuleSubsystem)
		return false;

	// 제외할 ID: 현재 선택지에 있는 모든 모듈 + 원래 이 슬롯에 있던 모듈
	TArray<int32> ExcludeIDs;
	for (int32 i = 0; i < SelectedModuleArray.Num(); i++)
	{
		ExcludeIDs.Add(SelectedModuleArray[i].Module_ID);
	}

	// 원래 모듈 ID도 제외 (같은 모듈로 리롤 방지)
	if (OriginalModuleIDs.IsValidIndex(SlotIndex))
	{
		ExcludeIDs.AddUnique(OriginalModuleIDs[SlotIndex]);
	}

	bool bSuccess = false;
	FOZModuleData NewModule = ModuleSubsystem->GetRandomModule(ExcludeIDs, bSuccess);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZInGameGameState] RerollSelectedModule: No available modules for slot %d"), SlotIndex);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Rerolled Module Slot %d: %s (ID: %d) -> %s (ID: %d)"),
		SlotIndex,
		*SelectedModuleArray[SlotIndex].name.ToString(), SelectedModuleArray[SlotIndex].Module_ID,
		*NewModule.name.ToString(), NewModule.Module_ID);

	SelectedModuleArray[SlotIndex] = NewModule;
	ModuleSlotRerollUsed[SlotIndex] = true;

	return true;
}

bool AOZInGameGameState::CanRerollModuleSlot(int32 SlotIndex) const
{
	if (!SelectedModuleArray.IsValidIndex(SlotIndex))
		return false;

	if (!ModuleSlotRerollUsed.IsValidIndex(SlotIndex))
		return false;

	return !ModuleSlotRerollUsed[SlotIndex];
}

void AOZInGameGameState::ResetModuleRerollStates()
{
	ModuleSlotRerollUsed.Empty();
	ModuleSlotRerollUsed.SetNum(SelectedModuleArray.Num());
	OriginalModuleIDs.Empty();
	OriginalModuleIDs.SetNum(SelectedModuleArray.Num());

	for (int32 i = 0; i < ModuleSlotRerollUsed.Num(); i++)
	{
		ModuleSlotRerollUsed[i] = false;
		if (SelectedModuleArray.IsValidIndex(i))
		{
			OriginalModuleIDs[i] = SelectedModuleArray[i].Module_ID;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[OZInGameGameState] Module reroll states reset"));
}

double AOZInGameGameState::GetPrepareRemainingTime_Local() const
{
	if (!GetWorld()) return 0.0;

	const double NowLocal = GetWorld()->GetTimeSeconds();
	const double ServerNowApprox = NowLocal + ClientServerRoundStartDelayTime;
	const double Elapsed = ServerNowApprox - RoundStartTime;

	return FMath::Max(0.0, (double)prepareStateTime - Elapsed);
}
#include "Kismet/GameplayStatics.h"

void AOZInGameGameState::TickPrepCountdownSfx()
{
	if (HasAuthority()) return;

	const float Remain = (float)GetPrepareRemainingTime_Local();

	if (!bPlayedPrep5SecSfx && PrevPrepRemainTime > 5.f && Remain <= 5.f)
	{
		bPlayedPrep5SecSfx = true;
		if (PrepFiveSecSfx)
		{
			UGameplayStatics::PlaySound2D(this, PrepFiveSecSfx);
		}
	}

	PrevPrepRemainTime = Remain;

	if (Remain <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(PrepCountdownSfxTimer);
	}
}

void AOZInGameGameState::Multicast_PlayOpeningCutscene_Implementation()
{
	if (AOZPlayerController* PC = Cast<AOZPlayerController>(GetLocallPlayercontroller()))
	{
		PC->Client_PlayOpeningCutscene();
	}
}
