// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Object/OZWorldBoundCacher.h"
#include "Widget/OZMinimapIcon.h"
#include "Character/OZPlayerController.h"
#include "OZMinimapWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class ARENA_LASTGATE_API UOZMinimapWidget : public UUserWidget
{
	GENERATED_BODY()

//Utils
public:
	FVector2D GetPlayerMinimapPos() const;

protected:
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;

private:

	FVector WorldCenter;
	FVector MapSize;

	FVector2D WorldToMinimapRatio;

	TObjectPtr<AActor> PlayerActor;
	const TArray<FVector>* VisionTraceResults;

	UPROPERTY()
	TObjectPtr<AOZPlayer> OZPlayer;

	bool bCanUpdateMinimap = true;

	TObjectPtr<class AOZInGameGameState> ozInGameState = nullptr;

	UPROPERTY()
	TArray<TObjectPtr<UOZMinimapIcon>> IconPool;

	TMap<TObjectPtr<AActor>, TObjectPtr<UOZMinimapIcon>> ActorIconMap;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> IconDisplayedPanel;

	UPROPERTY()
	TObjectPtr<UOZMinimapIcon> minimapIcon;

	UOZMinimapIcon* GetMinimapIcon();
	void ReturnMinimapIcon(UOZMinimapIcon* icon);

	void SetControlledPlayerMinimapIcon();

	//void PrePareMinimapVisionTriangles(TArray<FSlateVertex>& OutVertices, TArray<SlateIndex>& OutIndices);

	void DrawMakedVisionMinimap();

	void DrawVisionMaskRenderTarget(TObjectPtr<UCanvas> canvas);
	void EraseMinimap();
	void GetVisionCompResultData();

	void UpdateMinimapVision();
	void UpdateMinimapBlackSmoge(float InDeltaTime);

	FVector2D WorldToMinimap(const FVector& WorldPos) const;

	/////////////////////////////////////////
	//Prev(현재 기준 안전구역)
	FVector WorldFogPrevCenter = FVector::ZeroVector;
	float WorldFogPrevRadiusCm = 0.f;

	// Next(목표 안전구역)
	FVector WorldFogNextCenter = FVector::ZeroVector;
	float WorldFogNextRadiusCm = 0.f;
	///////////////////////////////////////////

	FLinearColor iconColor;

	void Debug_DrawMnimimapBlackSmogeCenter(FVector2D Minimap_FogPrevCenter, FVector2D Minimap_FogNextCenter);

public:
	//void SetDisplayedActors(TSet<AActor*> currViewActors) { DisplayedActors = currViewActors; }

	UPROPERTY(EditDefaultsOnly, Category = "OZ|Minimap")
	TObjectPtr<UTextureRenderTarget2D> MinimapSizeRef;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|Minimap")
	TSubclassOf<UOZMinimapIcon> MinimapIconWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category = "OZ|MinimapSize")
	FVector2D minimapSize = FVector2D(396.0f, 396.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Minimap")
	FVector4 MinimapNoneVisibleAreaMult = FVector4(0.3f, 0.3f, 0.3f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UMaterial> MinimapMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MinimapMaskInstance;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> MinimapVisionMaskRenderTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|MinimapIconReduceRatio")
	FVector2D iconRatio = FVector2D(0.5f, 0.5f);

	/*UFUNCTION(BlueprintImplementableEvent)
	void CallInitMinimap();*/

	UFUNCTION(BlueprintImplementableEvent)
	void SetMinimapMaterial(UMaterialInstanceDynamic* minimapMaterialInstance);

	void SetWorldPresetParmas(FVector worldCenter, FVector mapSize, float fogInitSize, TWeakObjectPtr<class UOZFollowMinimap> followMinimapRef);
	void SetMinimapTowerVisionArea(TArray<TPair<FVector, float>> towerVisions);

	void UpdateMinimapIcons(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor);

	void ActivateTowerVision();
	void DeActivateTowerVision();

	void OnPlayerDie();

	void PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers);

public:

	void SetVisibleSurvivalIconInfo(bool bIsVisible);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_NumSurvival;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_SurvivalIcon;

	UFUNCTION(BlueprintCallable)
	void SetSurvivalPlayersNum(int survivalPlayers);

	void SetShrinking(float shrinkingTime);

	FVector4 GetMinimap_FogPrevCenterParam4() { return VectorParamSender(Minimap_FogPrevCenter); }
	FVector4 GetMinimap_FogNextCenterParam4() { return VectorParamSender(Minimap_FogNextCenter); }
	float GetMinimap_FogPrevRadius() { return Minimap_FogPrevRadius; }
	float GetMinimap_FogNextRadius() { return Minimap_FogNextRadius; }

private:
	void InitMinimap();

	//Minimap BlackSmogeParam
	const float MinimapFullSizeRadius = 0.65;
	double FogToMinimapRatio = 0.01;

	FVector2D Minimap_FogPrevCenter;
	FVector2D Minimap_FogNextCenter;

	float Minimap_FogPrevRadius;
	float Minimap_FogNextRadius;

	//FVector4 BlackSmogeMaterialParamSender = FVector4(0.0f, 0.0f, 0.0f, 1.0f);

	FVector4 VectorParamSender(const FVector2D& param) const;

	bool  bIsShrinking = false;
	float ShrinkStartTime = 0.f;
	float ShrinkDuration = 0.f;

	float     CachedPrevRadius = 0.f;
	FVector2D CachedPrevCenter;

	FVector2D BlackSmogeToMinimapUV(FVector& worldPos);

private:
	//Follow Minimap 
	UPROPERTY()
	TWeakObjectPtr<class UOZFollowMinimap> FollowMinimap = nullptr;

	//Tower
	TArray<TPair<FVector2D, float>> MinimapTowerVisions;

	bool bIsActivateTower = false;
};
