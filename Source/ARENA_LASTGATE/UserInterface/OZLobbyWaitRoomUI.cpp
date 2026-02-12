// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZLobbyWaitRoomUI.h"
#include "Widget/OZLobbyWaitRoomUserInfoBarWidget.h"
#include "GameState/OZLobbyWaitGameState.h"
#include "GameMode/OZLobbyWaitGameMode.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Subsystem/OZOnlineSessionSubsystem.h"
#include "Character/OZPlayerController.h"

void UOZLobbyWaitRoomUI::NativeConstruct()
{
	Super::NativeConstruct();

	UserNameWidgets.Empty();
	UserNameWidgets.Reserve(4);

	UserNameWidgets.Add(UserNameWidget_1);
	UserNameWidgets.Add(UserNameWidget_2);
	UserNameWidgets.Add(UserNameWidget_3);
	UserNameWidgets.Add(UserNameWidget_4);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UOZLobbyWaitRoomUI::OnDelayedInitialize, 0.1f, false);
}

void UOZLobbyWaitRoomUI::OnDelayedInitialize()
{
	OnUIConstructed.Broadcast();
}

void UOZLobbyWaitRoomUI::OnExitLobbyWait()
{
	if (OwnerPlayerController->GetGameInstance() == nullptr)
		return;

	OwnerPlayerController->GetGameInstance()->GetSubsystem<UOZOnlineSessionSubsystem>()->ExitSession(OwnerPlayerController);
}

void UOZLobbyWaitRoomUI::ToggleReadyState()
{
	if (OwnerPlayerController == nullptr)
		return;

	OwnerPlayerController->ChangeLobbyWaitState(this);
}

bool UOZLobbyWaitRoomUI::CheckCanMoveToInGame()
{
	if (OwnerPlayerController->HasAuthority() == false)
		return false;

	AOZLobbyWaitGameState* lobbyGameState = Cast<AOZLobbyWaitGameState>(GetWorld()->GetGameState());
	if (lobbyGameState == nullptr)
		return false;

	for (FLobbySlot& currSlot : lobbyGameState->LobbySlots)
	{
		if (currSlot.PlayerState == nullptr)
			return false;
	}

	int numReadyPlayers = 0;
	for (ELobbyUserState currSlotState : lobbyGameState->SlotState)
	{
		if (currSlotState == ELobbyUserState::Ready)
			numReadyPlayers++;
	}

	return numReadyPlayers == lobbyGameState->GetMaxPlayerNum() -1;
}

FString UOZLobbyWaitRoomUI::GetInGameLevelPath()
{
	if (OwnerPlayerController->HasAuthority() == false)
		return {};

	AOZLobbyWaitGameMode* OZLobbyWaitGameMode = Cast<AOZLobbyWaitGameMode>(OwnerPlayerController->GetWorld()->GetAuthGameMode());

	if(OZLobbyWaitGameMode == nullptr)
		return {};

	
	FString levelPath = OZLobbyWaitGameMode->TravleInGameLevel.GetLongPackageName();

	return levelPath + TEXT("?listen");
}

int UOZLobbyWaitRoomUI::ChangeUserInfoReadyState(int index)
{
	if (UserNameWidgets[index] == nullptr)
		return 0;

	bool bIsReadyState = UserNameWidgets[index]->ToggleReadyState();

	if (bIsReadyState)
		return 1;

	else
		return -1;
}

int UOZLobbyWaitRoomUI::GetPlayerReadyState()
{
	return OwnerPlayerController->GetIsPlayerReadyState();
}
