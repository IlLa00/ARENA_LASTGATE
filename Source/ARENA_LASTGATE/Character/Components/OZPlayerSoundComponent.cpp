// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/OZPlayerSoundComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h" 

// Sets default values for this component's properties
UOZPlayerSoundComponent::UOZPlayerSoundComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UOZPlayerSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOZPlayerSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UOZPlayerSoundComponent::PlayGunFireSound(const FVector& SoundLocation, const FOZGunSoundParams& Params)
{
    if (!GunFireSound)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
        return;

    APawn* OwnerPawn = Cast<APawn>(OwnerActor);

    // 1) 본인(로컬 플레이어) 총소리는 2D로 재생 (거리 무관)
    if (OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        UGameplayStatics::PlaySound2D(this, GunFireSound, Params.Loudness, Params.Pitch);
        return;
    }

    // 2) 타인 총소리만 3D 거리 컷 적용
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
        return;

    FVector ListenerLoc, ListenerFront, ListenerRight;
    PC->GetAudioListenerPosition(ListenerLoc, ListenerFront, ListenerRight);

    const float Distance = FVector::Dist(ListenerLoc, SoundLocation);
    if (Distance >= Params.MaxRangeCm)
        return;

    const float t = FMath::Clamp(Distance / Params.MaxRangeCm, 0.f, 1.f);
    const float Base = 1.f - t;
    const float DistanceMul = FMath::Pow(Base, Params.FalloffRate);
    const float FinalVolume = Params.Loudness * DistanceMul;

    if (FinalVolume <= 0.001f)
        return;

    UGameplayStatics::SpawnSoundAtLocation(
        World,
        GunFireSound,
        SoundLocation,
        FRotator::ZeroRotator,
        FinalVolume,
        Params.Pitch,
        0.f,
        GunFireAttenuation
    );
}

void UOZPlayerSoundComponent::PlayFogDamageSound_Local()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    const float Now = World->GetTimeSeconds();

    // 1초 쿨다운
    if (Now - LastFogDamageSoundTime < FogDamageSoundCooldown)
        return;

    LastFogDamageSoundTime = Now;

    if (FogDamageSounds.Num() == 0)
        return;

    const int32 Index = FMath::RandRange(0, FogDamageSounds.Num() - 1);
    USoundBase* Sound = FogDamageSounds[Index];
    if (!Sound)
        return;

    // 자기만 듣는 2D 사운드 (거리 감쇠 없음)
    UGameplayStatics::PlaySound2D(this, Sound);
}

void UOZPlayerSoundComponent::PlayBattleItemThrowSound_Local(float ThrowDistanceCm)
{
    if (ThrowDistanceCm >= ThrowFarThresholdCm)
    {
        if (BattleItemThrowFarSound)
            UGameplayStatics::PlaySound2D(this, BattleItemThrowFarSound);
    }
    else
    {
        if (BattleItemThrowNearSound)
            UGameplayStatics::PlaySound2D(this, BattleItemThrowNearSound);
    }
}

