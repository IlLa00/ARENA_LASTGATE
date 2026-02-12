// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/OZFogData.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffect.h"
#include "Utils/Util.h"
#include "OZBlackFogManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackFogStartShrink, int, NumFogPhase, float, ShrinkingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackFogStartCountingForShrink, float, TimeforWaitting);

UCLASS()
class ARENA_LASTGATE_API AOZBlackFogManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AOZBlackFogManager();

	UFUNCTION(BlueprintCallable, Category = "Fog")
	void StartFogForRound();

	UFUNCTION(BlueprintCallable, Category = "Fog")
	void StopFogForRound();

	// 디버그용(원/상태 출력)
	UPROPERTY(EditAnywhere, Category = "Fog|Debug")
	bool bDrawDebug = true;

	UPROPERTY(EditAnywhere, Category = "Fog|Debug", meta = (ClampMin = "0.01"))
	float DebugDrawInterval = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Fog|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(BlueprintAssignable)
	FOnBlackFogStartShrink OnBlackFogStartShrink;
	UPROPERTY()
	FOnBlackFogStartCountingForShrink OnBlackFogStartCountingForShrink;

	float GetInitialMapRadiusCm() { return InitialMapRadiusCm; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// ===== Data =====
	UPROPERTY(EditAnywhere, Category = "Fog|Data")
	UDataTable* FogDataTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Fog|Data")
	FName RowPrefix = TEXT("Phase_"); // RowName: Phase_1 ~ Phase_5

	UPROPERTY(EditAnywhere, Category = "Fog|Data", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MaxPhase = 5;

	UPROPERTY(EditAnywhere, Category = "Fog|Data")
	float InitialMapRadiusCm = 7100.f;

	UPROPERTY(EditAnywhere, Category = "Fog|Data")
	FVector MapCenter = FVector::ZeroVector;

	// ===== Runtime State (Server Truth) =====
	EOZFogState State = EOZFogState::Idle;

	int32 PhaseIndex = 1; // 1~5 (Fog_Phase와 맞추기)

	FVector PrevCenter = FVector::ZeroVector; 
	float PrevRadiusCm = 0.f;                

	FVector NextCenter = FVector::ZeroVector; 
	float NextRadiusCm = 0.f;

	float StateStartTime = 0.f;
	float StateEndTime = 0.f;

	// Damage tick
	float DamageTickSeconds = 0.5f;
	int32 DamagePerTick = 0;

	FTimerHandle Timer_StateTransition;
	FTimerHandle Timer_DamageTick;
	FTimerHandle Timer_DebugDraw;

	// Outside 캐시(선택: 디버깅/최적화용)
	TSet<TWeakObjectPtr<APawn>> CachedOutsidePawns;

	// ===== Flow =====
	void EnterPhase(int32 NewPhaseIndex);

	void EnterWaiting(const FOZFogData& PhaseData);
	void EnterShrinking(const FOZFogData& PhaseData);
	void EnterFinalSuddenDeath(const FOZFogData& PhaseData);

	void OnWaitingFinished();
	void OnShrinkingFinished();

	// ===== Core Computation =====
	float GetNow() const;
	float GetCurrentRadiusCm(float Now) const;
	FVector GetCurrentCenter(float Now) const;
	bool IsFinalSuddenDeath() const;

	// ===== Center Sampling =====
	FVector PickRandomOffset(float MaxShiftCm);

	// ===== Damage =====
	void TickFogDamage();
	void RefreshDamageTimer(const FOZFogData& PhaseData);

	void ApplyFogDamageToPawn(APawn* Pawn, int32 DamageAmount);

	// ===== Debug =====
	void DebugDraw();
	FName MakeRowName(int32 Phase) const;
	const FOZFogData* FindPhaseRow(int32 Phase) const;

	// ===== Safety =====
	void ClearAllTimers();
	bool IsServer() const;

	// ===== Rep Snapshot Update (Server -> GameState) =====
	UPROPERTY(EditAnywhere, Category = "Fog|Replication", meta = (ClampMin = "0.01"))
	float RepUpdateInterval = 0.05f;

	FTimerHandle Timer_RepUpdate;

	void StartRepUpdate();
	void StopRepUpdate();
	void TickRepUpdate();
	void PushFogSnapshotToGameState();
};
