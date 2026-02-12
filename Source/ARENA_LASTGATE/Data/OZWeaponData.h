#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZWeaponData.generated.h"

USTRUCT(BlueprintType)
struct FOZWeaponData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Weapon_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseAtkDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 BaseMaxHeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Cooling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float CoolingDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float CoolingBuffer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float HeatCoefficient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 OverheatPenalty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseSpread_Angle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseNoise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float NoiseMaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float NoiseFalloff_Rate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseProjectileSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 BaseProjectileSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 BaseProjectilesPerShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float BaseKnockbackPower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool CanRicochet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 RicochetConut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool CanPierce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float ExplosionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float ExplosionDamage_mult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float ExplosionFalloff_Rate;
};