// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/OZDash.h"
#include "Character/OZPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


UOZDash::UOZDash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UOZDash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (StaminaBlockRegenEffectClass && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
        FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();

        ASC->ApplyGameplayEffectToSelf(
            StaminaBlockRegenEffectClass->GetDefaultObject<UGameplayEffect>(),
            1.f,
            Ctx
        );
    }

    AOZPlayer* OZPlayer = Cast<AOZPlayer>(ActorInfo->AvatarActor.Get());
    if (!OZPlayer)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector MoveDir = OZPlayer->GetVelocity();
    MoveDir.Z = 0.f;

    if (MoveDir.IsNearlyZero())
    {
        MoveDir = OZPlayer->GetActorForwardVector();
        MoveDir.Z = 0.f;
    }

    if (!MoveDir.Normalize())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const float AngleRad = FMath::Atan2(MoveDir.Y, MoveDir.X);
    float AngleDeg = FMath::RadiansToDegrees(AngleRad);

    float SnappedAngle = 45.f * FMath::RoundToInt(AngleDeg / 45.f);

    FRotator DashRot(0.f, SnappedAngle, 0.f);
    FVector DashDir = DashRot.Vector();

    const FVector DashVelocity = DashDir * DashSpeed;

    if (DashEffect)
    {
        FVector EffectLocation = OZPlayer->GetActorLocation() + DashEffectOffset;
        FRotator EffectRotation = DashDir.Rotation();

        UNiagaraComponent* SpawnedEffect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            DashEffect,
            GetAvatarActorFromActorInfo()->GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(1.f),
            true,
            true,
            ENCPoolMethod::AutoRelease
        );
    }

    if (OZPlayer->HasAuthority())
    {
        OZPlayer->LaunchCharacter(DashVelocity, true, true);
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
