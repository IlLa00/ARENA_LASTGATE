// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/OZConvertData.h"
#include "OZTrainingConverterListWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZTrainingConverterListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	EConvertGrade slotTier = EConvertGrade::None;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UScrollBox> slotScrollBox = nullptr;
	
};
