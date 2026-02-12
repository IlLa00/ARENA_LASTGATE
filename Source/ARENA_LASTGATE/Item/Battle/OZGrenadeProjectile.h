// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "OZGrenadeProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UAbilitySystemComponent;
class AOZPlayer;
class AOZFireArea;
class AOZSmokeArea;
class UParticleSystem;
class UNiagaraSystem;
struct FOZBattleItemData;

UENUM(BlueprintType)
enum class EOZGrenadeImpactMode : uint8
{
	ExplodeDamage UMETA(DisplayName = "Explode (Damage)"),
	SpawnFireArea UMETA(DisplayName = "Spawn FireArea"),
	FlashBang UMETA(DisplayName = "FlashBang (Blind)"),
	SpawnSmokeArea UMETA(DisplayName = "Spawn SmokeArea"),
};

UCLASS()
class ARENA_LASTGATE_API AOZGrenadeProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZGrenadeProjectile();

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	void InitInstigator(APawn* InInstigatorPawn);

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	void InitFromBattleItemData(const FOZBattleItemData Data);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> TrailComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec")
	EOZGrenadeImpactMode ImpactMode = EOZGrenadeImpactMode::ExplodeDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec")
	float ExplosionRadiusCm = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec", meta = (ClampMin = "0.0"))
	float DamageRatio = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec")
	bool bCanDamageInstigator = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	TSubclassOf<AOZFireArea> FireAreaClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	bool bSnapFireAreaToGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	float GroundTraceUpOffsetCm = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	float GroundTraceDownLengthCm = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	TObjectPtr<UNiagaraSystem> FireImpactEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Fire")
	float FireImpactEffectScale = 1.0f;

	UFUNCTION(NetMulticast, Reliable, Category = "BattleItem|VFX")
	void Multicast_PlayFireImpactEffect(FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Smoke")
	TSubclassOf<AOZSmokeArea> SmokeAreaClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|Smoke")
	bool bSnapSmokeAreaToGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|FlashBang", meta = (ClampMin = "0.0"))
	float FlashBangDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|FlashBang")
	TObjectPtr<UNiagaraSystem> FlashBangEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|FlashBang")
	float FlashBangEffectScale = 1.0f;

	UFUNCTION(NetMulticast, Reliable, Category = "BattleItem|VFX")
	void Multicast_PlayFlashBangEffect(FVector Location);

	// 섬광탄 맞은 플레이어에게만 재생되는 섬광탄 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec|FlashBang")
	TObjectPtr<UNiagaraSystem> FlashBangHitEffect;

	// 섬광탄에 맞은 플레이어에게 나이아가라 이펙트를 재생
	UFUNCTION(NetMulticast, Reliable, Category = "BattleItem|VFX")
	void Multicast_PlayFlashBangHitEffect(const TArray<FVector>& HitLocations);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|VFX")
	TObjectPtr<UParticleSystem> ExplosionParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|VFX")
	float ExplosionParticleScale = 1.0f;

	UFUNCTION(NetMulticast, Reliable, Category = "BattleItem|VFX")
	void Multicast_PlayExplosionParticle(FVector Location);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "BattleItem|Runtime")
	TObjectPtr<APawn> InstigatorPawn;

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Runtime")
	void InitVelocity(const FVector& InWorldVelocity);

	bool bImpacted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|Spec", meta = (ClampMin = "0.0"))
	float ItemStartDelaySec = 0.f;

	FTimerHandle StartDelayTimerHandle;

	bool bStartDelayStarted = false;

	FHitResult CachedImpactResult;

	UFUNCTION()
	void OnStartDelayFinished_Server();

	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);

	void HandleImpact_Server(const FHitResult& ImpactResult);
	void ExplodeDamage_Server(const FHitResult& ImpactResult);
	void SpawnFireArea_Server(const FHitResult& ImpactResult);
	void FlashBang_Server(const FHitResult& ImpactResult);
	void SpawnSmokeArea_Server(const FHitResult& ImpactResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> GrenadeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> SmokeShellSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> FlashBangSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> FireGrenadeSound;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayExplosionSound(USoundBase* Sound, FVector SoundLocation, bool bInstigatorOnly);

	float CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const;

	UAbilitySystemComponent* ResolveSourceASC() const;
	static UAbilitySystemComponent* ResolveTargetASC(AActor* TargetActor);
	static bool ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth);

	void ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC);

	bool FindGroundPointFromImpact(const FHitResult& ImpactResult, FVector& OutGroundPoint) const;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
