// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZConverterListSlotWidget.h"
#include "Components/Image.h"

void UOZConverterListSlotWidget::CallBoundActive(bool bActive)
{
	if (SelectedSlotBoundImage == nullptr)
		return;

	ESlateVisibility targetVisibility = ESlateVisibility::SelfHitTestInvisible;

	if (bActive == false)
	{
		targetVisibility = ESlateVisibility::Hidden;
	}

	SelectedSlotBoundImage->SetVisibility(targetVisibility);
}

void UOZConverterListSlotWidget::BroadCastOnConverterSelected(bool clickedMousePos)
{
	OnConverterSelectedDeligate.Broadcast(this, clickedMousePos);
}

void UOZConverterListSlotWidget::InitConverWidget(FOZConvertData data)
{
	if (data.Grade == EConvertGrade::None || data.ID == 0)
		return;

	converterData = data;

	ConverterIconImage->SetBrushFromTexture(converterData.Icon.Get());

	CallBoundActive(false);
}
