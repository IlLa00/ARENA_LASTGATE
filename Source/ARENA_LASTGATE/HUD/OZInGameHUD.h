// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/OZHud.h"
#include "Utils/Util.h"
#include "OZInGameHUD.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZInGameHUD : public AOZHud
{
	GENERATED_BODY()
	
public:

	bool bIsCanUpdateMinimap = false;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|InGameUIArray", meta = (ToolTip = "0 : RoundPrepareUI"))
	TArray<TObjectPtr<UUserWidget>> InGameUIArray;

	UFUNCTION(BlueprintCallable)
	void PostBeginPlay();

	UFUNCTION(BlueprintCallable)
	void SetInGameUI(EInGameUIType UiType, bool active);

	UFUNCTION(BlueprintCallable)
	void ClearAllInGameUI(bool bAlsoClearFloorUI = false);

	//Floor UI Function
	void SetFloorRoundNum(uint8 roundNum);
	void SetFloorUIState(EGameStateType currGameState, int numRound = 0);

	void SetInvenEntriesActive(bool active);

	void SetIsOnCutScene(bool isOnCutScene);

	//Round Prepare UIFunction
	void SetShowRoundUI(uint8 roundNum);
	void ToggleShopUI();

	void UpdateCombatTimer(float targetTime);
	void UpdateRemainTime(float ServerStartStamp, float ClientDelayedStamp, float StateTotalTime);

	//void SetRemainTimeCombat();

	void DisplayCombatResult();

	//Combat UI Function
	void InitCombatMinimap(FVector worldCenter, FVector mapSize);
	void OnPlayerDie();

	void PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers);
	void UpdateMinimap(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor);
	void SetActivateTowerVision(bool bActivate);
	void ShowCombatKillLog(FString KillerPlayerName, FString DeadPlayerName, int numSurvivalPlayers);
	void ShowBlackSmogeWarning();
	void SetBlackSmogeShrinking(float shrinkingTime);
	void ShowCombatResultPopupWidget(bool bIsWin);

	void SetCombatBonusRewardPopup(ECombatBonus bonuseType, TArray<FString> bonusePlayerNames);

	void SetGameResultUserInfo(TArray<TObjectPtr<class AOZPlayerState>> ozPlayerStates);

	void ToggleMinimapUI();

	UFUNCTION()
	void OnCombatResultFinished(bool bIsWin);


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class UOZCombatResultPopupWidget> CombatResultPopupWidget;

	UFUNCTION()
	void UpdateCurrentStamina(float ratio);
	/// ///////////


	UFUNCTION(BlueprintCallable, Category = "UI")
	class UOZInGameFloorUI* GetInGameFloorUI() const { return inGameFloorUI; }

	void ChangeFloorUISwitcher(bool bisAlive);

	bool bIsMinimapOpend = true;

private:
	bool bIsShopOpen = false;


	//Tick���� ĳ��Ʈ �ϱ� �δ��� ũ��, �ϴ� ĳġ�ؼ� ���
	TObjectPtr<class UOZInGameFloorUI> inGameFloorUI = nullptr;
	TObjectPtr<class AOZInGameGameState> inGameState = nullptr;
	TObjectPtr<class UOZRoundInComBatUI> inGameCombatUI = nullptr;
};
