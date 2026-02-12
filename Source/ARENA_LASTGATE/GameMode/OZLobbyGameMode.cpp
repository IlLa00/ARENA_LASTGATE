// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OZLobbyGameMode.h"
#include "GameFramework/HUD.h"
#include "HUD/OZLobbyHUD.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Character/OZPlayerController.h"
#include "Character/OZPlayerState.h"
#include "GameState/OZLobbyGameState.h"

AOZLobbyGameMode::AOZLobbyGameMode()
{
	PlayerControllerClass = AOZPlayerController::StaticClass();

	GameStateClass = AOZLobbyGameState::StaticClass();

	PlayerStateClass = AOZPlayerState::StaticClass();

	static ConstructorHelpers::FClassFinder<AOZLobbyHUD> LogInHUD(TEXT("/Game/UI/HUD/BP_LobbyHUD"));

	if (LogInHUD.Class != NULL)
	{
		HUDClass = LogInHUD.Class;
	}

	else
	{
		UE_LOG(LogTemp, Error, TEXT("InGameMode의 Defaul HUD와 Custum HUD 둘다 NULL 입니다."));
		return;
	}

	

}
