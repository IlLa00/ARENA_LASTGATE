#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZConvertData.generated.h"

UENUM(BlueprintType)
enum class EConvertGrade : uint8
{
	None		UMETA(DisplayName = "None"),
	Rare		UMETA(DisplayName = "Rare"),
	Unique		UMETA(DisplayName = "Unique"),
	Legendary	UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FOZConvertData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EConvertGrade Grade;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DamageAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageAmpMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AtkDelayMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AtkDelayAmpMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxHeatAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CoolingAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HeatCoefficientMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpreadAngleMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AmpSpreadAngleMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NoiseMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AmpNoiseMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NoiseMaxRangeMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RangeAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RangeMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ProjectileSpeedAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ProjectileSpeedMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ProjectileSizeAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ProjectilesPershotAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 KnockbackPowerAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseKnockbackPowerMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RicochetConutAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ExplosionRadiusAdd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText tooltip_text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class UGameplayEffect> Convert_Asset;
};
