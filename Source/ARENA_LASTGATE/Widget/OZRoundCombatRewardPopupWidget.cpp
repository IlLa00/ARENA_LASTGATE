// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZRoundCombatRewardPopupWidget.h"
#include "OZRoundCombatRewardPopupWidget.h"

void UOZRoundCombatRewardPopupWidget::NativeDestruct()
{
	Super::NativeDestruct();

	OnPopupAnimEnd.Clear();
}

void UOZRoundCombatRewardPopupWidget::CallPalyPopup()
{
	PlayAnim();
}
