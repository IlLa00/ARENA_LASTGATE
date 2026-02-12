// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/OZHud.h"
#include "OZPracticeHUD.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZPracticeHUD : public AOZHud
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ToggleCustomConverterUI();

	UFUNCTION(BlueprintCallable)
	void CallToggleCustomConverterUI();
	
};
