// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZPlayerStatusBarWidget.h"
#include "AbilitySystemComponent.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Image.h"

void UOZPlayerStatusBarWidget::SetTargetPlayerState(TObjectPtr<class AOZPlayerState> targetPlayerState)
{
	if (targetPlayerState == nullptr)
		return;

	PlayerState = targetPlayerState;

	if (PlayerState != nullptr)
	{
		catched_currentHP = PlayerState->GetHeatlInfo().MaxAmount;
		catched_maxSheild = PlayerState->GetSheildInfo().MaxAmount;
		catched_LossHP = 0;

		ResizeStatusBar();
	}
}

void UOZPlayerStatusBarWidget::SetCurrentHP(float currHP)
{

	catched_currentHP = currHP;
	catched_LossHP = PlayerState->GetHeatlInfo().MaxAmount - currHP;


	ResizeStatusBar();
}

void UOZPlayerStatusBarWidget::SetHPIndicator(float newMaxHP)
{
	int interverNum = newMaxHP / HPIndicatorInterval;

	UpdateHPIndicator(interverNum);
}

void UOZPlayerStatusBarWidget::UpdateSheildVolumnRatio(float maxSheildAmount, float maxHPAmount)
{
	if (!Image_HP || !ProgressBar_Sheild)
		return;

	const float Total = maxHPAmount + maxSheildAmount;
	if (Total <= 0.f)
		return;

	catched_currentHP = maxHPAmount;
	catched_maxSheild = maxSheildAmount;

	ResizeStatusBar();
}


void UOZPlayerStatusBarWidget::SetSheildRatio(float sheildRatio)
{
	ProgressBar_Sheild->SetPercent(sheildRatio);

	if (sheildRatio <= 0.0f)
	{
		ProgressBar_Sheild->SetPercent(0.0f);
	}
}


void UOZPlayerStatusBarWidget::UpdateStatus()
{
	
}

void UOZPlayerStatusBarWidget::ResizeStatusBar()
{
	UHorizontalBoxSlot* HPImageSlot = Cast<UHorizontalBoxSlot>(Image_HP->Slot);
	if (HPImageSlot)
	{
		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Fill;
		Size.Value = catched_currentHP;
		HPImageSlot->SetSize(Size);
	}

	UHorizontalBoxSlot* SheildProgressSlot = Cast<UHorizontalBoxSlot>(ProgressBar_Sheild->Slot);
	if (SheildProgressSlot)
	{
		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Fill;
		Size.Value = catched_maxSheild;
		SheildProgressSlot->SetSize(Size);
	}

	UHorizontalBoxSlot* LossBorderSlot = Cast<UHorizontalBoxSlot>(Border_LossHP->Slot);
	if (LossBorderSlot)
	{
		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Fill;
		Size.Value = catched_LossHP;
		LossBorderSlot->SetSize(Size);
	}
}
