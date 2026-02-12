// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZFlyingRewordCoreWidget.h"

void UOZFlyingRewordCoreWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnFlyingAnimEnd.Clear();
}

void UOZFlyingRewordCoreWidget::OnActivated()
{
	OnStarted();
}

void UOZFlyingRewordCoreWidget::BroadCast_WidgetOnTarget()
{
	if (assingedIndex == -1)
		return;

	OnFlyingAnimEnd.Broadcast(assingedIndex);
}
