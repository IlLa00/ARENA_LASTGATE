// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/Util.h"
#include "OZFogVisualActor.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZFogVisualActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZFogVisualActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION(BlueprintCallable, Category = "Fog|Visual")
	void ApplyFogSnapshot(
		EOZFogState InState,
		int32 InPhaseIndex,
		const FVector& InCenter,
		float InSafeRadiusCm,
		float InInitialMapRadiusCm
	);

	UFUNCTION(BlueprintCallable, Category = "Fog|Visual")
	void SetFogVisible(bool bVisible);

private:
	UPROPERTY(VisibleAnywhere, Category = "Fog|Visual")
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Fog|Visual")
	class UNiagaraComponent* FogNiagaraComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Fog|Visual")
	class UNiagaraComponent* BackdropNiagaraComp = nullptr;

	// ===== Niagara Asset =====
	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	class UNiagaraSystem* FogNiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	class UMaterialInterface* FogMaterial = nullptr;   // UMaterial 대신 Interface 추천

	UPROPERTY(Transient)
	class UMaterialInstanceDynamic* FogMID = nullptr;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	FName MatParamCenterPos = TEXT("CenterPos");

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	FName MatParamRadius = TEXT("Radius");


	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	float NiagaraZOffset = 100.f;

	// 파라미터명(에셋과 반드시 일치)
	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	FName ParamRadius = TEXT("User.Radius");

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	FName ParamHandleRadius = TEXT("User.HandleRadius");

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara")
	FName ParamSpawnRate = TEXT("User.SpawnRate");

	// ===== SpawnRate(Phase 기반) =====
	UPROPERTY(EditAnywhere, Category = "Fog|Niagara|SpawnRate")
	bool bUsePhaseSpawnRate = true;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara|SpawnRate", meta = (EditCondition = "bUsePhaseSpawnRate"))
	float SpawnRateStart = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara|SpawnRate", meta = (EditCondition = "bUsePhaseSpawnRate"))
	float SpawnRateStepPerPhase = 200.f;

	UPROPERTY(Transient)
	float CachedOuterRadiusCm = 0.f;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara|Backdrop")
	float BackdropExtraRadiusCm = 2500.f;

	UPROPERTY(EditAnywhere, Category = "Fog|Niagara|Backdrop", meta = (EditCondition = "bBackdropUseOwnSpawnRate"))
	float BackdropSpawnRate = 7500.f;

private:
	void EnsureNiagaraReady();
	void UpdateNiagaraParams(
		int32 PhaseIndex,
		const FVector& Center,
		float SafeRadiusCm,
		float InitialMapRadiusCm
	);
};
