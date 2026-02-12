#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "OZSpreadAngleMMC.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZSpreadAngleMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UOZSpreadAngleMMC();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	FGameplayEffectAttributeCaptureDefinition BaseSpreadAngleDef;
	FGameplayEffectAttributeCaptureDefinition SpreadAngleMultiDef;
	FGameplayEffectAttributeCaptureDefinition SpreadAngleAmpMultiDef;
	
};
