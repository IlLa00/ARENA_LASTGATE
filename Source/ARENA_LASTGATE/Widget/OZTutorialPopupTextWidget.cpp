// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZTutorialPopupTextWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

void UOZTutorialPopupTextWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnPopupAnimationEnd.Clear();
}

void UOZTutorialPopupTextWidget::BroadCastOnTutorialTextEnd()
{
	OnPopupAnimationEnd.Broadcast();
}

void UOZTutorialPopupTextWidget::PopupTutorialText(float lifeTime)
{
	OnPlayTutorialText(lifeTime);
}

void UOZTutorialPopupTextWidget::CallPopupTutorialText(float lifeTime)
{
	OnPlayTutorialText(lifeTime);
}

//void UOZTutorialPopupTextWidget::CallSetTutorialText(FString titleText, FString Msg)
//{
//	SetText(titleText, Msg);
//}

void UOZTutorialPopupTextWidget::SetText(FString titleText, FString Msg, bool bIsMinimapOpened)
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (CanvasSlot)
	{
		int MsgLen = Msg.Len();

		int linePerOffset = Text_MSG->Font.Size;;

		if (MsgLen > 100)
			linePerOffset *= -1.5;

		FVector2D textOffset = FVector2D(0.f, linePerOffset * MsgLen / 10);

		if (bIsMinimapOpened)
		{
			FVector2D NewPos = originPos - textOffset;

			CanvasSlot->SetPosition(NewPos);
		}

		else
		{
			FVector2D NewPos = (originPos) - (originPos / 3)*2 - textOffset;

			CanvasSlot->SetPosition(NewPos);
		}
		
	}


	if(Text_Title != nullptr)
		Text_Title->SetText(FText::FromString(titleText));

	if(Text_MSG != nullptr)
		Text_MSG->SetText(FText::FromString(Msg));
	
}
