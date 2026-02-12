#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "OZBuffProgressEntry.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;

DECLARE_DELEGATE_OneParam(FOnBuffExpired, class UOZBuffProgressEntry*);

UCLASS()
class ARENA_LASTGATE_API UOZBuffProgressEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitBuff(UTexture2D* Icon, float InTotalDuration, FGameplayTag InBuffTag, const FText& BuffName = FText::GetEmpty());

	FGameplayTag GetBuffTag() const { return BuffTag; }
	float GetTotalDuration() const { return TotalDuration; }

	FOnBuffExpired OnBuffExpired;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_BuffIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RemainingTime;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_BuffName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_ProgressFill;

	UPROPERTY(EditDefaultsOnly, Category = "BuffUI|Bar")
	TObjectPtr<UMaterialInterface> ProgressFillMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressFillMID;

	UPROPERTY(EditDefaultsOnly, Category = "BuffUI|Bar")
	TObjectPtr<UTexture2D> FillTex_TypeA;

	UPROPERTY(EditDefaultsOnly, Category = "BuffUI|Bar")
	TObjectPtr<UTexture2D> FillTex_TypeB;

	UPROPERTY(EditDefaultsOnly, Category = "BuffUI|Bar")
	TMap<FGameplayTag, TObjectPtr<UTexture2D>> FillTexByBuffTag;

	UPROPERTY(EditDefaultsOnly, Category = "BuffUI|Bar")
	TObjectPtr<UTexture2D> DefaultFillTex;

private:
	void UpdateProgressUI();

	float TotalDuration = 0.f;
	float RemainingTime = 0.f;
	FGameplayTag BuffTag;
};
