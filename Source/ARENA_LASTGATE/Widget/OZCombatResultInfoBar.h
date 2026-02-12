// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "OZCombatResultInfoBar.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZCombatResultInfoBar : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite)
	int RoundRanking = 0;

	UPROPERTY(BlueprintReadWrite)
	FString userName;

	UPROPERTY(BlueprintReadWrite)
	int RoundKill = 0;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AOZPlayerState> assingedPlayerState = nullptr;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoundRank;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_CrownImage;

	/*UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScaleBox> ScaleBox_ImageCore;*/

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Core;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoundKill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_RoundKill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_RoundDamageAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoundDamageAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgreeBar_RoundDamageAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_RoundDamagedAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RoundDamagedAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgreeBar_RoundDamagedAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Core;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_SurvivalRank;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBloc_KillBonus;
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void ToggleDisplayedParams(bool bIsShowPercentage);

	UFUNCTION(BlueprintCallable)
	void CallToggleDisplayedParams(bool bIsShowPercentage);

	UFUNCTION(BlueprintImplementableEvent)
	void SetBonusScrap(int32 survivalBonusScrap, int32 KillBonusScrap);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UImage> Image_RewardEffect = nullptr;

	UFUNCTION(BlueprintImplementableEvent)
	FVector2D GetRewardEffectPos(int32 assingedIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void ReturnInitPos();

	UFUNCTION(BlueprintCallable)
	void IncreaseOwningCoreNum();

	UFUNCTION(BlueprintImplementableEvent)
	void SetbIsHighlighted(bool bIsWinner);

	UFUNCTION(BlueprintCallable)
	void Call_PlayGainAnim();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayGainAnim();

	UPROPERTY(BlueprintReadWrite)
	FVector2D rewordTargetRatio = FVector2D(0.5f, 0.6f);
};
