// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OzBaseGameMode.h"
#include "OZLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbyGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

public:

	AOZLobbyGameMode();

protected:

	/*UPROPERTY(EditDefaultsOnly, Category = "OZ|HUD Class")
	TSubclassOf<AHUD> LobbyHUDClass;*/

	UPROPERTY()
	TObjectPtr<class AOZLobbyHUD> LobbyHud;

	
};
