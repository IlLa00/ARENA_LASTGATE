// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OZInGameMode.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerController.h"
#include "HUD/OZInGameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameState/OZInGameGameState.h"
#include "Utils/Util.h"
#include "Object/OZBlackFogManager.h"
#include "Object/OZTower.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Subsystem/OZRewardSubsystem.h"
#include "Subsystem/OZModuleSubsystem.h"
#include "Object/OZWorldBoundCacher.h"
#include "Data/OZRewardData.h"
#include "Tags/OZGameplayTags.h"
#include "Subsystem/OZOnlineSessionSubsystem.h"
#include "Subsystem/OZSoundSubsystem.h"
#include "AbilitySystemComponent.h"
#include "Instance/OZGameInstance.h"
#include "Misc/OutputDeviceNull.h"
#include "UserInterface/OZInGameFloorUI.h"

AOZInGameMode::AOZInGameMode()
{
	bUseSeamlessTravel = false;

	PlayerStateClass = AOZPlayerState::StaticClass();
	GameStateClass = AOZInGameGameState::StaticClass();
	PlayerControllerClass = AOZPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<AOZInGameHUD> InGameHUD(TEXT("/Game/UI/HUD/BP_InGameHUD"));
	if (InGameHUD.Class != NULL)
	{
		HUDClass = InGameHUD.Class;
	}

	static ConstructorHelpers::FClassFinder<AOZPlayer> OzPlayerClass(TEXT("/Game/Blueprint/Player/BP_OZPlayer"));
	if (OzPlayerClass.Class != NULL)
	{
		DefaultPawnClass = OzPlayerClass.Class;
	}
}

void AOZInGameMode::BeginPlay()
{
	Super::BeginPlay();

	AOZInGameGameState* GS = GetGameState<AOZInGameGameState>();
	if (GS)
	{
		GS->prepareStateTime = prepareStateTime;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		APlayerStart* Start = Cast<APlayerStart>(Actor);
		if (Start)
		{
			OZPlayerStartPosArray.Add(Start);
		}
	}

	TArray<AActor*> FogActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZBlackFogManager::StaticClass(), FogActors);

	if (FogActors.Num() > 0)
	{
		FogManager = Cast<AOZBlackFogManager>(FogActors[0]);
		
		if (FogManager != nullptr)
		{
			FogManager->OnBlackFogStartShrink.AddDynamic(this, &AOZInGameMode::OnDarkSmogeEnterPhase);
			FogManager->OnBlackFogStartCountingForShrink.AddDynamic(this, &AOZInGameMode::OnDarkSmogeStartCounting);
		}

	}
}

void AOZInGameMode::StartGameMode()
{
	InitializeGame();
	//TakeMinimapPicture();
	MultiCastHidLoadingScreen();

	//StartRound();

	bIsGameStarted = false;              
	FinishedCutscenePlayers.Empty();

	if (OzInGameState)
	{
		OzInGameState->Multicast_PlayOpeningCutscene();
	}
}

void AOZInGameMode::InitializeGame()
{
	CurrentRound = 0;

	if (RoundAbilities.IsEmpty())
	{
		RoundAbilities = { 1, 3, 6, 8, 11, 13, 15};
	}

	if (CombatSurvivalRankingRewardScraps.IsEmpty())
	{
		CombatSurvivalRankingRewardScraps = { 100, 70, 50, 40 };
	}

	OzInGameState = GetGameState<AOZInGameGameState>();
	if (OzInGameState)
	{
		OzInGameState->prepareStateTime = prepareStateTime;
		OzInGameState->combatStateTime = combatStateTime;
		OzInGameState->resultStateTime = roundResultShowingTime;
	}

	OZPlayerControllers = GetAllPlayerControllers(GetWorld());

	//CurrentRoundState = EGameStateType::STATE_PREP;

	TotalPlayerCount = GetNumPlayers();

	if (OzInGameState)
		OzInGameState->totalPlayerNum = TotalPlayerCount;

	SetPlayersState();

	UOZOnlineSessionSubsystem* OZSessionSubsystem = GetGameInstance()->GetSubsystem<UOZOnlineSessionSubsystem>();

	if (bIsTestMode == true)
		lastRound = lastRound;

	else if (OZSessionSubsystem != nullptr && OZSessionSubsystem->Session_NumTurns != 0)
	{
		lastRound = OZSessionSubsystem->Session_NumTurns;
	}
}

void AOZInGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsGameStarted)
		return;

	if (CurrentRoundState == EGameStateType::STATE_GAMEEND)
		return;

	if (CurrentRoundState == EGameStateType::STATE_PREP)
	{
		RoundRemainingTime -= DeltaSeconds;


		if (RoundRemainingTime <= 0)
		{
			RoundRemainingTime = prepareStateTime;
			CurrentRoundState = EGameStateType::STATE_COMBAT;

			Command_ChangeRoundState();

			
		}
	}

	/*else if (CurrentRoundState == EGameStateType::STATE_COMBAT)
	{
		CombatRemainingTime -= DeltaSeconds;

		if (CombatRemainingTime <= 0)
		{
			CombatRemainingTime = combatStateTime;
			CurrentRoundState = EGameStateType::STATE_RESULT;

			Command_ChangeRoundState();
		}
	}*/

	/*else if (CurrentRoundState == EGameStateType::STATE_RESULT)
	{
		RoundResultRemainingTime -= DeltaSeconds;


		if (RoundResultRemainingTime <= 0)
		{

			if (CurrentRound >= lastRound)
			{
				GameEnd();
				return;
			}


			RoundResultRemainingTime = roundResultShowingTime;
			CurrentRoundState = EGameStateType::STATE_PREP;

			Command_ChangeRoundState();
		}
	}*/

}

void AOZInGameMode::PrepareCombatstate()
{
	combatPlayerArray.Empty();
	CombatBonusUserNames.Empty();

	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
			if (Character)
			{
				AOZPlayerState* characterPlayerState = Cast<AOZPlayerState>(Character->GetPlayerState());
				if (characterPlayerState)
				{
					combatPlayerArray.Add(characterPlayerState);

					characterPlayerState->Round_DamageAmount = 0;
					characterPlayerState->Round_DamagedAmount = 0;
					characterPlayerState->Round_KillCount = 0;
					characterPlayerState->Round_SurvivalRanking = 1;
					characterPlayerState->RoundCombatBonus = ECombatBonus::Max;
				}

			}
		}
	}

	ResetInGameCombatState();
}

void AOZInGameMode::OnInCombatStatePlayerDead(TObjectPtr<class AOZPlayer> deadPlayer)
{
	CombatLivingPlayers--;

	AOZPlayerState* deadOzPlayerState = nullptr;

	for (int i = 0; i < combatPlayerArray.Num(); i++)
	{


		if (combatPlayerArray[i] != deadPlayer->GetPlayerState())
			continue;

		deadOzPlayerState = combatPlayerArray[i];

		if (deadOzPlayerState)
		{
			deadOzPlayerState->Round_SurvivalRanking = SurvivalRankingCount;

			if (deadOzPlayerState->LastDamageInstigatorState)
			{
				deadOzPlayerState->LastDamageInstigatorState->Round_KillCount++;
				deadOzPlayerState->LastDamageInstigatorState->Total_KillCount++;
			}

			SurvivalRankingCount--;

			AOZPlayerController* ozPlayerController = Cast<AOZPlayerController>(deadOzPlayerState->GetPlayerController());

			if(ozPlayerController != nullptr)
				ozPlayerController->ShowCombatResultPopup(false);
		}
	}

	if (CombatLivingPlayers == 1)
	{
		for (int i = 0; i < combatPlayerArray.Num(); i++)
		{
			if (combatPlayerArray[i]->Round_SurvivalRanking != 1)
				continue;

			AOZPlayerController* ozPlayerController = Cast<AOZPlayerController>(combatPlayerArray[i]->GetPlayerController());

			if (ozPlayerController != nullptr)
				ozPlayerController->ShowCombatResultPopup(true);

			break;
		}


		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AOZInGameMode::StopDarkSmoge);
	}

	else
	{
		ShowKillLog(deadOzPlayerState, CombatLivingPlayers);
	}
}

void AOZInGameMode::StopDarkSmoge()
{
	if (FogManager)
	{
		FogManager->StopFogForRound();
	}
}

void AOZInGameMode::RoundCombatEnd()
{
	CurrentRoundState = EGameStateType::STATE_RESULT;

	Command_ChangeRoundState();
}

