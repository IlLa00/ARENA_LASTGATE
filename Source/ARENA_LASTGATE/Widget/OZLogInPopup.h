// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLogInPopup.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLogInPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowLogInPopup();
};
