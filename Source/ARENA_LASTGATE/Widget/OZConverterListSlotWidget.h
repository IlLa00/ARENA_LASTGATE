// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/OZConvertData.h"
#include "OZConverterListSlotWidget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListSlotSelected, UOZConverterListSlotWidget*, clickedWidget, bool, bIsLeftClick);

UCLASS()
class ARENA_LASTGATE_API UOZConverterListSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnListSlotSelected OnConverterSelectedDeligate;

	UFUNCTION(BlueprintCallable)
	void CallBoundActive(bool bActive);

	UFUNCTION(BlueprintCallable)
	void BroadCastOnConverterSelected(bool clickedMousePos);

	void InitConverWidget(FOZConvertData data);

	int equipCount = 0;

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> SelectedSlotBoundImage;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> ConverterIconImage;

	UPROPERTY(BlueprintReadWrite)
	FOZConvertData converterData;
	
};