TArray<TObjectPtr<class AOZPlayerState>> AOZInGameMode::GetOZPlayerState()
{
	return OZPlayerStates;

	/*TArray<TObjectPtr<class AOZPlayerState>> combatPlayerStates;

	for (int i = 0; i < combatPlayerArray.Num(); i++)
	{
		AOZPlayer* OzPlayer = combatPlayerArray[i];
		AOZPlayerState* OzPlayerState = Cast<AOZPlayerState>(OzPlayer->GetPlayerState());

		combatPlayerStates.Add(OzPlayerState);
	}

	OZPlayerStates = combatPlayerStates;

	return OZPlayerStates;*/
}

void AOZInGameMode::PrintNetRoll()
{
	ENetMode NetMode = GetWorld()->GetNetMode();

	FString NetModeStr;

	switch (NetMode)
	{
	case NM_Standalone:    NetModeStr = TEXT("Standalone"); break;
	case NM_ListenServer:  NetModeStr = TEXT("Listen Server"); break;
	case NM_DedicatedServer: NetModeStr = TEXT("Dedicated Server"); break;
	case NM_Client:        NetModeStr = TEXT("Client"); break;
	}

	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Yellow,
		FString::Printf(TEXT("NetMode: %s"), *NetModeStr)
	);
}


//void AOZInGameMode::TakeMinimapPicture()
//{
//	if (HasAuthority())
//	{
//		TArray<AActor*> BoundCatchers;
//		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZWorldBoundCacher::StaticClass(), BoundCatchers);
//
//		if (BoundCatchers.Num() > 0)
//		{
//			AOZWorldBoundCacher* BoundCatcher = Cast<AOZWorldBoundCacher>(BoundCatchers[0]);
//
//			if (BoundCatcher != nullptr)
//			{
//				BoundCatcher->TakeAShot();
//			}
//
//		}
//
//	}
//}

void AOZInGameMode::Command_ChangeRoundState()
{
	if (CurrentRoundState == EGameStateType::STATE_PREP)
	{
		StartRound();
	}

	else if (CurrentRoundState == EGameStateType::STATE_COMBAT)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (AOZPlayerController* PC = Cast<AOZPlayerController>(It->Get()))
			{
				PC->Client_ForceCloseSlotUIAndReturnCamera();
			}
		}

		StartComBat();
	}

	else if (CurrentRoundState == EGameStateType::STATE_RESULT)
	{
		//StartRoundResult();

		SetComBatResult();
	}
}

