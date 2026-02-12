// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZRoundComBatResultUI.h"
#include "Character/OZPlayerState.h"
#include "Subsystem/OZRewardSubsystem.h"
#include "Data/OZRewardData.h"
#include "Widget/OZCombatResultInfoBar.h"

//void UOZRoundComBatResultUI::SetRoundResultRemainTime(uint8 remainingTime)
//{
//	//Text_RoundResultTimeRemian->SetText(FText::AsNumber(remainingTime));
//}

void UOZRoundComBatResultUI::SetCombatBonusPopup(ECombatBonus bonuseType, TArray<FString> bonusePlayerNames)
{
	if (bonuseType == ECombatBonus::Max || bonusePlayerNames.IsEmpty())
		return;

	combatBonusType = bonuseType;

	bonusePlayerNameArray.Empty();
	bonusePlayerNameArray = bonusePlayerNames;

	PrefareBonuseTextBlock();

	HighlightWinner();

	DisaplyPreBonuscores();

	ShowCombatBonusePopup();

}

void UOZRoundComBatResultUI::ReportCombatBonusRewardPopupEnd()
{
	OwnerPlayerController->Server_ReportCombatBonusRewardPopupEnd();
}

void UOZRoundComBatResultUI::OnRewardPopupDisapearAnimStarted(TArray<FString> rewaredPlayerNames, FVector2D startPos, const FString& upperName)
{
	for (int i = 0; i < rewaredPlayerNames.Num(); i++)
	{
		FString rewaredPlayerName = rewaredPlayerNames[i];

		int barIndex = 0;
		for (UOZCombatResultInfoBar* UserCombatResultBar : CombatResultBars)
		{
			//UserCombatResultBar->ReturnInitPos();
			FString playerName = UserCombatResultBar->Text_PlayerName->GetText().ToString();

			if (playerName.IsEmpty())
			{

				//CombatResultBars[0]->ReturnInitPos();
				FVector2D endPos1 = CombatResultBars[0]->GetRewardEffectPos(0);

				if (barIndex == 0 && FirstWinnerTargetPos.X == 0.f)
				{
					FirstWinnerTargetPos = endPos1;
				}

				//CombatResultBars[1]->ReturnInitPos();
				FVector2D endPos2 = CombatResultBars[1]->GetRewardEffectPos(1);

				//CombatResultBars[2]->ReturnInitPos();
				FVector2D endPos3 = CombatResultBars[2]->GetRewardEffectPos(2);

				//CombatResultBars[3]->ReturnInitPos();
				FVector2D endPos4 = CombatResultBars[3]->GetRewardEffectPos(3);

				if(endPos1.X > 0 && endPos1.Y > 0)
					OnPlayRewardCoreAnim(startPos, FirstWinnerTargetPos, RewardTimeLine, 0, upperName);

				if (endPos2.X > 0 && endPos2.Y > 0)
					OnPlayRewardCoreAnim(startPos, endPos2, RewardTimeLine, 1, upperName);
				
				if (endPos3.X > 0 && endPos3.Y > 0)
					OnPlayRewardCoreAnim(startPos, endPos3, RewardTimeLine, 2, upperName);
				
				if (endPos4.X > 0 && endPos4.Y > 0)
					OnPlayRewardCoreAnim(startPos, endPos4, RewardTimeLine, 3, upperName);
				
				return;
			}


			if (rewaredPlayerName.Equals(playerName) == false)
			{
				barIndex++;
				continue;
			}
				

			FVector2D endPos = UserCombatResultBar->GetRewardEffectPos(barIndex);
			//UserCombatResultBar->ReturnInitPos();

			if (barIndex == 0 && FirstWinnerTargetPos.X == 0.f)
			{
				FirstWinnerTargetPos = endPos;
			}

			if(barIndex != 0)
				OnPlayRewardCoreAnim(startPos, endPos, RewardTimeLine, barIndex, upperName);

			else if (barIndex == 0)
				OnPlayRewardCoreAnim(startPos, FirstWinnerTargetPos, RewardTimeLine, barIndex, upperName);
			
		}
	}
}

void UOZRoundComBatResultUI::HighlightWinner()
{
	for (int i = 0; i < CombatResultBars.Num(); i++)
	{

		bool bIsWinner = false;

		if (i == 0)
			bIsWinner = true;

		CombatResultBars[i]->SetbIsHighlighted(bIsWinner);
	}
}

void UOZRoundComBatResultUI::DisaplyPreBonuscores()
{
	for (int i = 0; i < bonusePlayerNameArray.Num(); i++)
	{
		FString rewaredPlayerName = bonusePlayerNameArray[i];

		for (UOZCombatResultInfoBar* ResultInfoBar : CombatResultBars)
		{

			AOZPlayerState* playerState = ResultInfoBar->assingedPlayerState;

			if (playerState == nullptr)
				continue;

			if (playerState->UserName.Equals(rewaredPlayerName) == false)
				continue;

			FText Text_Cores = ResultInfoBar->Text_Core->GetText();

			FString Str = Text_Cores.ToString();

			if (!Str.IsNumeric())
				continue;

			int32 num_cores = FCString::Atoi(*Str);

			num_cores--;

			num_cores == num_cores < 0 ? 0 : num_cores;

			ResultInfoBar->Text_Core->SetText(FText::AsNumber(num_cores));
		}
	}
}

void UOZRoundComBatResultUI::PrefareBonuseTextBlock()
{
	UOZRewardSubsystem* RewardSubsystem = GetGameInstance()->GetSubsystem<UOZRewardSubsystem>();

	if (RewardSubsystem == nullptr)
		return;

	int32 countIndex = 0;

	for (UOZCombatResultInfoBar* ResultInfoBar : CombatResultBars)
	{
		AOZPlayerState* playerState = ResultInfoBar->assingedPlayerState;

		if (playerState == nullptr)
			continue;

		int32 RankID = 40000 + (playerState->Round_SurvivalRanking - 1);

		FOZRewardData* RewardData = RewardSubsystem->GetRewardDataByRankID(RankID);
		if (RewardData)
		{
			int32 combatBaseReward = RewardData->Base_Reward;
			int32 combatSurvibalBonus = RewardData->Rank_Reward;
			int32 combatKillBonus = playerState->Round_KillCount * RewardData->Kill_Reward;

			ResultInfoBar->SetBonusScrap(combatBaseReward + combatSurvibalBonus, combatKillBonus);
		}

		ResultInfoBar->ToggleDisplayedParams(true);
	}
}
