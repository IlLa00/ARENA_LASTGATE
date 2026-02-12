// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "OZLobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbyGameState : public AGameState
{
	GENERATED_BODY()

protected:

public:

	AOZLobbyGameState();

	TArray<FString> GetUserNames();

};
