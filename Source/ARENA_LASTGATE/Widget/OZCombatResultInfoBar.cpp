// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZCombatResultInfoBar.h"
#include "Components/ScaleBox.h"

void UOZCombatResultInfoBar::CallToggleDisplayedParams(bool bIsShowPercentage)
{
	ToggleDisplayedParams(bIsShowPercentage);
}

void UOZCombatResultInfoBar::IncreaseOwningCoreNum()
{

	FText Text_Cores = Text_Core->GetText();

	FString Str = Text_Cores.ToString();

	if (!Str.IsNumeric())
		return;

	int32 num_cores = FCString::Atoi(*Str);

	num_cores++;

	Text_Core->SetText(FText::AsNumber(num_cores));
}

void UOZCombatResultInfoBar::Call_PlayGainAnim()
{
	PlayGainAnim();
}
