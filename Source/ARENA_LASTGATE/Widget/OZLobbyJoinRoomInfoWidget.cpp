// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZLobbyJoinRoomInfoWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UOZLobbyJoinRoomInfoWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnSessionClicked.Clear();
}

void UOZLobbyJoinRoomInfoWidget::HightLightInfoBar()
{
	if (Image_Highlight == nullptr)
		return;

	Image_RoomName->SetBrushFromTexture(Image_Highlight);
	Border_RoomPlayers->SetBrushFromTexture(Image_Highlight);
	Border_RoomState->SetBrushFromTexture(Image_Highlight);
}

void UOZLobbyJoinRoomInfoWidget::NormalizeInfoBar()
{
	if (Image_Normal == nullptr)
		return;

	Image_RoomName->SetBrushFromTexture(Image_Normal);
	Border_RoomPlayers->SetBrushFromTexture(Image_Normal);
	Border_RoomState->SetBrushFromTexture(Image_Normal);
}

void UOZLobbyJoinRoomInfoWidget::SetRoomInfo(FString roomState, int currPlayers)
{
	Text_RoomName->SetText(FText::FromString(ExtraSettinInfo.RoomName));
	Text_RoomPlayers->SetText(FText::Format(FText::FromString("{0} / {1}") ,currPlayers ,4));
	Text_RoomState->SetText(FText::FromString(roomState));

	if (ExtraSettinInfo.bIsPrivateRoom)
	{
		Image_SecurityImage->SetBrushFromTexture(Icon_Lock);
	}

	else
	{
		Image_SecurityImage->SetBrushFromTexture(Icon_UnLock);
	}

	bIsInitializedSucceed = true;
}

void UOZLobbyJoinRoomInfoWidget::BroadCastSessionClicekd()
{
	if (assingedIndex == -1)
		return;

	OnSessionClicked.Broadcast(ExtraSettinInfo, assingedIndex);
}
