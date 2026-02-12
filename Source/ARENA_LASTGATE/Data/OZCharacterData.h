#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FOZCharacterData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Char_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Core_num;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Scrap_num;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 HP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Stamina_Regen_Delay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Stamina_Regen_Rate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Walk_Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Sprint_Speed;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Sprint_Stamina_Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 EvLDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Ev_Delay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Ev_Stamina_Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool Ev_Hit_Decision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Aiming_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Aiming_Camera_Delay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Aiming_Move_Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Shield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Shield_Regen_Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Shield_Regen_Rate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Armor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool Incap_Collision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Hit_Move_Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Hit_Move_Speed_Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Viewing_Angle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Camera_depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Peripharal_vision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString ModelingID;
};