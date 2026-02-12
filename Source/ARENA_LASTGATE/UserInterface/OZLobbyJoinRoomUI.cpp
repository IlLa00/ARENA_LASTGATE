// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZLobbyJoinRoomUI.h"
#include "HUD/OZLobbySelectHUD.h"

void UOZLobbyJoinRoomUI::ShowJoinRoom()
{
	UpdateSessionList();
}

void UOZLobbyJoinRoomUI::LeaveJoinRoom()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(FindSessionTimerHandle);
    }
}
