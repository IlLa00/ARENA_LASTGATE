// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OZPlayerSoundComponent.generated.h"

USTRUCT(BlueprintType)
struct FOZGunSoundParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Loudness = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRangeCm = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FalloffRate = 1.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENA_LASTGATE_API UOZPlayerSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOZPlayerSoundComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundBase* GunFireSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	class USoundAttenuation* GunFireAttenuation = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Fog")
	TArray<TObjectPtr<USoundBase>> FogDamageSounds;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|Fog")
	float FogDamageSoundCooldown = 1.0f;

	float LastFogDamageSoundTime = -1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|BattleItem")
	class USoundBase* BattleItemThrowNearSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|BattleItem")
	class USoundBase* BattleItemThrowFarSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Sound|BattleItem")
	float ThrowFarThresholdCm = 1500.f;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PlayGunFireSound(const FVector& SoundLocation, const FOZGunSoundParams& Params);

	void PlayFogDamageSound_Local();

	void PlayBattleItemThrowSound_Local(float ThrowDistanceCm);
};
