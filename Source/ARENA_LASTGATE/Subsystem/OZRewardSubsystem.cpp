#include "Subsystem/OZRewardSubsystem.h"
#include "Engine/DataTable.h"
#include "Data/OZRewardData.h"

void UOZRewardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (RewardDataTableRef.IsNull())
		RewardDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Reward.DT_Reward"));

	RewardDataTable = RewardDataTableRef.LoadSynchronous();

	if (!RewardDataTable)
		UE_LOG(LogTemp, Error, TEXT("[RewardSubsystem] ? Failed to load RewardDataTable!"));
}

FOZRewardData* UOZRewardSubsystem::GetRewardDataByRankID(int32 RankID) const
{
	if (!RewardDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[RewardSubsystem] RewardDataTable is null!"));
		return nullptr;
	}

	TArray<FName> RowNames = RewardDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FOZRewardData* RewardData = RewardDataTable->FindRow<FOZRewardData>(RowName, TEXT(""));
		if (RewardData && RewardData->Rank_ID == RankID)
		{
			return RewardData;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[RewardSubsystem] No reward data found for RankID: %d"), RankID);
	return nullptr;
}
