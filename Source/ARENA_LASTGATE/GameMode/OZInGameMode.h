// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Util.h"
#include "GameFramework/GameMode.h"
#include "OzBaseGameMode.h"
#include "Object/OZTaggableActor.h"
#include "OZInGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZInGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

private:
	bool bIsGameStarted = false;
	uint8 CombatLivingPlayers = -1;

private:

	bool bIsRoundStarted = false;

	//Move To
	void Command_ChangeRoundState();

	//Judge Value
	bool bAllPlayersReady = false;
	uint8 ReadyPlayerCount = 0;
	uint8 TotalPlayerCount = 0;

	EGameStateType CurrentRoundState = EGameStateType::None;

	//Prepare State Value
	uint8 CurrentRound = -1;
	float RoundRemainingTime = -1;

	//Combat State
	TArray<TObjectPtr<class AOZPlayerState>> combatPlayerArray;
	void PrepareCombatstate();
	uint8 SurvivalRankingCount = 0;

	uint8 ResultRepPlayers = 0;
	uint8 CombatRewardEndPlayers = 0;

	void AddCombatReward();

	void ShowKillLog(AOZPlayerState* deadPlayer, int numSurvivalPlayers);

	UFUNCTION()
	void OnDarkSmogeEnterPhase(int numPhase, float ShrinkingDuration);

	UFUNCTION()
	void OnDarkSmogeStartCounting(float targetEndTime);

	//Round Result Value
	float RoundResultRemainingTime = -1;

	void InitializeGame();

	void StartRound();
	void StartComBat();
	void SetComBatResult();
	void StartCombatResult();
	void AddWinPoint();
	void GameEnd();

	ECombatBonus CombatMaxBonusType = ECombatBonus::Max;
	TArray<FString> CombatBonusUserNames; // �迭�� �� �������� ������ �̸�
	TArray<TObjectPtr<class AOZPlayerController>> OZPlayerControllers;
	TArray<TObjectPtr<class AOZPlayerController>> GetAllPlayerControllers(UWorld* World);


	void ReSpawnPlayers();
	TObjectPtr<class AOZInGameGameState> OzInGameState = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void StartGameMode() override;

public:
	UPROPERTY(EditAnywhere, Category = "OZ|TestMode")
	bool bIsTestMode = false;

	AOZInGameMode();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY()
	TArray<TObjectPtr<class APlayerStart>> OZPlayerStartPosArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|PrepareStateTime")
	float prepareStateTime = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|CombatStateTime")
	float combatStateTime = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|RoundResultTime")
	float roundResultShowingTime = 7;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|MaxRound")
	uint8 lastRound = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|AvilityRound")
	TArray<uint8> RoundAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|CombatReward")
	uint8 LastSurvivorRewardCore = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|CombatReward")
	int CombatBonusRewardCore = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|CombatReward")
	TArray<int32> CombatSurvivalRankingRewardScraps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|CombatReward")
	int CombatKillRewardScraps = 50;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "OZ|TargetInGameLevel")
	TSoftObjectPtr<UWorld> TravleLobbyLevel;

	UPROPERTY()
	class AOZBlackFogManager* FogManager = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|PlayerCustomization")
	TArray<UTexture2D*> BandanaTextures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|PlayerCustomization")
	TArray<FLinearColor> BandanaColors;

	void OnInCombatStatePlayerDead(TObjectPtr<class AOZPlayer> deadPlayer);
	void StopDarkSmoge();
	void RoundCombatEnd();

	void RecievedCombatResultRep();

	void RevievedCombatPopupEnd();

	void NotifyPlayerFinishedCutscene(APlayerController* FinishedPC);
private:
	void SetPlayersState();

	TArray<TObjectPtr<class AOZPlayerState>> OZPlayerStates;
	TArray<TObjectPtr<class AOZPlayerState>> GetOZPlayerState();

	//void TakeMinimapPicture();

	void PrintNetRoll();

	void ResetInGameCombatState();

	void MultiCastHidLoadingScreen();

	TSet<TWeakObjectPtr<APlayerController>> FinishedCutscenePlayers;
};
