// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/OZConvertData.h"
#include "OZTraningConverterWidget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConverterSelected, UOZTraningConverterWidget*, clickedWidget);

UCLASS()
class ARENA_LASTGATE_API UOZTraningConverterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnConverterSelected OnConverterSelectedDeligate;

	UFUNCTION(BlueprintImplementableEvent)
	void SetBoundActive(bool bActive);

	UFUNCTION(BlueprintCallable)
	void CallBoundActive(bool bActive);

	UFUNCTION(BlueprintCallable)
	void BroadCastOnConverterSelected();
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_ConverterName;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<class UImage> Image_ConverterIcon;

	UPROPERTY(BlueprintReadWrite)
	int32 ID;

	UPROPERTY(BlueprintReadWrite)
	EConvertGrade Grade;
};
