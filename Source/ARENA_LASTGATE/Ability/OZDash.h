// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NiagaraSystem.h"
#include "OZDash.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZDash : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
    UOZDash();

    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;


protected:
    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float DashSpeed = 1200.f;

    UPROPERTY(EditDefaultsOnly, Category = "Dash")
    float EffectDuration = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Stamina")
    TSubclassOf<class UGameplayEffect> StaminaBlockRegenEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Effect")
    TObjectPtr<UNiagaraSystem> DashEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Dash|Effect")
    FVector DashEffectOffset = FVector::ZeroVector;

    FTimerHandle DashEffectTimerHandle;

    void DeactivateDashEffect(UNiagaraComponent* EffectComp);

};
