// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZAimWidget.generated.h"

/**
 * 
 */

UCLASS()
class ARENA_LASTGATE_API UOZAimWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairsCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairsLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairsRight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairsTop;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairsBottom;	

	void NativeConstruct() override;

private:
	// 크로스헤어가 최종적으로 얼만큼 벌어져서 그려져야하는지
	float CrosshairSpread = 0;
	// 크로스헤어 색상값
	FColor CrosshairColor = FColor::White;

	TObjectPtr<class UMaterialInstanceDynamic> HeatMaterial;
	TObjectPtr<class UMaterialInstanceDynamic> StaminaMaterial;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_HeatGage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_StaminaGage;

	

	/*UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_CrosshiarCenter;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_CrosshairBottom;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_CrosshairLeft;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_CrosshairRight;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_CrosshairTop;*/


public:
	FORCEINLINE void SetCrosshairSpread(float Spread) { CrosshairSpread = Spread; }
	FORCEINLINE void SetCrosshairColor(FColor Color) { CrosshairColor = Color; }

	UFUNCTION(BlueprintCallable)
	void SetHeatRatio(float ratio);

	UFUNCTION(BlueprintCallable)
	void SetStaminaRatio(float ratio);

	UPROPERTY(BlueprintReadWrite)
	float AimWidgetCenterOffset = -1.0f;

	UFUNCTION(BlueprintCallable)
	class UImage* GetImage_HeatGage() { return Image_HeatGage.Get(); }

	UFUNCTION(BlueprintImplementableEvent)
	void Calck_CenterOffset();

	UFUNCTION(BlueprintCallable)
	void Call_CalckCenterOffset();
};
