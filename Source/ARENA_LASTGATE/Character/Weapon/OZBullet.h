// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "OZBullet.generated.h"

USTRUCT(BlueprintType)
struct FOZBulletInitParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    AActor* Shooter = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    float InitialDamage = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    float CollisionRadius = 2.f;

    // Ricochet
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init|Ricochet")
    bool bRicochet = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init|Ricochet")
    int32 RicochetCount = 0;

    // Explosion
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init|Explosion")
    float ExplosionRadius = 0.f;    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init|Explosion")
    float ExplosionDamageMult = 0.f;    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init|Explosion")
    float ExplosionFalloffRate = 1.f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    float MaxRangeCm = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    float ProjectileSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Init")
    FVector InitialVelocity = FVector::ZeroVector;
};

UCLASS()
class ARENA_LASTGATE_API AOZBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZBullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 충돌 처리용 구체 콜리전 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet")
    class USphereComponent* CollisionComp;

    /** 이동 처리 (속도 / 중력 / 바운스 등) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet")
    class UProjectileMovementComponent* ProjectileMovement;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet")
    TObjectPtr<AActor> Shooter = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet")
    float CurrentDamage = 0.f;

    // 도탄 여부 플래그
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet")
    bool bRicochet = false;
    
    // 관통 레벨 계산 함수
    int32 CalcPierceLevel(float Damage) const;

    // 관통 레벨에 따른 데미지 감소 함수
    float GetPenetrationReduceRate(int32 ObjectResistLevel) const;

    // 관통 불가 태그
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|Penetration")
    FGameplayTag PenetrationBlockedTag;

    // 같은 벽에서 여러 번 비용 차감되는 걸 방지
    UPROPERTY()
    TSet<TWeakObjectPtr<AActor>> PenetratedActors;

    // Overlap 콜백
    UFUNCTION()
    void OnBulletOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    // 도탄 처리
    UFUNCTION()
    void PerformRicochet(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult);

    // 도탄 튜닝
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|Ricochet")
    float RicochetCooldownSeconds = 0.05f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|Ricochet")
    float RicochetPushOutDistance = 5.f; // cm

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|Ricochet")
    float RicochetTraceDistance = 30.f;  // cm (normal 보강용 sweep 길이)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|Ricochet")
    float RicochetSpeedRetain = 1.0f; // 1이면 감쇠 없음

    UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Bullet|Ricochet")
    int32 RemainingRicochetCount = 0;

    // --- Ricochet 런타임 캐시 ---
    float LastRicochetTimeSeconds = -1000.f;
    FVector LastValidRicochetNormal = FVector::UpVector;
    TWeakObjectPtr<const UPrimitiveComponent> LastRicochetComp;

    // normal 보강용 helper
    bool TryGetRicochetNormal(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult, FVector& OutNormal) const;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet|Explosion")
    float ExplosionRadius = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet|Explosion")
    float ExplosionDamageMult = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet|Explosion")
    float ExplosionFalloffRate = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Bullet|Explosion|Debug")
    bool bDrawExplosionDebug = true;

    void ExecuteExplosion(const FVector& Center);
    bool HasExplosionLineOfSight(const FVector& Center, const AActor* TargetActor) const;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Bullet|Explosion|VFX")
    TObjectPtr<class UNiagaraSystem> ExplosionNiagara = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Bullet|Explosion|VFX")
    float ExplosionVfxScalePerRadius = 0.01f;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayExplosionFX(FVector Center, float Radius);

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Bullet|Range")
    float MaxRangeCm = 0.f;

    UPROPERTY()
    FVector StartLocation;

    FTimerHandle RangeCheckTimerHandle;

    void CheckRangeExceeded();

    bool bHandlingOverlap = false;
    bool bInitBullet = false;

    bool TrySpawnSafeLocation(const FVector& ShootDir);

    UPROPERTY(ReplicatedUsing = OnRep_BulletVisual)
    float RepCollisionRadius = 15.f;

    UPROPERTY(ReplicatedUsing = OnRep_BulletVisual)
    float RepNiagaraScale = 1.f; 

    // --- VFX Scale Mapping (CollisionRadius -> Niagara User.Scale) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    float VfxBaseCollisionRadius = 15.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    float VfxTargetCollisionRadius = 60.f; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    float VfxBaseScale = 1.f; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    float VfxTargetScale = 8.f; 

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    float VfxMaxScale = 15.f; 

    UFUNCTION()
    void OnRep_BulletVisual();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    TObjectPtr<class UNiagaraComponent> BulletNiagaraComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    TObjectPtr<UNiagaraSystem> BulletNiagaraAsset = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Bullet|VFX")
    FName BulletScaleUserParamName = TEXT("User.Scale");
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // 발사 직후 초기화용
    UFUNCTION(BlueprintCallable, Category = "Weapon|Bullet")
    void InitBullet(const FOZBulletInitParams& Params);

    UFUNCTION(BlueprintPure, Category = "Weapon|Bullet")
    float GetCurrentDamage() const { return CurrentDamage; }
};
