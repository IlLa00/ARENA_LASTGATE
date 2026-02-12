// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/OZLobbyGameState.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerController.h"
#include "HUD/OZLobbyHUD.h"

AOZLobbyGameState::AOZLobbyGameState()
{
    //PrimaryActorTick.bCanEverTick = true;
}

TArray<FString> AOZLobbyGameState::GetUserNames()
{
    TArray<FString> UserNames;

    TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

    for (APlayerState* PS : PlayerStates)
    {
        if (AOZPlayerState* OzplayerState = Cast<AOZPlayerState>(PS))
        {
            UserNames.Add(OzplayerState->UserName);
        }
    }

    return UserNames;
}