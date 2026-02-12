// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZPracticeModuleInfoBar.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZPracticeModuleInfoBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "OZ|GetCustomModuleStat")
	int GetDisplayedCustomModuleStat();

	UFUNCTION(BlueprintCallable)
	int GetDisplayedModuleStateAmount();
	
	UPROPERTY(BlueprintReadWrite)
	int32 ModuleID;
};
