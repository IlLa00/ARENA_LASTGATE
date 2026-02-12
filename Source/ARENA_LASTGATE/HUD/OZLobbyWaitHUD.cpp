// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OZLobbyWaitHUD.h"
#include "UserInterface/OZLobbyWaitRoomUI.h"
#include "GameState/OZLobbyWaitGameState.h"
#include "Character/OZPlayerState.h"
#include "Widget/OZLobbyWaitRoomUserInfoBarWidget.h"

//void AOZLobbyWaitHUD::UpdateUI()
//{
//	if (CachedUserNames.IsEmpty())
//		return;
//
//	//Cast<UOZLobbyWaitRoomUI>(BindingUI)->SetUserInfo(CachedUserNames);
//}
//
//void AOZLobbyWaitHUD::SetLobbyWaitUserNames(TArray<FString> UserNames)
//{
//	if (Cast<UOZLobbyWaitRoomUI>(BindingUI) == nullptr)
//		return;
//
//	//Cast<UOZLobbyWaitRoomUI>(BindingUI)->SetUserInfo(UserNames);
//}

void AOZLobbyWaitHUD::BindGameState(TObjectPtr<class AOZLobbyWaitGameState> gameState)
{
	lobbywaitRoomUI = Cast<UOZLobbyWaitRoomUI>(BindingUI);
	OwnedGameState = gameState;
	
	gameState->OnLobbyWaitSlotChanged.AddUObject(this, &AOZLobbyWaitHUD::RefreshFromGameState);
	gameState->OnLobbyChattingUpdated.AddUObject(this, &AOZLobbyWaitHUD::RefreshLobbyChatting);

	AssingedLeastChattIndex = gameState->GetChattIndex();
}

void AOZLobbyWaitHUD::RefreshFromGameState()
{
	const TArray<FLobbySlot>& Slots = OwnedGameState->LobbySlots;
	const TArray<ELobbyUserState>& SlotState = OwnedGameState->SlotState;

	for (int32 i = 0; i < SlotState.Num(); ++i)
	{
		ELobbyUserState currSlotState = SlotState[i];

		switch (currSlotState)
		{
		case ELobbyUserState::None:
			lobbywaitRoomUI->UserNameWidgets[i]->SetUserInfoWait();
			break;
		case ELobbyUserState::Participate:

			if (Slots[i].PlayerState == nullptr)
				break;

			lobbywaitRoomUI->UserNameWidgets[i]->SetUserInfoHasPlayer(Slots[i].PlayerState->UserName);
			break;

		case ELobbyUserState::Ready:

			if (Slots[i].PlayerState == nullptr)
				break;

			lobbywaitRoomUI->UserNameWidgets[i]->SetUserInfoReady(Slots[i].PlayerState->UserName);
			break;
		default:
			break;
		}
	}

	//서버 전용 게임 시작 활성화 비활성화
	if (OZPlayer->HasAuthority())
	{
		int numReadyPlayers = 0;
		for (ELobbyUserState currSlotState : SlotState)
		{
			if (currSlotState == ELobbyUserState::Ready)
				numReadyPlayers++;
		}

		if (numReadyPlayers == OwnedGameState->GetMaxPlayerNum() -1)
		{
			lobbywaitRoomUI->ActiveOwnersGameStartBtn();
		}

		else
		{
			lobbywaitRoomUI->DeActiveOwnersGameStartBtn();
		}
	}
}

void AOZLobbyWaitHUD::RefreshLobbyChatting()
{
	if (lobbywaitRoomUI == nullptr)
		return;

	lobbywaitRoomUI->UpdateChatting(AssingedLeastChattIndex);
}

void AOZLobbyWaitHUD::OnKeyboardEnterPressed()
{
	lobbywaitRoomUI->OnPressedEnter();
}

void AOZLobbyWaitHUD::OnKeyboardESCPressed()
{
	lobbywaitRoomUI->OnPressedESC();
}
