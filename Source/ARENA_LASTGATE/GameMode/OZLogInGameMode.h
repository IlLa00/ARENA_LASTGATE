// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OzBaseGameMode.h"
#include "OZLogInGameMode.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARENA_LASTGATE_API AOZLogInGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

public:

	AOZLogInGameMode();

	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "OZ|HUD Class")
	TSubclassOf<AHUD> LogInHUDClass;



};
