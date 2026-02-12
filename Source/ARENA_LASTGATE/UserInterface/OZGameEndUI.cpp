// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZGameEndUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Character/OZPlayerState.h"
#include "Widget/OZGameResultUserInfoWidget.h"

void UOZGameEndUI::SetGameResult(TArray<TObjectPtr<class AOZPlayerState>>& ozPlayerStates)
{
	for (int i = 0; i < GameResultWidgetArray.Num(); i++)
	{
		GameResultWidgetArray[i]->SetDefaultState();
		GameResultWidgetArray[i]->SetVisibility(ESlateVisibility::Hidden);
	}
		

	TArray<TObjectPtr<AOZPlayerState>> SortedStates = ozPlayerStates;

	SortedStates.Sort([](const TObjectPtr<AOZPlayerState>& A,
		const TObjectPtr<AOZPlayerState>& B)
		{
			return A->OwningCores > B->OwningCores;
		});

	int maxNumCores = -1;

	int maxNumAtk = 0;
	int maxNumHit = 0;

	//Cark Max 
	for (int i = 0; i < ozPlayerStates.Num(); i++)
	{
		AOZPlayerState* currOzState = ozPlayerStates[i];

		if (maxNumCores < currOzState->OwningCores)
			maxNumCores = currOzState->OwningCores;

		if (maxNumAtk < currOzState->Total_DamageAmount)
			maxNumAtk = currOzState->Total_DamageAmount;

		if (maxNumHit < currOzState->Total_DamagedAmount)
			maxNumHit = currOzState->Total_DamagedAmount;
	}


	for (int i = 0; i < SortedStates.Num(); i++)
	{
		int numOwningCore = SortedStates[i]->OwningCores;

		GameResultWidgetArray[i]->SetVisibility(ESlateVisibility::HitTestInvisible);

		GameResultWidgetArray[i]->SetResult
		(
			numOwningCore >= maxNumCores ? true : false,
			SortedStates[i],
			maxNumAtk,
			maxNumHit
		);
	}
}
