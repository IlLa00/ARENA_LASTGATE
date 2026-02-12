// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZFollowMinimap.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZFollowMinimap : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetRefFromMinimap(TObjectPtr<UTextureRenderTarget2D> minimapVisionMaskRenderTarget, TObjectPtr<class UOZMinimapWidget> minimapRef);

	UPROPERTY(BlueprintReadWrite, Category = "OZ|Minimap")
	FVector2D FollowMinimapSize = FVector2D(132.0f, 132.0f);

	UPROPERTY(BlueprintReadWrite, Category = "OZ|Minimap")
	FVector2D FollowMinimapReduceRatio = FVector2D(0.33f, 0.33f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "OZ|Minimap")
	FVector4 FollowMinimapNoneVisibleAreaMult = FVector4(0.3f, 0.3f, 0.3f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "OZ|Minimap")
	TSubclassOf<class UOZMinimapIcon> MinimapIconWidgetClass;

	UFUNCTION(BlueprintImplementableEvent)
	void SetFollowMinimapMaterial(UMaterialInstanceDynamic* minimapMaterialInstance);

	void UpdateFollowMinimap();

	void AwakeFollowMinimap();

	UPROPERTY(BlueprintReadWrite)
	bool bISVisible = false;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|MinimapIconReduceRatio")
	float ReduceRatio = 0.3;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> FollowMinimapImage;

	UPROPERTY()
	FVector2D Origin = FVector2D(120.0f, 123.0f);

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
	TObjectPtr<UMaterial> FollowMinimapMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FollowMinimapMaskInstance;

private:
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> MinimapVisionMaskRenderTarget = nullptr;

	UPROPERTY()
	TObjectPtr<class UOZMinimapWidget> MinimapReference = nullptr;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> IconDisplayedPanel;

	

	UPROPERTY()
	TArray<TObjectPtr<class UOZMinimapIcon>> IconPool;
};
