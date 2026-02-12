// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZPlayerStatusBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZPlayerStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetTargetPlayerState(TObjectPtr<class AOZPlayerState> targetPlayerState);
	void SetCurrentHP(float currHP);
	void SetHPIndicator(float newMaxHP);

	void UpdateSheildVolumnRatio(float maxSheildAmount, float maxHPAmount);
	void SetSheildRatio(float sheildRatio);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHPIndicator(int numIndicator);
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = "OZ|Status", meta = (BindWidget))
	TObjectPtr<class UImage> Image_HP;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|Status", meta = (BindWidget))
	TObjectPtr<class UBorder> Border_LossHP;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|Status", meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Sheild;

	
	
	
private:

	void UpdateStatus();

	void ResizeStatusBar();

	TObjectPtr<class AOZPlayerState> PlayerState = nullptr;

	float HPIndicatorInterval = 100.0f;

	float catched_maxSheild = 0.0f;
	float catched_LossHP = 0.0f;
	float catched_currentHP = 0.0f;
};
