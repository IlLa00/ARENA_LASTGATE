// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZRoundInComBatUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZRoundInComBatUI : public UOZUI
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|KillLogMainTainTime")
	float KillLogShowingTime = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|DarkSmogeWarningMainTainTime")
	float DarkSmogeWarningShowingTime = 2.0f;

	UFUNCTION(BlueprintImplementableEvent)
	void SetAimHeatRatio(float ratio);

	UFUNCTION(BlueprintImplementableEvent)
	void SetAimStaminaRatio(float value);

	void InitMinimapWidget(FVector worldCenter, FVector mapSize, float fogInitSize);
	void SetTowerVisionArea(TArray<TPair<FVector, float>> towerVisions);

	//void UpdateMinimapIcons(TSet<AActor*> currViewActors);

	void UpdateMinimapIcons(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor);
	void SetActivitionMinimapTowerVision(bool bISActivated);
	void PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers);
	void OnPlayerDie();

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<class UOZMinimapWidget> MinimapWidget = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<class UOZFollowMinimap> FollowMinimapWidget = nullptr;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowKillLog(const FString& KillerName, const FString& KilledName, int numSurvivalPlayers);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDarkSmogeWarning();

	void OnStartShrinking(float shrinkingTime);

	void ToggleMinimap(bool bIsMinimapOpen);
};
