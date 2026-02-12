// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZWorldBoundCacher.generated.h"

class UBoxComponent;
class UBillboardComponent;

USTRUCT(BlueprintType)
struct FOZWorldBoundInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Center = FVector::ZeroVector;

	UPROPERTY()
	FVector Size = FVector::ZeroVector;

	bool IsValid() const
	{
		return !Size.IsZero();
	}
};

UCLASS()
class ARENA_LASTGATE_API AOZWorldBoundCacher : public AActor
{
	GENERATED_BODY()
	
public:	
	AOZWorldBoundCacher();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	//싱글톤에서 여럿이서 동일한 사진을 쓰려면 Texture을 만들어서 사용해야 함
	void CreateMinimapTextureAsset();

	void UpdateTextureFromRenderTarget(UTexture2D* Texture, UTextureRenderTarget2D* RenderTarget);

public:

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	TObjectPtr<UBoxComponent> AreaBounds;

	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	TObjectPtr<UBillboardComponent> Billboard;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USceneCaptureComponent2D> SceneCaptureComponent2D;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTextureRenderTarget2D> MinimapRenderTargetTexture = nullptr;

	/** BoxComponent의 Scaled Extent 캐시 값 */
	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	FVector CachedBounds;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	bool bChangeDirectionLight = false;

public:
	UFUNCTION(CallInEditor, BlueprintCallable)
	void TakeAShot();
};