void AOZInGameMode::StartRound()
{
	if (bIsRoundStarted == true)
		return;

	CurrentRoundState = EGameStateType::STATE_PREP;

	if (CurrentRound != 0)
		ReSpawnPlayers();

	if (CurrentRound == 0 || CurrentRound == 1)
		MultiCastHidLoadingScreen();

	RoundRemainingTime = prepareStateTime;

	// FinalValue 초기화 및 클라이언트 동기화
	if (UOZModuleSubsystem* ModuleSubsystem = GetGameInstance()->GetSubsystem<UOZModuleSubsystem>())
	{
		ModuleSubsystem->CalculateAndCacheAllModuleFinalValues(CurrentRound);

		// 서버에서 계산된 FinalValue를 클라이언트에 동기화
		if (OzInGameState)
		{
			TArray<int32> ModuleIDs;
			TArray<float> FinalValues;
			TArray<float> MinValues;
			TArray<float> MaxValues;

			ModuleSubsystem->GetAllCachedFinalValues(ModuleIDs, FinalValues);
			ModuleSubsystem->GetAllCachedMinValues(ModuleIDs, MinValues);
			ModuleSubsystem->GetAllCachedMaxValues(ModuleIDs, MaxValues);

			OzInGameState->Multicast_SyncModuleFinalValues(ModuleIDs, FinalValues);
			OzInGameState->Multicast_SyncModuleMinValues(ModuleIDs, MinValues);
			OzInGameState->Multicast_SyncModuleMaxValues(ModuleIDs, MaxValues);
		}
	}

	CurrentRound += 1;

	// 짝수 라운드마다 모듈 FinalValue 미리 계산 및 클라이언트 동기화
	if (CurrentRound % 2 == 0)
	{
		if (UOZModuleSubsystem* ModuleSubsystem = GetGameInstance()->GetSubsystem<UOZModuleSubsystem>())
		{
			ModuleSubsystem->CalculateAndCacheAllModuleFinalValues(CurrentRound);

			// 서버에서 계산된 FinalValue를 클라이언트에 동기화
			if (OzInGameState)
			{
				TArray<int32> ModuleIDs;
				TArray<float> FinalValues;
				TArray<float> MinValues;
				TArray<float> MaxValues;

				ModuleSubsystem->GetAllCachedFinalValues(ModuleIDs, FinalValues);
				ModuleSubsystem->GetAllCachedMinValues(ModuleIDs, MinValues);
				ModuleSubsystem->GetAllCachedMaxValues(ModuleIDs, MaxValues);

				OzInGameState->Multicast_SyncModuleFinalValues(ModuleIDs, FinalValues);
				OzInGameState->Multicast_SyncModuleMinValues(ModuleIDs, MinValues);
				OzInGameState->Multicast_SyncModuleMaxValues(ModuleIDs, MaxValues);
			}
		}
	}

	bool bIsAvilityRound = RoundAbilities.Contains(CurrentRound);

	if (OzInGameState != nullptr)
	{
		OzInGameState->CurrentRoundState = CurrentRoundState;
		OzInGameState->bIsAvilityRound = bIsAvilityRound;
		OzInGameState->Multicast_OnStartRound(GetWorld()->GetTimeSeconds());

		OzInGameState->SpawnPrepareStateActors();

		OzInGameState->DeActivatePlayerWidgetComponent();

		// 서버에서 BGM 재생 (두 번째 라운드부터는 결과 BGM 페이드아웃 후 재생)
		if (UOZSoundSubsystem* SoundSubsystem = GetGameInstance()->GetSubsystem<UOZSoundSubsystem>())
		{
			if (OzInGameState->PrepareBGM)
			{
				if (CurrentRound > 1)
				{
					SoundSubsystem->StopBGMAndPlayNext(OzInGameState->PrepareBGM, 0.5f);
				}
				else
				{
					SoundSubsystem->PlayBGM(OzInGameState->PrepareBGM);
				}
			}
		}
	}

	for (int i = 0; i < OZPlayerControllers.Num(); i++)
		OZPlayerControllers[i]->OnRoundStart();


	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetShowRoundUI(CurrentRound);

		OZInGameHud->SetFloorUIState(CurrentRoundState);
		OZInGameHud->SetInvenEntriesActive(true);

		OZInGameHud->PrepareMinimap(OzInGameState, TotalPlayerCount);

		if (bIsAvilityRound)
		{
			OZInGameHud->SetInGameUI(EInGameUIType::AVILITY_UI, true);
		}

	}

	bIsRoundStarted = true;
}

void AOZInGameMode::StartComBat()
{
	// 전투단계 돌입 시 정비단계 BGM 페이드아웃
	if (UOZSoundSubsystem* SoundSubsystem = GetGameInstance()->GetSubsystem<UOZSoundSubsystem>())
	{
		SoundSubsystem->StopBGM(0.5f);
	}

	// 모든 타워를 초기 상태로 리셋
	TArray<AActor*> AllTowers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOZTower::StaticClass(), AllTowers);
	for (AActor* TowerActor : AllTowers)
	{
		if (AOZTower* Tower = Cast<AOZTower>(TowerActor))
		{
			Tower->ResetTower();
		}
	}

	if (FogManager)
	{
		FogManager->StartFogForRound();
	}

	//CombatRemainingTime = combatStateTime;
	TotalPlayerCount = GetNumPlayers();

	CombatLivingPlayers = TotalPlayerCount;
	SurvivalRankingCount = TotalPlayerCount;

	PrepareCombatstate();

	if (OzInGameState != nullptr)
	{
		OzInGameState->CurrentRoundState = CurrentRoundState;
		OzInGameState->OnCombatStateEntered.Broadcast();
		OzInGameState->Multicast_OnStartCombat(GetWorld()->GetTimeSeconds());
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->SetFloorUIState(CurrentRoundState, CurrentRound);

		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetInGameUI(EInGameUIType::ROUND_COMBAT, true);
		OZInGameHud->bIsCanUpdateMinimap = true;
		OZInGameHud->SetInvenEntriesActive(false);
	}

	if (OzInGameState)
	{
		OzInGameState->DestroyPrepareStateActors();
	}

	//첫 라운드 일 경우, 검은안개 경고 메세지 출력
	if (CurrentRound == 1)
	{
		OzInGameState->Multicast_CommonTutorialMsg(7);
		OzInGameState->Multicast_CommonTutorialMsg(8);
	}
}

