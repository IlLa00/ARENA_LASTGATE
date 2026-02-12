// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZTraningConverterWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UOZTraningConverterWidget::CallBoundActive(bool bActive)
{
	SetBoundActive(bActive);
}

void UOZTraningConverterWidget::BroadCastOnConverterSelected()
{
	OnConverterSelectedDeligate.Broadcast(this);
}
