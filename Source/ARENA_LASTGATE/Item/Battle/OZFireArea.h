// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZFireArea.generated.h"

class USphereComponent;
class UAbilitySystemComponent;
class AOZPlayer;
class UNiagaraSystem;
class UNiagaraComponent;
struct FOZBattleItemData;

UCLASS()
class ARENA_LASTGATE_API AOZFireArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZFireArea();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	// Battle_Item_Extent 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float RadiusCm = 400.f;

	// Battle_Item_Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float DamageRatio = 0.1f;

	// Battle_Item_Damage_Delay 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float TickIntervalSec = 0.5f;

	// Battel_Item_Installation_Time 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Spec")
	float InstallationTimeSec = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|Debug")
	bool bDebugDraw = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|VFX")
	TObjectPtr<UNiagaraSystem> FireEffect;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> FireEffectComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatItem|VFX")
	float FireEffectScale = 1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "CombatItem|Runtime")
	TObjectPtr<APawn> InstigatorPawn;

	UFUNCTION(BlueprintCallable, Category = "CombatItem|Runtime")
	void InitInstigator(APawn* InInstigatorPawn);

	UFUNCTION(BlueprintCallable, Category = "CombatItem|Runtime")
	void InitFromBattleItemData(const FOZBattleItemData Data);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> AreaSphere;

	FTimerHandle TickTimerHandle;

	// 현재 영역 내에 있는 플레이어들 (이펙트 관리용)
	UPROPERTY(Transient)
	TSet<TObjectPtr<AOZPlayer>> PlayersInArea;

	// Overlap 이벤트
	UFUNCTION()
	void OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Server only
	void TickDamage_Server();

	// ��ƿ: TargetActor �� TargetASC ã�� (Actor or PlayerState)
	static UAbilitySystemComponent* ResolveTargetASC(AActor* TargetActor);

	// ��ƿ: SourceASC(Instigator�� ASC) ã��
	UAbilitySystemComponent* ResolveSourceASC() const;

	// ��ƿ: TargetASC���� MaxHealth ���
	static bool ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth);

	// ��� 1������ % ������ ����
	void ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC);
};