void AOZInGameMode::SetComBatResult()
{
	ResultRepPlayers = 0;
	CombatRewardEndPlayers = 0;
	RoundResultRemainingTime = 99;

	TArray<TObjectPtr<AOZPlayerState>> playerStates = GetOZPlayerState();

	AddCombatReward();

	if (OzInGameState != nullptr)
	{
		OzInGameState->roundCombatBonus = CombatMaxBonusType;
		OzInGameState->bonusePlayerNames = CombatBonusUserNames;
	}

	OzInGameState->roundCombatPlayerState = playerStates;
	OzInGameState->bIsCombatResultSetted = true;

	OzInGameState->ForceNetUpdate();

	//���� ���� ��
	ResultRepPlayers++;
}

void AOZInGameMode::RecievedCombatResultRep()
{
	ResultRepPlayers++;

	if (ResultRepPlayers == TotalPlayerCount)
	{
		StartCombatResult();
		//AddCombatReward();
	}
}

void AOZInGameMode::RevievedCombatPopupEnd()
{
	CombatRewardEndPlayers++;

	if (CombatRewardEndPlayers == TotalPlayerCount)
	{
		if (CurrentRound >= lastRound)
		{
			GameEnd();
			return;
		}

		bIsRoundStarted = false;

		RoundResultRemainingTime = roundResultShowingTime;
		CurrentRoundState = EGameStateType::STATE_PREP;

		Command_ChangeRoundState();
	}
}

void AOZInGameMode::StartCombatResult()
{
	RoundResultRemainingTime = roundResultShowingTime;
	TArray<TObjectPtr<AOZPlayerState>> playerStates = GetOZPlayerState();

	if (OzInGameState != nullptr)
	{
		OzInGameState->CurrentRoundState = CurrentRoundState;
		OzInGameState->roundCombatBonus = CombatMaxBonusType;
		OzInGameState->bonusePlayerNames = CombatBonusUserNames;
		OzInGameState->Multicast_OnStartResult(GetWorld()->GetTimeSeconds());

		if (UOZSoundSubsystem* SoundSubsystem = GetGameInstance()->GetSubsystem<UOZSoundSubsystem>())
		{
			if (OzInGameState->ResultBGM)
				SoundSubsystem->PlayBGM(OzInGameState->ResultBGM);
		}
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->SetFloorUIState(CurrentRoundState);

		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->ChangeFloorUISwitcher(true);
		OZInGameHud->SetInGameUI(EInGameUIType::ROUND_COMBATRESULT, true);
		OZInGameHud->DisplayCombatResult();
		OZInGameHud->SetCombatBonusRewardPopup(CombatMaxBonusType, CombatBonusUserNames);
		OZInGameHud->bIsCanUpdateMinimap = false;
	}
}

void AOZInGameMode::AddWinPoint()
{
	int maxCore = 0;

	for (int i = 0; i < OZPlayerStates.Num(); i++)
	{
		if (maxCore < OZPlayerStates[i]->OwningCores)
			maxCore = OZPlayerStates[i]->OwningCores;
	}

	for (int i = 0; i < OZPlayerStates.Num(); i++)
	{
		if (maxCore == OZPlayerStates[i]->OwningCores)
		{
			UOZGameInstance* ozGameInstacne = OZPlayerStates[i]->GetPlayerController()->GetGameInstance<UOZGameInstance>();

			if (ozGameInstacne != nullptr)
				ozGameInstacne->TotalWinCount += 1;
		}
	}
}

