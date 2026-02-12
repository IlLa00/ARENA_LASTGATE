// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OZInGameHUD.h"
#include "UserInterface/OZRoundPrepareUI.h"
#include "UserInterface/OZShopUI.h"
#include "UserInterface/OZRoundInComBatUI.h"
#include "UserInterface/OZRoundComBatResultUI.h"
#include "UserInterface/OZInGameFloorUI.h"
#include "UserInterface/OZAbilityUI.h"
#include "GameState/OZInGameGameState.h"
#include "Character/OZPlayerState.h"
#include "Utils/Util.h"
#include "Widget/OZCombatResultInfoBar.h"
#include "Character/OZPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Object/OZBlackFogManager.h"
#include "Widget/OZCombatResultPopupWidget.h"
#include "Object/OZTowerVisionArea.h"
#include "UserInterface/OZGameEndUI.h"
#include "Subsystem/OZInGameTutorialPopupSubSystem.h"
#include "Components/ScaleBox.h"
#include "Components/ProgressBar.h"

void AOZInGameHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (inGameState)
	{
		EGameStateType CurrRoundState = inGameState->CurrentRoundState;

		float ServerStartStamp = 0;
		float ClientDelayedStamp = 0;
		float StateTotalTime = 0;

		switch (CurrRoundState)
		{
		case EGameStateType::None:
			break;
		case EGameStateType::STATE_PREP:
			ServerStartStamp = inGameState->RoundStartTime;
			ClientDelayedStamp = inGameState->ClientServerRoundStartDelayTime;
			StateTotalTime = inGameState->prepareStateTime;
			//UpdateRemainRoundPrepareTime(DeltaTime);
			break;
		case EGameStateType::STATE_COMBAT:
			ServerStartStamp = inGameState->CombatStartTime;
			ClientDelayedStamp = inGameState->ClientServerCombatStartDelayTime;
			StateTotalTime = inGameState->combatStateTime;

			UpdateCombatTimer(inGameState->TargetShrinkingTime);

			//UpdateRemainCombatTime(DeltaTime);
			break;
		case EGameStateType::STATE_RESULT:
			ServerStartStamp = inGameState->ResultStartTime;
			ClientDelayedStamp = inGameState->ClientServerResultDelayTime;
			StateTotalTime = inGameState->resultStateTime;
			//UpdateRemainRoundResultTime(DeltaTime);
			break;
		case EGameStateType::MAX:
			break;
		default:
			break;
		}

		if (CurrRoundState != EGameStateType::STATE_COMBAT)
			UpdateRemainTime(ServerStartStamp, ClientDelayedStamp, StateTotalTime);

	}
}

void AOZInGameHUD::PostBeginPlay()
{
	inGameFloorUI = CastChecked<UOZInGameFloorUI>(InGameUIArray[6]);

	inGameState = Cast<AOZInGameGameState>(GetWorld()->GetGameState());

	SetInGameUI(EInGameUIType::INGAMEFLOORUI, true);

	if (OZPlayer != nullptr)
	{
		OZPlayer->OnCurrentStaminaChanged.AddDynamic(this,&AOZInGameHUD::UpdateCurrentStamina);
		//OZPlayer->OnMaxStaminaChanged.AddDynamic(this, &AOZInGameHUD::UpdateMaxStamina);
	}

	if (UOZInGameTutorialPopupSubSystem* tutorialPopupSubSystem = OZPlayer->GetGameInstance()->GetSubsystem<UOZInGameTutorialPopupSubSystem>())
	{
		tutorialPopupSubSystem->inGameFloorUI = inGameFloorUI;
		tutorialPopupSubSystem->BindPopupEndEvent();
	}
	
}

