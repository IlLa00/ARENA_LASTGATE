// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZCombatResultPopupWidget.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UOZCombatResultPopupWidget::ShowCombatResult(bool bIsWin)
{
	CallShowCombatResult(bIsWin);
}

void UOZCombatResultPopupWidget::BroadCastCombatResult(bool bIsWin)
{
	CombatResultDelegate.Broadcast(bIsWin);
}

void UOZCombatResultPopupWidget::PlayResultSound(bool bIsWin)
{
	USoundBase* TargetSfx = bIsWin ? WinSfx : LoseSfx;
	if (!TargetSfx) return;

	UGameplayStatics::PlaySound2D(this, TargetSfx);
}
