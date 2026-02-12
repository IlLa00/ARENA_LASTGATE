#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZBattleItemData.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FOZBattleItemData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Item_Max_Range;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Item_Projectile_Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Item_Start_Delay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Item_Form;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Form_Extent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Item_Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float Damage_Over_Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_Installation_Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Item_Cost;
      
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Max_Stack;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText Explanation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSoftObjectPtr<UTexture2D> ItemIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
    TSoftClassPtr<UGameplayAbility> ItemAbilityClass;
};