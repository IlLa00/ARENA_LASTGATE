#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "OZDamageMMC.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZDamageMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UOZDamageMMC();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;
	FGameplayEffectAttributeCaptureDefinition DamageMultiDef;
	FGameplayEffectAttributeCaptureDefinition DamageAmpMultiDef;
};
