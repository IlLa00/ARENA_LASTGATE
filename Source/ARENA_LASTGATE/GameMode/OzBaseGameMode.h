// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OzBaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOzBaseGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	void AddPlayerReady(APlayerController* ClientController);
	
protected:
	TSet<APlayerController*> ReadyClientPlayers;

	virtual void StartGameMode() {};

private:
	void CheckAllPlayersReady();
};
