// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OZLobbySelectHUD.h"
#include "Blueprint/UserWidget.h"
#include "UserInterface/OZLobbyJoinRoomUI.h"
#include "Widget/OZLogInPopup.h"
#include "OZLobbySelectHUD.h"

void AOZLobbySelectHUD::UpdateUI()
{

}

void AOZLobbySelectHUD::SetLobbySelectUserNames(TArray<FString> UserNames)
{
}

void AOZLobbySelectHUD::SetLobbySelectUI(ELobbyUIType UiType, bool active)
{
	int32 index = (int32)UiType;
	if (!LobbySelectUIArray.IsValidIndex(index) || LobbySelectUIArray[index] == nullptr)
	{
		return;
	}

	if (active == true)
	{
		switch (UiType)
		{
		case ELobbyUIType::LOBBYUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Visible);
			break;
		case ELobbyUIType::LOGINPOPUPUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Visible);
			Cast<UOZLogInPopup>(LobbySelectUIArray[index])->OnShowLogInPopup();
			break;
		case ELobbyUIType::LOBBYSELECTUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Visible);
			break;
		case ELobbyUIType::LOBBYMAKEROOMUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Visible);
			break;
		case ELobbyUIType::LOBBYJOINROOMUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Visible);
			Cast<UOZLobbyJoinRoomUI>(LobbySelectUIArray[index])->ShowJoinRoom();
			break;
		/*case ELobbyUIType::LOBBYWAITROOMUI:
			LobbySelectUIArray[5]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			break;*/
		}

		//SetInputModeUIOnly(OZPlayer, LobbySelectUIArray[index], true);
	}


	else if (active == false)
	{
		switch (UiType)
		{
		case ELobbyUIType::LOBBYUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case ELobbyUIType::LOGINPOPUPUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case ELobbyUIType::LOBBYSELECTUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case ELobbyUIType::LOBBYMAKEROOMUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Collapsed);
			break;
		case ELobbyUIType::LOBBYJOINROOMUI:
			LobbySelectUIArray[index]->SetVisibility(ESlateVisibility::Collapsed);
			break;
		/*case ELobbyUIType::LOBBYWAITROOMUI:
			LobbySelectUIArray[5]->SetVisibility(ESlateVisibility::Collapsed);
			break;*/
		}
	}
}

void AOZLobbySelectHUD::OnMakeRoomButtonClicked()
{
	SetLobbySelectUI(ELobbyUIType::LOBBYSELECTUI, false);

	SetLobbySelectUI(ELobbyUIType::LOBBYMAKEROOMUI, true);

	OnLobbyUIShow(ELobbyUIType::LOBBYMAKEROOMUI);
}

void AOZLobbySelectHUD::OnMakeRoomCloseButtonClicked()
{
	SetLobbySelectUI(ELobbyUIType::LOBBYMAKEROOMUI, false);

	SetLobbySelectUI(ELobbyUIType::LOBBYSELECTUI, true);

	OnLobbyUIShow(ELobbyUIType::LOBBYSELECTUI);
}

void AOZLobbySelectHUD::OnJoinRoomButtonClicked()
{
	SetLobbySelectUI(ELobbyUIType::LOBBYSELECTUI, false);

	SetLobbySelectUI(ELobbyUIType::LOBBYJOINROOMUI, true);

	OnLobbyUIShow(ELobbyUIType::LOBBYJOINROOMUI);
}

void AOZLobbySelectHUD::OnJoinRoomCloseButtonClicked()
{
	SetLobbySelectUI(ELobbyUIType::LOBBYJOINROOMUI, false);

	SetLobbySelectUI(ELobbyUIType::LOBBYSELECTUI, true);

	OnLobbyUIShow(ELobbyUIType::LOBBYSELECTUI);
}

void AOZLobbySelectHUD::OnJoinRoomCreateButtonClicked()
{
	Cast<UOZLobbyJoinRoomUI>(LobbySelectUIArray[(int)ELobbyUIType::LOBBYJOINROOMUI])->LeaveJoinRoom();
}
