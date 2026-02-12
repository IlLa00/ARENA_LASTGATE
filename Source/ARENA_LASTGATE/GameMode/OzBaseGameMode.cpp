// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OzBaseGameMode.h"
#include "GameFramework/GameStateBase.h"

void AOzBaseGameMode::AddPlayerReady(APlayerController* ClientController)
{
	ReadyClientPlayers.Add(ClientController);

	CheckAllPlayersReady();
}

void AOzBaseGameMode::CheckAllPlayersReady()
{
	if (!GameState) return;

	TArray<APlayerController*> ClientControllers;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && !PC->IsLocalController())
		{
			ClientControllers.Add(PC);
		}
	}

	if (ReadyClientPlayers.Num() != ClientControllers.Num())
		return;

	for (APlayerController* PC : ClientControllers)
	{
		if (!ReadyClientPlayers.Contains(PC))
			return;
	}

	StartGameMode();
}
