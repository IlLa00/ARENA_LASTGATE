#include "ModMagnitudeCalculation/OZWeaponDivisiveMMC.h"
#include "Character/Weapon/OZWeaponAttributeSet.h"
#include "AbilitySystemComponent.h"

UOZWeaponDivisiveMMC::UOZWeaponDivisiveMMC()
{
    BaseAtkDelayDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetBaseAtkDelayAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        true);

    AtkDelayMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetAtkMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    AtkDelayAmpMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetAtkAmpMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    RelevantAttributesToCapture.Add(BaseAtkDelayDef);
    RelevantAttributesToCapture.Add(AtkDelayMultiDef);
    RelevantAttributesToCapture.Add(AtkDelayAmpMultiDef);
}

float UOZWeaponDivisiveMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;

    float BaseValue = 0.0f;
    float MultiValue = 0.0f;
    float AmpMultiValue = 0.0f;

    GetCapturedAttributeMagnitude(BaseAtkDelayDef, Spec, EvaluationParameters, BaseValue);
    GetCapturedAttributeMagnitude(AtkDelayMultiDef, Spec, EvaluationParameters, MultiValue);
    GetCapturedAttributeMagnitude(AtkDelayAmpMultiDef, Spec, EvaluationParameters, AmpMultiValue);

    float Denominator = (1.0f + MultiValue) * (1.0f + AmpMultiValue);
    float Result = BaseValue / Denominator;

    return Result;
}