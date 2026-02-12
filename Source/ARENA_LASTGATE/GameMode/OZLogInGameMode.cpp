// Fill out your copyright notice in the Description page of Project Settings.


#include "OZLogInGameMode.h"
#include "GameFramework/HUD.h"
#include "Character/OZPlayerController.h"

AOZLogInGameMode::AOZLogInGameMode()
{
	PlayerControllerClass = AOZPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<AHUD> LogInHUD(TEXT("/Game/UI/HUD/BP_LogInHUD"));

	if (LogInHUDClass == NULL && LogInHUD.Class != NULL)
	{
		HUDClass = LogInHUD.Class;

		return;
	}

	else if(LogInHUDClass != NULL)
	{
		HUDClass = LogInHUDClass;

		return;
	}

	UE_LOG(LogTemp, Error, TEXT("InGameMode의 Defaul HUD와 Custum HUD 둘다 NULL 입니다."));
	return;

}

void AOZLogInGameMode::BeginPlay()
{
	
}
