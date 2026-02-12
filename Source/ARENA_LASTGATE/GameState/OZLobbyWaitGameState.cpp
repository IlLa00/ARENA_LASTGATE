// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/OZLobbyWaitGameState.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerController.h"
#include "Subsystem/OZLoadingSubsystem.h"
#include "Net/UnrealNetwork.h"

void AOZLobbyWaitGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	OnLobbyWaitSlotChanged.Clear();
}

AOZLobbyWaitGameState::AOZLobbyWaitGameState()
{
	LobbySlots.SetNum(maxPlayerNum);
	SlotState.SetNum(maxPlayerNum);

	for (int32 i = 0; i < maxPlayerNum; i++)
	{
		LobbySlots[i].SlotIndex = i;
	}

	for (int32 i = 0; i < maxPlayerNum; i++)
	{
		SlotState[i] = ELobbyUserState::None;
	}

	chattIndex = 0;
	ChatMessages.Empty();
}


void AOZLobbyWaitGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZLobbyWaitGameState, LobbySlots);
	DOREPLIFETIME(AOZLobbyWaitGameState, SlotState);

	DOREPLIFETIME(AOZLobbyWaitGameState, chattIndex);
	DOREPLIFETIME(AOZLobbyWaitGameState, ChatMessages);
}

void AOZLobbyWaitGameState::RegisterPlayer(AOZPlayerState* NewPlayerState)
{
	if (!HasAuthority() || !NewPlayerState)
		return;

	/*for (const FLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == NewPlayerState)
			return;
	}*/

	int index = 0;
	for (FLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == nullptr)
		{
			Slot.PlayerState = NewPlayerState;
			SlotState[index] = ELobbyUserState::Participate;
			Cast<AOZPlayerController>(NewPlayerState->GetPlayerController())->assingedWaitRoomIndex = index;
			return;
		}
		index++;
	}


}

void AOZLobbyWaitGameState::UnregisterPlayer(AOZPlayerState* LeavingPlayerState)
{
	if (!HasAuthority() || !LeavingPlayerState)
		return;

	int index = 0;
	for (FLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == LeavingPlayerState)
		{
			Slot.PlayerState = nullptr;
			SlotState[index] = ELobbyUserState::None;
			break;
		}
		index++;
	}

	OnLobbyWaitSlotChanged.Broadcast();
}

int AOZLobbyWaitGameState::GetIsPlayerReady(AOZPlayerState* TargetPlayerState)
{
	int playerIndex = GetAssingedIndex(TargetPlayerState);

	if (playerIndex == -1)
		return 0;

	ELobbyUserState targetplayerSlotState = SlotState[playerIndex];

	switch (targetplayerSlotState)
	{
	case ELobbyUserState::None:
		return 0;

	case ELobbyUserState::Participate:
		return -1;

	case ELobbyUserState::Ready:
		return 1;
	default:
		break;
	}

	return false;
}

int AOZLobbyWaitGameState::GetAssingedIndex(AOZPlayerState* RequestPlayerState)
{
	int index = 0;
	for (FLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == RequestPlayerState)
		{
			return index;
		}
		index++;
	}
	return -1;
}

void AOZLobbyWaitGameState::ChangePlayerReadyState(int playerIndex, bool bIsReadyState)
{
	if (bIsReadyState)
	{
		SlotState[playerIndex] = ELobbyUserState::Ready;
	}

	else
	{
		SlotState[playerIndex] = ELobbyUserState::Participate;
	}

	OnLobbyWaitSlotChanged.Broadcast();
}

void AOZLobbyWaitGameState::OnRep_LobbySlots()
{
	OnLobbyWaitSlotChanged.Broadcast();
}

void AOZLobbyWaitGameState::OnRep_SlotStateChanged()
{
	OnLobbyWaitSlotChanged.Broadcast();
}

void AOZLobbyWaitGameState::OnRep_ChatMessages()
{
	//Update chatting
	OnLobbyChattingUpdated.Broadcast();
}

void AOZLobbyWaitGameState::AddChatMessage(FChatMessage msg)
{
	if (HasAuthority() == false)
		return;

	msg.ChatIndex = chattIndex;

	ChatMessages.Add(msg);

	chattIndex++;

	OnLobbyChattingUpdated.Broadcast();
}


void AOZLobbyWaitGameState::ShowLoadingScreen_Implementation()
{
	UOZLoadingSubsystem* loadingSubsystem = GetGameInstance()->GetSubsystem<UOZLoadingSubsystem>();

	if (loadingSubsystem == nullptr)
		return;

	loadingSubsystem->PlayLoadingScene();
}

