// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OZLobbyWaitGameMode.h"
#include "HUD/OZLobbyWaitHUD.h"
#include "Character/OZPlayerController.h"
#include "Character/OZPlayerState.h"
#include "GameState/OZLobbyWaitGameState.h"

AOZLobbyWaitGameMode::AOZLobbyWaitGameMode()
{
	PlayerControllerClass = AOZPlayerController::StaticClass();

	GameStateClass = AOZLobbyWaitGameState::StaticClass();

	PlayerStateClass = AOZPlayerState::StaticClass();

	static ConstructorHelpers::FClassFinder<AOZLobbyWaitHUD> LobbyWaitHUD(TEXT("/Game/UI/HUD/Fixed/BP_LobbyWaitHUD"));

	if (LobbyWaitHUD.Class != NULL)
	{
		HUDClass = LobbyWaitHUD.Class;
	}

	else
	{
		UE_LOG(LogTemp, Error, TEXT("InGameMode의 Defaul HUD와 Custum HUD 둘다 NULL 입니다."));
		return;
	}
}
