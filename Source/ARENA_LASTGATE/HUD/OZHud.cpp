// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OZHud.h"
#include "UserInterface/OZUI.h"
#include "Character/OZPlayerController.h"

void AOZHud::ListenUIConstructed()
{
	if (BindingUI == nullptr)
		return;

	BindingUI->OnUIConstructed.AddDynamic(this, &AOZHud::PostUIInit);
}

void AOZHud::PostUIInit()
{
	OZPlayer->OnUINativeConstructClear();
}

void AOZHud::SetInputModeUIOnly(TObjectPtr<AOZPlayerController> controller, TObjectPtr<UUserWidget> focusWidget, bool bCursorActive)
{
    if (controller == nullptr)
        return;

    FInputModeUIOnly InputMode;
    if (focusWidget)
    {
        InputMode.SetWidgetToFocus(focusWidget->TakeWidget());
    }

    //InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);

    //controller->SetInputMode(InputMode);
    controller->SetOzInputMode(EInputMode::UIOnly);
    controller->bShowMouseCursor = bCursorActive;
}

void AOZHud::SetInputModeUIANDGame(TObjectPtr<AOZPlayerController> controller, bool bCursorActive)
{
    if (controller == nullptr)
        return;

    FInputModeGameAndUI InputMode;

    //InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    //controller->SetInputMode(InputMode);
    controller->SetOzInputMode(EInputMode::GameAndUI);
    controller->bShowMouseCursor = bCursorActive;
}

void AOZHud::SetInputModeGameOnly(TObjectPtr<class AOZPlayerController> controller, bool bCursorActive)
{
    if (controller == nullptr)
        return;

    FInputModeGameOnly InputMode;

    //controller->SetInputMode(InputMode);
    controller->SetOzInputMode(EInputMode::GameOnly);
    controller->bShowMouseCursor = bCursorActive;
}