void AOZInGameMode::AddCombatReward()
{
	// Get RewardSubsystem
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[AOZInGameMode] Failed to get GameInstance!"));
		return;
	}

	UOZRewardSubsystem* RewardSubsystem = GameInstance->GetSubsystem<UOZRewardSubsystem>();
	if (!RewardSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[AOZInGameMode] Failed to get RewardSubsystem!"));
		return;
	}

	CombatMaxBonusType = static_cast<ECombatBonus>(FMath::RandRange(0, static_cast<int32>(ECombatBonus::Max) - 1));
	//CombatMaxBonusType = ECombatBonus::MaxHit;

	FString lastSurvivalPlayer;
	TArray<AOZPlayerState*> CombatBonusPlayers;

	int MaxValue = 0;

	for (int i = 0; i < combatPlayerArray.Num(); i++)
	{
		AOZPlayerState* playerState = combatPlayerArray[i];

		if (playerState)
		{
			int CurrentValue = -1;

			int RoundKill = playerState->Round_KillCount;
			int SurvivalRank = playerState->Round_SurvivalRanking;
			int RoundDamage = playerState->Round_DamageAmount;
			int RoundHit = playerState->Round_DamagedAmount;

			switch (CombatMaxBonusType)
			{
			case ECombatBonus::MaxKill:
				CurrentValue = RoundKill;
				break;

			case ECombatBonus::MaxDamage:
				CurrentValue = RoundDamage;
				break;

			case ECombatBonus::MaxHit:
				CurrentValue = RoundHit;
				break;
			}

			if (CurrentValue > MaxValue)
			{
				MaxValue = CurrentValue;

				CombatBonusPlayers.Empty();
				CombatBonusPlayers.Add(playerState);

				CombatBonusUserNames.Empty();
				CombatBonusUserNames.Add(playerState->UserName);
			}

			else if (CurrentValue == MaxValue)
			{
				CombatBonusPlayers.Add(playerState);
				CombatBonusUserNames.Add(playerState->UserName);
			}

			int32 RankID = 40000 + (SurvivalRank - 1);
			FOZRewardData* RewardData = RewardSubsystem->GetRewardDataByRankID(RankID);
			if (RewardData)
			{
				playerState->OwningScraps += RewardData->Base_Reward;

				playerState->OwningScraps += RoundKill * RewardData->Kill_Reward;

				playerState->OwningScraps += RewardData->Rank_Reward;

				if (RewardData->Round_WinnerReward && SurvivalRank == 1)
				{
					playerState->OwningCores += LastSurvivorRewardCore;
					//CombatBonusUserNames.Add(playerState->UserName);
					lastSurvivalPlayer = playerState->UserName;
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[AOZInGameMode] No reward data found for SurvivalRank: %d (RankID: %d)"), SurvivalRank, RankID);
			}
		}
	}

	CombatBonusUserNames.Add(lastSurvivalPlayer);

	for (int i = 0; i < CombatBonusPlayers.Num(); i++)
	{
		CombatBonusPlayers[i]->OwningCores += CombatBonusRewardCore;
		CombatBonusPlayers[i]->RoundCombatBonus = CombatMaxBonusType;
	}
}

void AOZInGameMode::ShowKillLog(AOZPlayerState* deadPlayer, int numSurvivalPlayers)
{
	FString KillerPlayerName;
	AOZPlayerState* killerState = deadPlayer->LastDamageInstigatorState;

	if (killerState == nullptr)
		KillerPlayerName = TEXT("Suicide");

	else
		KillerPlayerName = deadPlayer->LastDamageInstigatorState->UserName;

	
	FString DeadPlayerName = deadPlayer->UserName;

	for (AOZPlayerState* currentState : combatPlayerArray)
	{
		if (AOZPlayerController* OZPlayerController = Cast<AOZPlayerController>(currentState->GetPlayerController()))
		{
			OZPlayerController->ShowKillLog(KillerPlayerName, DeadPlayerName, numSurvivalPlayers);
		}
	}
}

void AOZInGameMode::OnDarkSmogeEnterPhase(int numPhase, float ShrinkingDuration)
{
	if (OZPlayerControllers.IsEmpty())
	{
		for (AOZPlayerController* CurrOZPlayerController : GetAllPlayerControllers(GetWorld()))
		{
			CurrOZPlayerController->StartDarkSmogeShrink(ShrinkingDuration);
		}
	}

	else
	{
		for (AOZPlayerController* CurrOZPlayerController : OZPlayerControllers)
		{
			CurrOZPlayerController->StartDarkSmogeShrink(ShrinkingDuration);
		}
	}

	/*for (AOZPlayerState* currentState : combatPlayerArray)
	{
		if (AOZPlayerController* OZPlayerController = Cast<AOZPlayerController>(currentState->GetPlayerController()))
		{
			OZPlayerController->ShowDarkSmogeWarningText();
		}
	}*/
}

void AOZInGameMode::OnDarkSmogeStartCounting(float targetEndTime)
{
	if (OzInGameState == nullptr)
		return;

	OzInGameState->TargetShrinkingTime = targetEndTime;
}

