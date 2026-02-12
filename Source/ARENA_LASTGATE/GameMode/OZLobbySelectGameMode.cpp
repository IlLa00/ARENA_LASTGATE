// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OZLobbySelectGameMode.h"
#include "GameFramework/HUD.h"
#include "Character/OZPlayerController.h"

AOZLobbySelectGameMode::AOZLobbySelectGameMode()
{
	PlayerControllerClass = AOZPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<AHUD> LogInHUD(TEXT("/Game/UI/HUD/Fixed/BP_LobbySelectHUD"));

	if (LobbySelectHUDClass == NULL && LogInHUD.Class != NULL)
	{
		HUDClass = LogInHUD.Class;

		return;
	}

	else if (LobbySelectHUDClass != NULL)
	{
		HUDClass = LobbySelectHUDClass;

		return;
	}

	UE_LOG(LogTemp, Error, TEXT("InGameMode의 Defaul HUD와 Custum HUD 둘다 NULL 입니다."));
	return;
}

void AOZLobbySelectGameMode::BeginPlay()
{
}
