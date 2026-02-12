// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZLobbyWaitRoomUserInfoBarWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOZLobbyWaitRoomUserInfoBarWidget::SetUserInfoWait()
{
	if (Image_UserInfoBarBackGround == nullptr || Text_PlayerName == nullptr)
		return;

	Image_UserInfoBarBackGround->SetBrushFromTexture(Image_Wait);
	Text_PlayerName->SetText(FText::FromString(TEXT("")));
}

void UOZLobbyWaitRoomUserInfoBarWidget::SetUserInfoHasPlayer(FString userName)
{
	if (Image_UserInfoBarBackGround == nullptr || Text_PlayerName == nullptr)
		return;

	Image_UserInfoBarBackGround->SetBrushFromTexture(Image_HasPlayer);
	Text_PlayerName->SetText(FText::FromString(userName));
}

void UOZLobbyWaitRoomUserInfoBarWidget::SetUserInfoReady(FString userName)
{
	if (Image_UserInfoBarBackGround == nullptr || Text_PlayerName == nullptr)
		return;

	Image_UserInfoBarBackGround->SetBrushFromTexture(Image_Ready);
	Text_PlayerName->SetText(FText::FromString(userName));
}

bool UOZLobbyWaitRoomUserInfoBarWidget::ToggleReadyState()
{
	bIsReadyState = !bIsReadyState;

	if (bIsReadyState)
	{
		Image_UserInfoBarBackGround->SetBrushFromTexture(Image_Ready);
	}

	else
	{
		Image_UserInfoBarBackGround->SetBrushFromTexture(Image_HasPlayer);
	}

	return bIsReadyState;
}
