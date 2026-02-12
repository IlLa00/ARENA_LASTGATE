// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OZLobbyHUD.h"
#include "Character/OZPlayerController.h"
#include "UserInterface/OZLobbyUI.h"

//void AOZLobbyHUD::ListenUIConstructed()
//{
//	if (LobbyUI == nullptr)
//		return;
//
//	LobbyUI->OnLobbyUIConstructed.AddDynamic(this, &AOZLobbyHUD::PostUIInit);
//}

//void AOZLobbyHUD::PostUIInit()
//{
//	OZPlayer->OnUINativeConstructClear();
//}

void AOZLobbyHUD::UpdateUI()
{
	if (CachedUserNames.IsEmpty())
		return;

	

	Cast<UOZLobbyUI>(BindingUI)->SetUserInfo(CachedUserNames);
}

void AOZLobbyHUD::SetLobbyUserNames(TArray<FString> UserNames)
{
	if (Cast<UOZLobbyUI>(BindingUI) == nullptr)
		return;

	Cast<UOZLobbyUI>(BindingUI)->SetUserInfo(UserNames);
}
