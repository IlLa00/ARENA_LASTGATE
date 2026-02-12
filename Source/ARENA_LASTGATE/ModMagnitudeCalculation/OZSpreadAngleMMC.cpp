#include "ModMagnitudeCalculation/OZSpreadAngleMMC.h"
#include "Character/Weapon/OZWeaponAttributeSet.h"

UOZSpreadAngleMMC::UOZSpreadAngleMMC()
{
    // ¼Ó¼º Ä¸Ã³

    BaseSpreadAngleDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetBaseSpreadAngleAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        true);

    SpreadAngleMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetSpreadAngleMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    SpreadAngleAmpMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetSpreadAngleAmpMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    RelevantAttributesToCapture.Add(BaseSpreadAngleDef);
    RelevantAttributesToCapture.Add(SpreadAngleMultiDef);
    RelevantAttributesToCapture.Add(SpreadAngleAmpMultiDef);
}

float UOZSpreadAngleMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;

    float BaseValue = 0.0f;
    float MultiValue = 0.0f;
    float AmpMultiValue = 0.0f;

    GetCapturedAttributeMagnitude(BaseSpreadAngleDef, Spec, EvaluationParameters, BaseValue);
    GetCapturedAttributeMagnitude(SpreadAngleMultiDef, Spec, EvaluationParameters, MultiValue);
    GetCapturedAttributeMagnitude(SpreadAngleAmpMultiDef, Spec, EvaluationParameters, AmpMultiValue);

    // float Result = BaseValue / (1 + MultiValue);
    float Denominator = (1.0f + MultiValue) * (1.0f + AmpMultiValue);
    float Result = BaseValue / Denominator;

    return Result;
}
