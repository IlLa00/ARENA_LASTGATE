// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SlotMachineReward.generated.h"

UENUM(BlueprintType)
enum class EOZSlotSymbol : uint8
{
	HP      UMETA(DisplayName = "HP"),
	Speed   UMETA(DisplayName = "Speed"),
	EvLDistance   UMETA(DisplayName = "EvLDistance"),
	Armor   UMETA(DisplayName = "Armor"),
	Shield  UMETA(DisplayName = "Shield"),
	Stamina UMETA(DisplayName = "Stamina"),
	Jackpot    UMETA(DisplayName = "Jackpot"),
	None    UMETA(DisplayName = "None"),
};

USTRUCT(BlueprintType)
struct FOZGrantedItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemID = -1;

	UPROPERTY(BlueprintReadOnly)
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FOZSlotRewardResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bWin = false;

	// 어떤 조건으로 걸렸는지 디버그용(나중에 UI표시에도 사용 가능)
	UPROPERTY(BlueprintReadOnly)
	FName RuleName = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	TArray<int32> RewardItemIDs;

	UPROPERTY(BlueprintReadOnly)
	uint8 RewardCores = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 RewardScraps = 0;
};

UCLASS()
class ARENA_LASTGATE_API USlotMachineReward : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// BP_SlotMachine에서 Result 3개 모아둔 후 호출
	UFUNCTION(BlueprintCallable, Category = "Slot|Reward")
	FOZSlotRewardResult EvaluateSpinResults(const TArray<EOZSlotSymbol>& Symbols);

	// (선택) 아이콘 경로/이름으로도 호출하고 싶으면 여기서 변환
	UFUNCTION(BlueprintCallable, Category = "Slot|Reward")
	FOZSlotRewardResult EvaluateSpinResults_ByIconPath(const TArray<FString>& IconPaths);

private:
	// IconPath -> Symbol 변환 (지금은 하드코딩, 추후 DT/Map으로 교체)
	EOZSlotSymbol ConvertIconPathToSymbol(const FString& Path) const;

	// 내부 룰 판정/보상 산출
	FOZSlotRewardResult EvaluateInternal(const TArray<EOZSlotSymbol>& Symbols) const;
};