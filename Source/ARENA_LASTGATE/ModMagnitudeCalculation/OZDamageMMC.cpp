#include "ModMagnitudeCalculation/OZDamageMMC.h"
#include "Character/Weapon/OZWeaponAttributeSet.h"

UOZDamageMMC::UOZDamageMMC()
{
    BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetBaseDamageAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    DamageMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetDamageMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    DamageAmpMultiDef = FGameplayEffectAttributeCaptureDefinition(
        UOZWeaponAttributeSet::GetDamageAmpMultiAttribute(),
        EGameplayEffectAttributeCaptureSource::Source,
        false);

    RelevantAttributesToCapture.Add(BaseDamageDef);
    RelevantAttributesToCapture.Add(DamageMultiDef);
    RelevantAttributesToCapture.Add(DamageAmpMultiDef);
}

float UOZDamageMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    FAggregatorEvaluateParameters EvaluationParameters;

    float BaseValue = 0.0f;
    float MultiValue = 0.0f;
    float AmpMultiValue = 0.0f;

    GetCapturedAttributeMagnitude(BaseDamageDef, Spec, EvaluationParameters, BaseValue);
    GetCapturedAttributeMagnitude(DamageMultiDef, Spec, EvaluationParameters, MultiValue);
    GetCapturedAttributeMagnitude(DamageAmpMultiDef, Spec, EvaluationParameters, AmpMultiValue);

    // Calculate final damage: BaseDamage * (1 + DamageMulti) * (1 + DamageAmpMulti)
    float Result = BaseValue * (1.0f + MultiValue) * (1.0f + AmpMultiValue);

    UE_LOG(LogTemp, Warning, TEXT("[OZDamageMMC] BaseDamage=%.2f, DamageMulti=%.2f, DamageAmpMulti=%.2f, Result=%.2f"),
        BaseValue, MultiValue, AmpMultiValue, Result);

    return Result;
}
