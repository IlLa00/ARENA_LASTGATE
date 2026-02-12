#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZRewardData.generated.h"

USTRUCT(BlueprintType)
struct FOZRewardData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Rank_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Reward_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Base_Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Kill_Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Rank_Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool Round_WinnerReward;
};