// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZRoundPrepareUI.h"
#include "Components/TextBlock.h"

void UOZRoundPrepareUI::NativeConstruct()
{
	Super::NativeConstruct();

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UOZRoundPrepareUI::OnDelayedInitialize, 0.1f, false);
}

void UOZRoundPrepareUI::SetPrepareRemainTime(uint8 remainingTime)
{
	//Text_PrepareTimeRemian->SetText(FText::AsNumber(remainingTime));
}