void AOZInGameHUD::SetInGameUI(EInGameUIType UiType, bool active)
{
	int32 index = (int32)UiType;
	if (!InGameUIArray.IsValidIndex(index) || InGameUIArray[index] == nullptr)
	{
		return;
	}

	if (active == true)
	{
		switch (UiType)
		{
		case EInGameUIType::ROUND_PREPARE:

			InGameUIArray[0]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;
		case EInGameUIType::SHOP_UI:
			InGameUIArray[1]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			if (AOZPlayerController* PC = Cast<AOZPlayerController>(OZPlayer))
			{
				PC->bBlockFireByUI = true;
			}
			break;
		case EInGameUIType::AVILITY_UI:
			InGameUIArray[2]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Cast<UOZAbilityUI>(InGameUIArray[2])->UpdateConverts();
			Cast<UOZAbilityUI>(InGameUIArray[2])->OnShowAbilityUICall();
			SetInputModeUIOnly(OZPlayer, InGameUIArray[2], true);
			break;
		case EInGameUIType::ROUND_COMBAT:
			InGameUIArray[3]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			SetInputModeGameOnly(OZPlayer, false);
			break;
		case EInGameUIType::ROUND_COMBATRESULT:
			SetInGameUI(EInGameUIType::INGAMEFLOORUI, false);
			InGameUIArray[4]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			SetInputModeUIANDGame(OZPlayer, true);
			break;
		case EInGameUIType::GAME_END:
			ClearAllInGameUI(true);
			InGameUIArray[5]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			SetInputModeUIOnly(OZPlayer, InGameUIArray[5], true);
			break;
		case EInGameUIType::INGAMEFLOORUI:
			InGameUIArray[6]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	else if (active == false)
	{
		switch (UiType)
		{
		case EInGameUIType::ROUND_PREPARE:
			InGameUIArray[0]->SetVisibility(ESlateVisibility::Hidden);
			break;
		case EInGameUIType::SHOP_UI:
			InGameUIArray[1]->SetVisibility(ESlateVisibility::Hidden);
			if (AOZPlayerController* PC = Cast<AOZPlayerController>(OZPlayer))
			{
				PC->bBlockFireByUI = false;
			}
			break;
		case EInGameUIType::AVILITY_UI:
			InGameUIArray[2]->SetVisibility(ESlateVisibility::Hidden);
			SetInputModeUIANDGame(OZPlayer, true);
			break;
		case EInGameUIType::ROUND_COMBAT:
			InGameUIArray[3]->SetVisibility(ESlateVisibility::Hidden);
			break;
		case EInGameUIType::ROUND_COMBATRESULT:
			InGameUIArray[4]->SetVisibility(ESlateVisibility::Hidden);
			break;
		case EInGameUIType::GAME_END:
			InGameUIArray[5]->SetVisibility(ESlateVisibility::Hidden);
			SetInputModeUIANDGame(OZPlayer, true);
			break;
		case EInGameUIType::INGAMEFLOORUI:
			InGameUIArray[6]->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AOZInGameHUD::ClearAllInGameUI(bool bAlsoClearFloorUI)
{
	int targetIndex = bAlsoClearFloorUI ? InGameUIArray.Num() : InGameUIArray.Num() - 1;
	//Floor UI�� ����
	for (int i = 0; i < targetIndex; i++)
	{
		InGameUIArray[i]->SetVisibility(ESlateVisibility::Hidden);
	}

	bIsShopOpen = false;

	if (AOZPlayerController* PC = Cast<AOZPlayerController>(OZPlayer))
	{
		PC->bBlockFireByUI = false;
	}

	bIsMinimapOpend = true;

	if (inGameCombatUI != nullptr)
		inGameCombatUI->ToggleMinimap(bIsMinimapOpend);


	SetInputModeUIANDGame(OZPlayer, true);
}

//void AOZInGameHUD::ClearAllInGameUI()
//{
//	//Floor UI�� ����
//	for (int i = 0; i < InGameUIArray.Num() - 1; i++)
//	{
//		InGameUIArray[i]->SetVisibility(ESlateVisibility::Hidden);
//	}
//
//	bIsShopOpen = false;
//
//	bIsMinimapOpend = true;
//
//	if(inGameCombatUI != nullptr)
//		inGameCombatUI->ToggleMinimap(bIsMinimapOpend);
//	
//
//	SetInputModeUIANDGame(OZPlayer, true);
//}

void AOZInGameHUD::SetFloorRoundNum(uint8 roundNum)
{
	inGameFloorUI->SetCurrentRound(roundNum);
}

void AOZInGameHUD::SetFloorUIState(EGameStateType currGameState, int numRound)
{
	inGameFloorUI->SetCurrentGameState(currGameState, numRound);
}

void AOZInGameHUD::SetInvenEntriesActive(bool active)
{
	inGameFloorUI->SetActiveInvenEntries(active);
}

void AOZInGameHUD::SetShowRoundUI(uint8 roundNum)
{
	SetInGameUI(EInGameUIType::INGAMEFLOORUI, true);
	SetInGameUI(EInGameUIType::ROUND_PREPARE, true);

	if (InGameUIArray[0] != nullptr)
	{
		Cast<UOZRoundPrepareUI>(InGameUIArray[0])->SetRoundNum(roundNum);
		Cast<UOZRoundPrepareUI>(InGameUIArray[0])->ShowRoundNum();
	}

	UOZRoundComBatResultUI* combatResultUI = Cast< UOZRoundComBatResultUI>(InGameUIArray[4]);

	if(combatResultUI != nullptr)
		combatResultUI->OnRoundcombatResultUIHide();
}

void AOZInGameHUD::ToggleShopUI()
{
	bIsShopOpen = !bIsShopOpen;

	SetInGameUI(EInGameUIType::SHOP_UI, bIsShopOpen);
}

void AOZInGameHUD::UpdateRemainTime(float ServerStartStamp, float ClientDelayedStamp, float StateTotalTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Elapsed = CurrentTime + ClientDelayedStamp - ServerStartStamp;
	float Remain = StateTotalTime - Elapsed;

	Remain = (Remain < 0) ? 0 : Remain;

	uint8 Remain_Int = static_cast<uint8>(Remain);

	if (inGameFloorUI == nullptr)
	{
		Cast<UOZInGameFloorUI>(InGameUIArray[6])->SetCurrentRemainTime(Remain_Int);
	}

	else
	{
		inGameFloorUI->SetCurrentRemainTime(Remain_Int);
	}
}

void AOZInGameHUD::UpdateCombatTimer(float targetTime)
{
	float CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();;
	float Remain = targetTime - CurrentTime;

	Remain = (Remain < 0) ? 0 : Remain;

	if (inGameFloorUI == nullptr)
	{
		Cast<UOZInGameFloorUI>(InGameUIArray[6])->SetCombatTimer(Remain);
	}

	else
	{
		inGameFloorUI->SetCombatTimer(Remain);
	}
}

//void AOZInGameHUD::SetRemainTimeCombat()
//{
//	//None USe
//	inGameFloorUI->SetCurrentRemainTime(-99);
//}

void AOZInGameHUD::DisplayCombatResult()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	AGameStateBase* GS = World->GetGameState();
	if (GS == nullptr)
		return;

	AOZInGameGameState* InGameState = Cast<AOZInGameGameState>(GS);
	if (InGameState == nullptr)
		return;


	TArray<TObjectPtr<AOZPlayerState>> RoundCombatPlayerState = InGameState->roundCombatPlayerState;
	UOZRoundComBatResultUI* combatResultUI = Cast< UOZRoundComBatResultUI>(InGameUIArray[4]);

	// 등수대로 정렬
	RoundCombatPlayerState.Sort([](const TObjectPtr<AOZPlayerState>& A, const TObjectPtr<AOZPlayerState>& B)
		{
			return A->Round_SurvivalRanking < B->Round_SurvivalRanking;
		});

	// 최대 DamageAmount와 DamagedAmount 찾기
	float MaxDamageAmount = 0.0f;
	float MaxDamagedAmount = 0.0f;

	for (const auto& playerState : RoundCombatPlayerState)
	{
		if (playerState)
		{
			MaxDamageAmount = FMath::Max(MaxDamageAmount, static_cast<float>(playerState->Round_DamageAmount));
			MaxDamagedAmount = FMath::Max(MaxDamagedAmount, static_cast<float>(playerState->Round_DamagedAmount));
		}
	}

	int userIndex = 0;

	for (int i = 0; i < RoundCombatPlayerState.Num(); i++)
	{
		AOZPlayerState* playerState = RoundCombatPlayerState[i];

		if (combatResultUI)
		{
			if (combatResultUI->CombatResultBars.IsValidIndex(i))
			{
				UOZCombatResultInfoBar* bar = combatResultUI->CombatResultBars[i];

				//최소 4자리 보장
				FNumberFormattingOptions FormatOptions;

				FormatOptions.MinimumIntegralDigits = 4;

				if (bar)
				{
					bar->assingedPlayerState = playerState;

					bar->RoundRanking = playerState->Round_SurvivalRanking;
					bar->Text_RoundRank->SetText(FText::AsNumber(playerState->Round_SurvivalRanking));

					bar->userName = playerState->UserName;
					bar->Text_PlayerName->SetText(FText::FromString(playerState->UserName));

					bar->RoundKill = playerState->Round_KillCount;
					bar->Text_RoundKill->SetText(FText::AsNumber(playerState->Round_KillCount));

					bar->Text_RoundDamageAmount->SetText(FText::AsNumber((int)playerState->Round_DamageAmount, &FormatOptions));
					bar->Text_RoundDamagedAmount->SetText(FText::AsNumber((int)playerState->Round_DamagedAmount, &FormatOptions));
					bar->Text_Core->SetText(FText::AsNumber(playerState->OwningCores));

					float DamagePercent = (MaxDamageAmount > 0.0f) ? (static_cast<float>(playerState->Round_DamageAmount) / MaxDamageAmount) : 0.0f;
					float DamagedPercent = (MaxDamagedAmount > 0.0f) ? (static_cast<float>(playerState->Round_DamagedAmount) / MaxDamagedAmount) : 0.0f;

					{
						if (DamagePercent >= 1)
							bar->ProgreeBar_RoundDamageAmount->SetFillColorAndOpacity(FLinearColor::Yellow);
						else
							bar->ProgreeBar_RoundDamageAmount->SetFillColorAndOpacity(FLinearColor::White);

						bar->ProgreeBar_RoundDamageAmount->SetPercent(DamagePercent);

						if (DamagedPercent >= 1)
							bar->ProgreeBar_RoundDamagedAmount->SetFillColorAndOpacity(FLinearColor::Yellow);
						else
							bar->ProgreeBar_RoundDamagedAmount->SetFillColorAndOpacity(FLinearColor::White);

						bar->ProgreeBar_RoundDamagedAmount->SetPercent(DamagedPercent);

						if (playerState->Round_SurvivalRanking == 1)
							bar->SetbIsHighlighted(true);
					}
					

					if (playerState->Round_SurvivalRanking != 1)
					{
						bar->Image_CrownImage->SetVisibility(ESlateVisibility::Collapsed);
						bar->Image_RoundKill->SetVisibility(ESlateVisibility::Collapsed);
						bar->Image_RoundDamageAmount->SetVisibility(ESlateVisibility::Collapsed);
						bar->Image_RoundDamagedAmount->SetVisibility(ESlateVisibility::Collapsed);
						bar->Image_Core->SetVisibility(ESlateVisibility::Collapsed);
						bar->Text_SurvivalRank->SetVisibility(ESlateVisibility::Collapsed);
						bar->TextBloc_KillBonus->SetVisibility(ESlateVisibility::Collapsed);

						bar->SetbIsHighlighted(false);
					}

					userIndex = i;
				}
			}
		}
	}

	userIndex++;

	for (userIndex; userIndex < 4; userIndex++)
	{
		if (combatResultUI)
		{
			combatResultUI->CombatResultBars[userIndex]->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AOZInGameHUD::SetIsOnCutScene(bool isOnCutScene)
{
	inGameFloorUI->SetIsOnCutScene(isOnCutScene);
}

void AOZInGameHUD::InitCombatMinimap(FVector worldCenter, FVector mapSize)
{
	if (InGameUIArray.IsEmpty())
		return;

	float fogInitSize = 7100;

	TArray<AActor*> FogActors;
	UGameplayStatics::GetAllActorsOfClass(OZPlayer->GetWorld(), AOZBlackFogManager::StaticClass(), FogActors);

	if (FogActors.Num() > 0)
	{
		AOZBlackFogManager* FogManager = Cast<AOZBlackFogManager>(FogActors[0]);

		if (FogManager != nullptr)
		{
			fogInitSize = FogManager->GetInitialMapRadiusCm();
		}

	}

	inGameCombatUI = Cast<UOZRoundInComBatUI>(InGameUIArray[3]);
	if (inGameCombatUI)
	{
		inGameCombatUI->InitMinimapWidget(worldCenter, mapSize, fogInitSize);
	}

	TArray<AActor*> TowerVisionAreas;
	UGameplayStatics::GetAllActorsOfClass(OZPlayer->GetWorld(), AOZTowerVisionArea::StaticClass(), TowerVisionAreas);

	if (TowerVisionAreas.Num() > 0)
	{
		TArray<TPair<FVector, float>> TowerVisonInfos;

		for (int i = 0; i < TowerVisionAreas.Num(); i++)
		{
			AOZTowerVisionArea* currVisionArea = Cast<AOZTowerVisionArea>(TowerVisionAreas[i]);

			if (currVisionArea == nullptr)
				continue;

			TPair<FVector, float> towerVision = MakeTuple(currVisionArea->GetActorLocation(), currVisionArea->VisionRadius);

			TowerVisonInfos.Add(towerVision);
		}


		inGameCombatUI->SetTowerVisionArea(TowerVisonInfos);
	}
}

void AOZInGameHUD::OnPlayerDie()
{
	inGameCombatUI = Cast<UOZRoundInComBatUI>(InGameUIArray[3]);
	if (inGameCombatUI)
	{
		bIsCanUpdateMinimap = false;
		inGameCombatUI->OnPlayerDie();
	}
}

void AOZInGameHUD::PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers)
{
	if (inGameCombatUI == nullptr)
		return;

	inGameCombatUI->PrepareMinimap(ozIngameState, numPlayers);
	bIsCanUpdateMinimap = true;
}

void AOZInGameHUD::UpdateMinimap(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor)
{
	if (bIsCanUpdateMinimap == true)
		inGameCombatUI->UpdateMinimapIcons(AddedActor, RemovedActor);

}

void AOZInGameHUD::SetActivateTowerVision(bool bActivate)
{
	if (inGameCombatUI == nullptr)
		return;

	inGameCombatUI->SetActivitionMinimapTowerVision(bActivate);

}

void AOZInGameHUD::ShowCombatKillLog(FString KillerPlayerName, FString DeadPlayerName, int numSurvivalPlayers)
{
	if (inGameCombatUI == nullptr)
		return;

	inGameCombatUI->ShowKillLog(KillerPlayerName, DeadPlayerName, numSurvivalPlayers);
}

void AOZInGameHUD::ShowBlackSmogeWarning()
{
	if (inGameCombatUI == nullptr)
		return;

	inGameCombatUI->ShowDarkSmogeWarning();
}

void AOZInGameHUD::SetBlackSmogeShrinking(float shrinkingTime)
{
	if (inGameCombatUI == nullptr)
		return;

	inGameCombatUI->OnStartShrinking(shrinkingTime);
}

void AOZInGameHUD::ShowCombatResultPopupWidget(bool bIsWin)
{
	if (CombatResultPopupWidget == nullptr)
		return;

	UOZCombatResultPopupWidget* PopupWidget = CreateWidget<UOZCombatResultPopupWidget>(OZPlayer, CombatResultPopupWidget);

	if (!PopupWidget)
		return;

	PopupWidget->CombatResultDelegate.Clear();
	PopupWidget->CombatResultDelegate.AddDynamic(this, &AOZInGameHUD::OnCombatResultFinished);
	PopupWidget->CombatResultDelegate.AddDynamic(inGameFloorUI, &UOZInGameFloorUI::ChangeFloorUI);

	PopupWidget->AddToViewport(100);
	PopupWidget->ShowCombatResult(bIsWin);
	PopupWidget->PlayResultSound(bIsWin);
}

void AOZInGameHUD::SetCombatBonusRewardPopup(ECombatBonus bonuseType, TArray<FString> bonusePlayerNames)
{
	UOZRoundComBatResultUI* combatResultUI = Cast< UOZRoundComBatResultUI>(InGameUIArray[4]);
	if (combatResultUI == nullptr)
		return;

	combatResultUI->SetCombatBonusPopup(bonuseType, bonusePlayerNames);
}

void AOZInGameHUD::SetGameResultUserInfo(TArray<TObjectPtr<class AOZPlayerState>> ozPlayerStates)
{
	Cast<UOZGameEndUI>(InGameUIArray[5])->SetGameResult(ozPlayerStates);
}

void AOZInGameHUD::ToggleMinimapUI()
{
	bIsMinimapOpend = !bIsMinimapOpend;

	inGameCombatUI->ToggleMinimap(bIsMinimapOpend);
}

void AOZInGameHUD::OnCombatResultFinished(bool bIsWin)
{
	if (bIsWin == false)
	{
		return;
	}

	else if (bIsWin == true)
	{
		OZPlayer->ReportRoundCombatEnd();
	}

	// HUD가 인게임을 안다 여기서 바꾸면되고
}

void AOZInGameHUD::UpdateCurrentStamina(float ratio)
{
	inGameCombatUI->SetAimStaminaRatio(ratio);
}

void AOZInGameHUD::ChangeFloorUISwitcher(bool bisAlive)
{
	inGameFloorUI->ChangeFloorUI(bisAlive);
}
