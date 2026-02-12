#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OZRewardSubsystem.generated.h"

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZRewardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon Data")
	class UDataTable* GetRewardDataTable() const { return RewardDataTable; }

	struct FOZRewardData* GetRewardDataByRankID(int32 RankID) const;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Reward Data")
	TSoftObjectPtr<UDataTable> RewardDataTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> RewardDataTable;
};
