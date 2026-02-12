// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZFogVisualActor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AOZFogVisualActor::AOZFogVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;
	SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FogNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogNiagaraComp"));
	FogNiagaraComp->SetupAttachment(Root);
	FogNiagaraComp->SetAutoActivate(false);
	FogNiagaraComp->SetHiddenInGame(true);

	BackdropNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BackdropNiagaraComp"));
	BackdropNiagaraComp->SetupAttachment(Root);
	BackdropNiagaraComp->SetAutoActivate(false);
	BackdropNiagaraComp->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void AOZFogVisualActor::BeginPlay()
{
	Super::BeginPlay();
	
	EnsureNiagaraReady();

	if (FogMaterial)
	{
		FogMID = UMaterialInstanceDynamic::Create(FogMaterial, this);

		// Niagara에 재질 파라미터로 넣기
		if (FogNiagaraComp)
		{
			FogNiagaraComp->SetVariableMaterial(FName(TEXT("MaterialParam")), FogMID);
		}
	}
}

// Called every frame
void AOZFogVisualActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZFogVisualActor::EnsureNiagaraReady()
{
	if (!FogNiagaraComp)
		return;

	// 에셋 지정되어 있으면 컴포넌트에 연결
	if (FogNiagaraSystem)
	{
		if (FogNiagaraComp && FogNiagaraComp->GetAsset() != FogNiagaraSystem)
		{
			FogNiagaraComp->SetAsset(FogNiagaraSystem);
		}

		if (BackdropNiagaraComp && BackdropNiagaraComp->GetAsset() != FogNiagaraSystem)
		{
			BackdropNiagaraComp->SetAsset(FogNiagaraSystem);
		}
	}
}

void AOZFogVisualActor::SetFogVisible(bool bVisible)
{
	if (!FogNiagaraComp)
		return;

	if (!bVisible)
	{
		FogNiagaraComp->Deactivate();
		FogNiagaraComp->SetHiddenInGame(true);

		if (BackdropNiagaraComp)
		{
			BackdropNiagaraComp->Deactivate();
			BackdropNiagaraComp->SetHiddenInGame(true);
		}
		return;
	}

	EnsureNiagaraReady();
	FogNiagaraComp->SetHiddenInGame(false);
	FogNiagaraComp->Activate(true);

	if (BackdropNiagaraComp)
	{
		BackdropNiagaraComp->SetHiddenInGame(false);
		BackdropNiagaraComp->Activate(true);
	}
}

void AOZFogVisualActor::ApplyFogSnapshot(
	EOZFogState InState,
	int32 InPhaseIndex,
	const FVector& InCenter,
	float InSafeRadiusCm,
	float InInitialMapRadiusCm
)
{
	// Idle면 끔
	if (InState == EOZFogState::Idle)
	{
		CachedOuterRadiusCm = 0.f;
		SetFogVisible(false);
		return;
	}

	if (CachedOuterRadiusCm <= 0.f)
	{
		CachedOuterRadiusCm = FMath::Max(0.f, InInitialMapRadiusCm) + FMath::Max(0.f, BackdropExtraRadiusCm);
	}

	// 그 외 상태면 켬
	SetFogVisible(true);

	UpdateNiagaraParams(InPhaseIndex, InCenter, InSafeRadiusCm, InInitialMapRadiusCm);
}

void AOZFogVisualActor::UpdateNiagaraParams(
	int32 PhaseIndex,
	const FVector& Center,
	float SafeRadiusCm,
	float InitialMapRadiusCm
)
{
	if (!FogNiagaraComp)
		return;

	// 위치 동기화 (중심 따라가기)
	FVector Loc = Center;
	Loc.Z += NiagaraZOffset;
	SetActorLocation(Loc);

	// BaseRadius는 맵 최대 반경(고정 값)
	const float BaseRadius = FMath::Max(InitialMapRadiusCm, 0.f);

	// SafeRadius는 현재 안전구역 반경
	const float SafeRadius = FMath::Clamp(SafeRadiusCm, 0.f, BaseRadius);

	// Handle = Base - Safe 
	float Handle = (BaseRadius - SafeRadius) * 0.5f;
	Handle = FMath::Clamp(Handle, 0.f, BaseRadius);

	float RadiusForNiagara = BaseRadius - Handle;
	RadiusForNiagara = FMath::Clamp(RadiusForNiagara, 0.f, BaseRadius);

	if (FogMID)
	{
		// 머티리얼 그래프가 월드 좌표 기반이면 Center도 월드로 그대로 넣는 게 맞음
		FogMID->SetVectorParameterValue(MatParamCenterPos, FLinearColor(Center.X, Center.Y, Center.Z, 0.f));
		FogMID->SetScalarParameterValue(MatParamRadius, SafeRadius);
	}

	// 파라미터 적용
	FogNiagaraComp->SetVariableFloat(ParamRadius, RadiusForNiagara);
	FogNiagaraComp->SetVariableFloat(ParamHandleRadius, Handle);

	// Phase 기반 SpawnRate
	if (bUsePhaseSpawnRate)
	{
		const float Rate = SpawnRateStart + SpawnRateStepPerPhase * float(FMath::Max(PhaseIndex - 1, 0));
		FogNiagaraComp->SetVariableFloat(ParamSpawnRate, Rate);
	} 

	if (BackdropNiagaraComp && CachedOuterRadiusCm > 0.f)
	{
		const float Inner = RadiusForNiagara;
		const float Outer = FMath::Max(CachedOuterRadiusCm, Inner);

		float BackdropHandle = (Outer - Inner) * 0.5f;
		BackdropHandle = FMath::Clamp(BackdropHandle, 0.f, Outer);

		float BackdropRadius = Outer - BackdropHandle;
		BackdropRadius = FMath::Clamp(BackdropRadius, 0.f, Outer);

		BackdropNiagaraComp->SetVariableFloat(ParamRadius, BackdropRadius);
		BackdropNiagaraComp->SetVariableFloat(ParamHandleRadius, BackdropHandle);

		BackdropNiagaraComp->SetVariableFloat(ParamSpawnRate, FMath::Max(0.f, BackdropSpawnRate));
	}
}