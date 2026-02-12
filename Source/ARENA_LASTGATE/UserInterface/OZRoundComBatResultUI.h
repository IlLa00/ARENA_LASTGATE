// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZRoundComBatResultUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZRoundComBatResultUI : public UOZUI
{
	GENERATED_BODY()

public:
	//void SetRoundResultRemainTime(uint8 remainingTime);
	virtual void NativeConstruct() override
	{
		Super::NativeConstruct();

		CombatResultBars.Add(User1CombatResultInfoBar);
		CombatResultBars.Add(User2CombatResultInfoBar);
		CombatResultBars.Add(User3CombatResultInfoBar);
		CombatResultBars.Add(User4CombatResultInfoBar);
	}
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOZCombatResultInfoBar> User1CombatResultInfoBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOZCombatResultInfoBar> User2CombatResultInfoBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOZCombatResultInfoBar> User3CombatResultInfoBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UOZCombatResultInfoBar> User4CombatResultInfoBar;

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<class UOZCombatResultInfoBar>> CombatResultBars;

	UPROPERTY(EditAnywhere)
	float RewardTimeLine = 1.0f;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float LastSurvivalBonusePopupDelay = 1.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float  LastSurvivalBonusePopupRemain = 2.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RandomMaxBonusePopupDelay = 1.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RandomMaxBonusePopupRemain = 2.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PopupEndDelay = 2.f;

public:
	UPROPERTY(BlueprintReadWrite)
	ECombatBonus combatBonusType = ECombatBonus::Max;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> bonusePlayerNameArray;

	void SetCombatBonusPopup(ECombatBonus bonuseType, TArray<FString> bonusePlayerNames);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowCombatBonusePopup();

	UFUNCTION(BlueprintCallable)
	void ReportCombatBonusRewardPopupEnd();

	UFUNCTION(BlueprintCallable)
	void OnRewardPopupDisapearAnimStarted(TArray<FString> rewaredPlayerNames, FVector2D startPos, const FString& upperName);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayRewardCoreAnim(FVector2D startPos, FVector2D endPos, float timeLine, int assingedIndex, const FString& upperName);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRoundcombatResultUIHide();

private:
	void HighlightWinner();
	void DisaplyPreBonuscores();
	void PrefareBonuseTextBlock();


	FVector2D FirstWinnerTargetPos = FVector2D(0.f, 0.f);
};
