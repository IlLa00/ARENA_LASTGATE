#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "OZWeaponDivisiveMMC.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZWeaponDivisiveMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UOZWeaponDivisiveMMC();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition BaseAtkDelayDef;
	FGameplayEffectAttributeCaptureDefinition AtkDelayMultiDef;
	FGameplayEffectAttributeCaptureDefinition AtkDelayAmpMultiDef;
};
