// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/OzBaseGameMode.h"
#include "OZLobbySelectGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbySelectGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

public:

	AOZLobbySelectGameMode();

	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "OZ|HUD Class")
	TSubclassOf<AHUD> LobbySelectHUDClass;
	
};