void AOZInGameMode::GameEnd()
{
	CurrentRoundState = EGameStateType::STATE_GAMEEND;
	OzInGameState->CurrentRoundState = EGameStateType::STATE_GAMEEND;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (PC == nullptr)
		return;

	if (AOZInGameHUD* OZInGameHud = PC->GetHUD<AOZInGameHUD>())
	{
		OZInGameHud->ClearAllInGameUI();
		OZInGameHud->SetInGameUI(EInGameUIType::GAME_END, true);
		OZInGameHud->SetGameResultUserInfo(OZPlayerStates);
	}

	AddWinPoint();
}

TArray<TObjectPtr<class AOZPlayerController>> AOZInGameMode::GetAllPlayerControllers(UWorld* World)
{
	TArray<TObjectPtr<AOZPlayerController>> playerControllers;

	if (!World) return playerControllers;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (AOZPlayerController* PC = CastChecked< AOZPlayerController>(It->Get()))
		{
			playerControllers.Add(PC);
		}
	}

	return playerControllers;
}

void AOZInGameMode::ReSpawnPlayers()
{
	if (OZPlayerControllers.Num() == 0)
		return;

	TArray<TObjectPtr<APlayerStart>> playersStartPos = OZPlayerStartPosArray;

	for (int i = 0; i < OZPlayerControllers.Num(); i++)
	{
		APlayerController* PC = OZPlayerControllers[i];
		if (!PC) continue;

		AOZPlayerState* PS = PC->GetPlayerState<AOZPlayerState>();
		if (PS)
		{
			if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
			{
				ASC->RemoveLooseGameplayTag(OZGameplayTags::Player_State_Dead);
			}
		}

		int randomIndex = FMath::RandRange(0, playersStartPos.Num() - 1);
		APlayerStart* selectedRandomStartPos = playersStartPos[randomIndex];
		playersStartPos.RemoveAtSwap(randomIndex);

		OZPlayerControllers[i]->GetPawn()->Destroy();

		RestartPlayerAtPlayerStart(OZPlayerControllers[i], selectedRandomStartPos);

		if (AOZPlayer* NewPlayer = Cast<AOZPlayer>(PC->GetPawn()))
		{
			NewPlayer->InitASCAndAttributes();
			if (PS)
			{
				PS->SetRespawnAttributes();
			}
			if (PS && PS->CoolingAbilityClass)
			{
				if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
				{
					ASC->TryActivateAbilityByClass(PS->CoolingAbilityClass);
				}
			}
		}

		SetPlayersState();
	}

}


void AOZInGameMode::SetPlayersState()
{
	if (OZPlayerControllers.IsEmpty())
		return;

	OZPlayerStates.Empty();

	for (int i = 0; i < OZPlayerControllers.Num(); i++)
	{
		AOZPlayerState* playerState = OZPlayerControllers[i]->GetPlayerState<AOZPlayerState>();
		OZPlayerStates.Add(playerState);

		// 두건색 바꾸기
		AOZPlayer* Player = Cast<AOZPlayer>(OZPlayerControllers[i]->GetCharacter());
		if (!Player)
			continue;

		UTexture2D* BandanaTexture = nullptr;
		if (BandanaTextures.IsValidIndex(i))
		{
			BandanaTexture = BandanaTextures[i];
		}

		FLinearColor BandanaColor = FLinearColor::White;
		if (BandanaColors.IsValidIndex(i))
		{
			BandanaColor = BandanaColors[i];
		}

		Player->Multicast_SetBandanaTexture(BandanaTexture, BandanaColor);
	}
}

void AOZInGameMode::ResetInGameCombatState()
{
	OzInGameState->roundCombatPlayerState.Empty();

	OzInGameState->bIsCombatResultSetted = false;
	OzInGameState->roundCombatBonus = ECombatBonus::Max;
	OzInGameState->bonusePlayerNames.Empty();
}

void AOZInGameMode::MultiCastHidLoadingScreen()
{
	if (OzInGameState == nullptr)
		return;

	OzInGameState->HidLoadingScreen();
}

void AOZInGameMode::NotifyPlayerFinishedCutscene(APlayerController* FinishedPC)
{
	if (!HasAuthority())
		return;

	if (!FinishedPC)
		return;

	FinishedCutscenePlayers.Add(FinishedPC);

	const int32 Total = GetNumPlayers();
	const int32 Done = FinishedCutscenePlayers.Num();

	UE_LOG(LogTemp, Warning, TEXT("[Cutscene] Done %d / Total %d"), Done, Total);

	if (Done >= Total)
	{
		bIsGameStarted = true;
		StartRound();
	}
}
