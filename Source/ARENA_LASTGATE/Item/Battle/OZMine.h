// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "OZMine.generated.h"

class USphereComponent;
class UParticleSystem;
class UAbilitySystemComponent;
class AOZPlayer;
struct FOZBattleItemData;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class ARENA_LASTGATE_API AOZMine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZMine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Battle_Item_Extent (cm) : ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float ExplosionRadiusCm = 400.f;

	// Battle_Item_Damage (ratio 0~1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float DamageRatio = 0.5f;

	// Battel_Item_Installation_Time (sec) : ��ġ ���ӽð� 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float InstallationTimeSec = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|VFX")
	TObjectPtr<UParticleSystem> ExplosionParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|VFX")
	float ExplosionParticleScale = 1.0f;

	UFUNCTION(NetMulticast, Reliable, Category = "CombatItem|VFX")
	void Multicast_PlayExplosionParticle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> MineSound;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayExplosionSound(USoundBase* Sound, FVector SoundLocation);

	float CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const;

	// ===== Runtime =====
	UPROPERTY(ReplicatedUsing = OnRep_InstigatorPawn, VisibleInstanceOnly, BlueprintReadOnly, Category = "CombatItem|Runtime")
	TObjectPtr<APawn> InstigatorPawn;

	UFUNCTION()
	void OnRep_InstigatorPawn();

	UFUNCTION(BlueprintCallable, Category = "CombatItem|Runtime")
	void InitInstigator(APawn* InInstigatorPawn);

	UFUNCTION(BlueprintCallable, Category = "CombatItem|Runtime")
	void InitFromBattleItemData(const FOZBattleItemData Data);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ArmMine_Server(const FHitResult& Hit);

	UPROPERTY(ReplicatedUsing = OnRep_Armed, VisibleInstanceOnly, BlueprintReadOnly, Category = "CombatItem|Runtime")
	bool bArmed = false;

	UFUNCTION()
	void OnRep_Armed();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float ItemStartDelaySec = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|VFX")
	TObjectPtr<UNiagaraSystem> AuraNiagara;

	UPROPERTY(VisibleAnywhere, Category = "CombatItem|VFX")
	TObjectPtr<UNiagaraComponent> AuraNiagaraComp;

	void UpdateAuraVisibility();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> ExplosionSphere; // ���� Overlap ������(������)

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	bool bExploded = false;

	//UFUNCTION()
	/*void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);*/

	void Explode_Server();

	static UAbilitySystemComponent* ResolveTargetASC(AActor* TargetActor);
	static bool ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth);
	UAbilitySystemComponent* ResolveSourceASC() const;

	void ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC);

	FTimerHandle TriggerPollTimerHandle;
	FTimerHandle StartDelayTimerHandle;

	UFUNCTION()
	void PollTrigger_Server();

	UFUNCTION()
	void OnStartDelayFinished_Server();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BattleItem|SFX")
	TObjectPtr<USoundBase> InstallSound;
};
