// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZSmokeArea.generated.h"

class USphereComponent;
class AOZPlayer;
class UParticleSystem;
class UAudioComponent;
struct FOZBattleItemData;

UCLASS()
class ARENA_LASTGATE_API AOZSmokeArea : public AActor
{
	GENERATED_BODY()

public:
	AOZSmokeArea();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec")
	float RadiusCm = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec")
	float DurationSec = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|VFX")
	TObjectPtr<UParticleSystem> SmokeParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|VFX")
	float SmokeParticleScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> SmokeLoopSound;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BattleItem|Runtime")
	TObjectPtr<APawn> InstigatorPawn;

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	void InitInstigator(APawn* InInstigatorPawn);

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	void InitFromBattleItemData(const FOZBattleItemData Data);

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	bool IsPlayerInside(AOZPlayer* Player) const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SmokeSphere;

	UPROPERTY()
	TObjectPtr<UAudioComponent> SmokeAudioComp;

	FTimerHandle VolumeUpdateTimerHandle;

	void UpdateSoundVolume();
	float CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const;
};
