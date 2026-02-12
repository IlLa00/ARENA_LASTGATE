// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZRoundInComBatUI.h"
#include "Widget/OZAimWidget.h"
#include "Components/TextBlock.h"
#include "GameState/OZInGameGameState.h"
#include "Widget/OZMinimapWidget.h"
#include "Widget/OZFollowMinimap.h"

void UOZRoundInComBatUI::InitMinimapWidget(FVector worldCenter, FVector mapSize, float fogInitSize)
{
	if (!MinimapWidget.IsValid())
		return;

	MinimapWidget->SetWorldPresetParmas(worldCenter, mapSize, fogInitSize, FollowMinimapWidget);
}

void UOZRoundInComBatUI::SetTowerVisionArea(TArray<TPair<FVector, float>> towerVisions)
{
	if (!MinimapWidget.IsValid())
		return;

	MinimapWidget->SetMinimapTowerVisionArea(towerVisions);
}

void UOZRoundInComBatUI::UpdateMinimapIcons(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor)
{
	if (!MinimapWidget.IsValid())
		return;

	MinimapWidget->UpdateMinimapIcons(AddedActor, RemovedActor);
}

void UOZRoundInComBatUI::SetActivitionMinimapTowerVision(bool bISActivated)
{
	if (!MinimapWidget.IsValid())
		return;

	if (bISActivated)
	{
		MinimapWidget->ActivateTowerVision();
	}

	else
		MinimapWidget->DeActivateTowerVision();
	//MinimapWidget
}

void UOZRoundInComBatUI::PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers)
{
	if (!MinimapWidget.IsValid())
		return;

	MinimapWidget->PrepareMinimap(ozIngameState, numPlayers);
}

void UOZRoundInComBatUI::OnPlayerDie()
{
	if (!MinimapWidget.IsValid())
		return;

	MinimapWidget->OnPlayerDie();
}

void UOZRoundInComBatUI::OnStartShrinking(float shrinkingTime)
{
	MinimapWidget->SetShrinking(shrinkingTime);
}

void UOZRoundInComBatUI::ToggleMinimap(bool bIsMinimapOpen)
{
	if (bIsMinimapOpen)
	{
		//MinimapWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		MinimapWidget->SetRenderOpacity(1.0f);
		FollowMinimapWidget->SetVisibility(ESlateVisibility::Hidden);
		FollowMinimapWidget->bISVisible = false;
	}

	else
	{
		//위젯은 Hidden일 경우 Tick 조차 돌지 않음
		//MinimapWidget->SetVisibility(ESlateVisibility::Hidden);
		MinimapWidget->SetRenderOpacity(0.0f);
		FollowMinimapWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		FollowMinimapWidget->bISVisible = true;

		FollowMinimapWidget->AwakeFollowMinimap();
	}
}

//void UOZRoundInComBatUI::UpdateMinimapIcons(TSet<AActor*> currViewActors)
//{
//	WBP_MinimapWidget->SetDisplayedActors(currViewActors);
//}
