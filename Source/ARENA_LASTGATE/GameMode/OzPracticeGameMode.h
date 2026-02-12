// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/OzBaseGameMode.h"
#include "Data/OZConvertData.h"
#include "OZPracticeGameMode.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZPracticeGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

public:
	AOZPracticeGameMode();

	// 이 함수를 반드시 사용하시면 됩니다
	/*UFUNCTION(BlueprintCallable, Category = "Effects")
	TArray<FEffectData> GetAllEffects();

	UPROPERTY(BlueprintReadOnly)
	TArray<FEffectData> AllEffectArray;*/
};
	