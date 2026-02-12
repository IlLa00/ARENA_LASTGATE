// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZGameResultUserInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Character/OZPlayerState.h"

void UOZGameResultUserInfoWidget::SetResult(bool bIsExcape, AOZPlayerState* playerState, int gameMaxAttack, int gameMaxAttacked)
{
	if (bIsExcape)
	{
		DisplayGameResult(true, playerState->UserName);
	}

	else
	{
		DisplayGameResult(false, playerState->UserName);
	}

	int totalDamage = (int)(playerState->Total_DamageAmount);
	int totalHit = (int)(playerState->Total_DamagedAmount);

	Text_TotalNumcores->SetText(FText::AsNumber(playerState->OwningCores));
	Text_TotalNumKills->SetText(FText::AsNumber(playerState->Total_KillCount));
	Text_TotalNumAtack->SetText(FText::AsNumber(totalDamage));
	Text_TotalNumAttacked->SetText(FText::AsNumber(totalHit));

	float AtkRatio = playerState->Total_DamageAmount / gameMaxAttack;
	float HitRatio = playerState->Total_DamagedAmount / gameMaxAttacked;

	ProgressBar_Attack->SetPercent(AtkRatio);
	ProgressBar_Attacked->SetPercent(HitRatio);

	//맥스 파라미터 효과
	if (AtkRatio == 1)
	{
		ProgressBar_Attack->SetFillColorAndOpacity(FLinearColor::Yellow);
		//HighlithAtkPercentagebar();
	}
		

	if (HitRatio)
	{
		ProgressBar_Attacked->SetFillColorAndOpacity(FLinearColor::Yellow);
		//HighlithHitPercentagebar();
	}
		
}

void UOZGameResultUserInfoWidget::SetDefaultState()
{
	ProgressBar_Attack->SetFillColorAndOpacity(FLinearColor::White);
	ProgressBar_Attacked->SetFillColorAndOpacity(FLinearColor::White);
}
