#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "OZVisionComponent.generated.h"

class UNiagaraComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARENA_LASTGATE_API UOZVisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOZVisionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnAiming();
	void OffAiming();
	const TArray<FVector>& GetVisionResults() const;
	void UpdateVisionParameters();

	void ClearVisionRenderTarget();

	void DisableVisionAndShowAllPlayers();

	void EnableVisionSystem();

protected:
	void LosVisionSystem();
	void CreateCone();
	void PrePareTriangles();
	void UpdateMinimapPlayerVisionObject(const TSet<class AActor*>& MinimapVisibleObjects);
	void UpdateMinimapVisibility();
	void UpdateBushHiddenSet();
	void UpdateWidgetVisibility(const TSet<class AOZPlayer*>& VisiblePlayers);
	void UpdateSmokeAndBushVisibility(const TSet<class AOZPlayer*>& VisiblePlayers);
	void UpdateShieldEffectVisibility(class AOZPlayer* Player, bool bVisible);

	void DrawTowerVisionAreas();
	void UpdateTowerWidgetVisibility(const TSet<class AOZPlayer*>& TowerVisiblePlayers);

	void SetupVisionTextureForRemotePlayers();

	// 액터 캐시 관련
	void RefreshActorCaches();
	void RefreshPlayerCache();
	void RefreshSmokeCache();
	void RefreshTowerVisionCache();

	void SetDegreePerTrace(float NewDegree);
	void SetTraceRange(float NewRange);

	// Vision 초기화 함수 (BeginPlay 또는 지연 초기화에서 호출)
	void InitializeVisionSystem();

	void RecalculateNumTraces();
	int32 CalculateNumTracesFromAngle(float Angle, float DegreeStep) const;
	void TryInitializeViewingAngle();

public:
	UPROPERTY(EditAnywhere, Category = "Vision")
	bool IsActivate = true;

	UPROPERTY(EditAnywhere, Category = "Vision")
	float TraceRange = 2200.f;

	UPROPERTY(EditAnywhere, Category = "Vision")
	float BaseTraceRange = 2200.f;

	UPROPERTY(EditAnywhere, Category = "Vision")
	class UMaterial* Parent;

	UPROPERTY(EditAnywhere, Category = "Vision")
	class UMaterialParameterCollection* TraceCollection;

	UPROPERTY(EditAnywhere, Category = "Vision")
	class UMaterialParameterCollection* PlayerCollection;

	UPROPERTY(EditAnywhere, Category = "Vision")
	class UTextureRenderTarget2D* DynamicRenderTarget;

	UPROPERTY(EditAnywhere, Category = "Vision")
	int32 NumTraces;

	UPROPERTY(EditAnywhere, Category = "Vision")
	float DegreePerTrace;

	class UMaterialInstanceDynamic* VisionMaterialInstance;

protected:
	// PostProcessVolume 캐싱 (재생성 시에도 올바른 Volume 참조 보장)
	UPROPERTY()
	class APostProcessVolume* CachedPostProcessVolume;

	// Vision System이 초기화되었는지 여부
	bool bIsVisionInitialized = false;

	float BaseDegreePerTrace = 1.0f;
	float AimingDegreePerTrace = 0.5f;
	float CachedViewingAngle = 60.0f;
	bool bIsAiming = false;
	bool bViewingAngleInitialized = false;

	// 부쉬 시야 감소
	bool bOwnerInBush = false;
	float BushTraceRangeRatio = 0.8f;

public:
	bool bFlashbanged = false;

	// TraceRange 일괄 재계산 (에이밍/부쉬 상태 변경 시 호출)
	void RecalculateTraceRange();

	TArray<FCanvasUVTri> CanvasTriangles;
	TArray<UMaterialInstanceDynamic*> CharacterMeterialInstances;
	TArray<UMaterialInstanceDynamic*> WeaponMeterialInstances;
	TArray<FVector> TraceResults;

	TSet<AOZPlayer*> PreviousDetectedPlayers;
	TSet<AOZPlayer*> PreviousTowerDetectedPlayers;
	TSet<AOZPlayer*> InitializedRemotePlayers;
	TSet<AOZPlayer*> BushHiddenPlayers;

	// 액터 캐시 (GetAllActorsOfClass 최적화)
	UPROPERTY()
	TArray<class AOZPlayer*> CachedPlayers;

	UPROPERTY()
	TArray<class AOZSmokeArea*> CachedSmokeAreas;

	UPROPERTY()
	TArray<class AOZTowerVisionArea*> CachedTowerVisionAreas;

	// 캐시 갱신 타이머
	float CacheRefreshTimer = 0.f;
	float CacheRefreshInterval = 0.5f;  // 0.5초마다 캐시 갱신

private:
	TSet<AActor*> PlayerVisionSet;
	TSet<AActor*> TowerVisionSet;

	TSet<AActor*> HideObjectSet;
};
