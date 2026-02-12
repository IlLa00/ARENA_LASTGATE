// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/SlotMachineReward.h"
#include "Kismet/KismetStringLibrary.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Character/OZPlayerState.h"

FOZSlotRewardResult USlotMachineReward::EvaluateSpinResults(const TArray<EOZSlotSymbol>& Symbols)
{
	return EvaluateInternal(Symbols);
}

FOZSlotRewardResult USlotMachineReward::EvaluateSpinResults_ByIconPath(const TArray<FString>& IconPaths)
{
	TArray<EOZSlotSymbol> Symbols;
	Symbols.Reserve(IconPaths.Num());

	for (const FString& P : IconPaths)
	{
		Symbols.Add(ConvertIconPathToSymbol(P));
	}

	return EvaluateInternal(Symbols);
}

EOZSlotSymbol USlotMachineReward::ConvertIconPathToSymbol(const FString& Path) const
{
	const FString Lower = Path.ToLower();

	if (Lower.Contains(TEXT("hp"))) return EOZSlotSymbol::HP;
	if (Lower.Contains(TEXT("evldistance"))) return EOZSlotSymbol::EvLDistance;
	if (Lower.Contains(TEXT("movedpeed"))) return EOZSlotSymbol::Speed;
	if (Lower.Contains(TEXT("armor"))) return EOZSlotSymbol::Armor;
	if (Lower.Contains(TEXT("shield")))  return EOZSlotSymbol::Shield;
	if (Lower.Contains(TEXT("stamina"))) return EOZSlotSymbol::Stamina;
	if (Lower.Contains(TEXT("jackpot"))) return EOZSlotSymbol::Jackpot;

	return EOZSlotSymbol::None;
}

FOZSlotRewardResult USlotMachineReward::EvaluateInternal(const TArray<EOZSlotSymbol>& Symbols) const
{
	FOZSlotRewardResult Out;

	UOZItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UOZItemSubsystem>();

	auto AddReward = [&](int32 ItemID)
		{
			if (ItemSubsystem && ItemSubsystem->GetBuffItemData(ItemID))
			{
				Out.RewardItemIDs.Add(ItemID);
			}
		};

	auto AddScrap = [&](int32 Amount)
		{
			Out.RewardScraps += Amount;
		};

	auto AddCore = [&](uint8 Amount)
		{
			Out.RewardCores = (uint8)FMath::Clamp<int32>((int32)Out.RewardCores + (int32)Amount, 0, 255);
		};

	// 방어적 체크: 슬롯 3개 기준
	if (Symbols.Num() != 3)
	{
		Out.bWin = false;
		Out.RuleName = "InvalidSymbolCount";
		return Out;
	}

	// 카운트 집계
	TMap<EOZSlotSymbol, int32> CountMap;
	for (EOZSlotSymbol S : Symbols)
	{
		CountMap.FindOrAdd(S)++;
	}

	auto GetCount = [&CountMap](EOZSlotSymbol S) -> int32
		{
			if (const int32* Found = CountMap.Find(S)) return *Found;
			return 0;
		};


	const bool bHP = GetCount(EOZSlotSymbol::HP) >= 1;
	const bool bArmor = GetCount(EOZSlotSymbol::Armor) >= 1;
	const bool bShield = GetCount(EOZSlotSymbol::Shield) >= 1;
	if (bHP && bArmor && bShield)
	{
		Out.bWin = true;
		Out.RuleName = "HP_Armor_Shield";

		for (int i = 0; i < 2; i++)
		{
			AddReward(30000);
			AddReward(30003);
			AddReward(30004);
		}
		return Out;
	}

	// 2) 이동속도+회피+스태미너 조합
	const bool bSpeed = GetCount(EOZSlotSymbol::Speed) >= 1;
	const bool bEvLDistance = GetCount(EOZSlotSymbol::EvLDistance) >= 1;
	const bool bStamina = GetCount(EOZSlotSymbol::Stamina) >= 1;
	if (bSpeed && bEvLDistance && bStamina)
	{
		Out.bWin = true;
		Out.RuleName = "Speed_Dodge_Stamina";
		for (int i = 0; i < 3; i++)
		{
			AddReward(30002);
			AddReward(30003);
		}
		
		return Out;
	}

	for (const TPair<EOZSlotSymbol, int32>& KVP : CountMap)
	{
		if (KVP.Key != EOZSlotSymbol::None && KVP.Value >= 3)
		{
			Out.bWin = true;
			Out.RuleName = "TripleSame";

			AddScrap(1000);

			return Out;
		}
	}

	// 4) Jackpot 3개
	if (GetCount(EOZSlotSymbol::Jackpot) >= 3)
	{
		Out.bWin = true;
		Out.RuleName = "TripleStar";
		
		AddCore(1);
				
		return Out;
	}

	// 5) 같은 심볼 2개 -> 해당 심볼에 맞는 아이템 2개 (예시)
	for (const TPair<EOZSlotSymbol, int32>& KVP : CountMap)
	{
		if (KVP.Key != EOZSlotSymbol::None && KVP.Value >= 2 && GetCount(EOZSlotSymbol::Jackpot) < 2)
		{
			Out.bWin = true;
			Out.RuleName = "DoubleSame";

			switch (KVP.Key)
			{
			case EOZSlotSymbol::HP:      AddReward(30000);AddReward(30000); break;
			case EOZSlotSymbol::Speed:   AddReward(30002);AddReward(30002); break;
			case EOZSlotSymbol::EvLDistance:   AddReward(30001);AddReward(30001); break;
			case EOZSlotSymbol::Armor:   AddReward(30003);AddReward(30003); break;
			case EOZSlotSymbol::Shield:  AddReward(30004);AddReward(30004); break;
			case EOZSlotSymbol::Stamina: AddReward(30001);AddReward(30001); break;
			default: break;
			}

			return Out;
		}
	}

	// 6) 그 외 조합: 무보상
	Out.bWin = false;
	Out.RuleName = "NoReward";
	return Out;
}
