#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZBuffItemData.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FOZBuffItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Target_Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Item_Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int Item_Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int Item_Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int Max_Stack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Explanation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UTexture2D> ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    TSoftClassPtr<UGameplayAbility> ItemAbilityClass;
};