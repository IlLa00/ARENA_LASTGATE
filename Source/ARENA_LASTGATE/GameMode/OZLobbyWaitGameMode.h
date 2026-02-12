// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/OzBaseGameMode.h"
#include "OZLobbyWaitGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbyWaitGameMode : public AOzBaseGameMode
{
	GENERATED_BODY()

public:
	AOZLobbyWaitGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "OZ|TargetInGameLevel")
	TSoftObjectPtr<UWorld> TravleInGameLevel;
	
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool CheckPlayerReady = false;*/

};
