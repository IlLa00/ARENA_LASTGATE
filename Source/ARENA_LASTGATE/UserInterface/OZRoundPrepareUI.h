// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZRoundPrepareUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZRoundPrepareUI : public UOZUI
{
	GENERATED_BODY()

protected:

	void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|RoundNumText Active Time")
	float RoundNumTextActiveTime = 5.0f;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetRoundNum(uint8 roundNum);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowRoundNum();

	void SetPrepareRemainTime(uint8 remainingTime);

	/*UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_PrepareTimeRemian;*/
};
