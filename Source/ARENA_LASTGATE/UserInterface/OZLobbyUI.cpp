// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZLobbyUI.h"
#include "Widget/OZLobbyUserInfoWidget.h"
#include "Character/OZPlayerController.h"
#include "HUD/OZLobbyHUD.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Character/OZPlayerState.h"
#include "Components/TextBlock.h"

void UOZLobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

    UserInfoWidgets = {
        UserInfo1.Get(),
        UserInfo2.Get(),
        UserInfo3.Get(),
        UserInfo4.Get(),
        UserInfo5.Get(),
        UserInfo6.Get(),
        UserInfo7.Get(),
        UserInfo8.Get()
    };

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UOZLobbyUI::OnDelayedInitialize, 0.1f, false);

}

void UOZLobbyUI::SetUserInfo(TArray<FString> UserNames)
{
    for (int i = 0; i < UserNames.Num(); i++)
    {
        if (UserInfoWidgets[i])
        {
            UserInfoWidgets[i]->UserNameText->SetText(FText::FromString(UserNames[i]));
        }
    }
}

//void UOZLobbyUI::OnDelayedInitialize()
//{
//    OnUIConstructed.Broadcast();
//}
