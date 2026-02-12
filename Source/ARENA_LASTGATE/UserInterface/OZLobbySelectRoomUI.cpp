// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZLobbySelectRoomUI.h"
#include "Instance/OZGameInstance.h"
#include "Components/TextBlock.h"
#include "HUD/OZLobbySelectHUD.h"

void UOZLobbySelectRoomUI::DisplayUserAccountInfo()
{
	UOZGameInstance* OZGameInstacne = GetGameInstance<UOZGameInstance>();

	if (!OZGameInstacne)
		return;

	FString userName = OZGameInstacne->PlayerName;
	int32 winCount = OZGameInstacne->TotalWinCount;

	if (Text_UserName)
	{
		Text_UserName->SetText(FText::FromString(OZGameInstacne->PlayerName));
	}

	if (Text_UserScroe)
	{
		const int32 WinCount = OZGameInstacne->TotalWinCount;

		const FString WinCountString = FString::Printf(TEXT("%02d"), WinCount);

		Text_UserScroe->SetText(FText::FromString(WinCountString));
	}

}