#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZInteractionUI.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZInteractionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetTextVisibility(AActor* InActor);
	void SetGaugePercent(float Percent);

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Txt_Mini;

	// 배경 이미지 (항상 보임)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_GaugeA;

	// 퍼센트에 따라 밑에서 위로 차오르는 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Img_GaugeB;

	// B 이미지용 머티리얼 (에디터에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gauge")
	TObjectPtr<class UMaterialInterface> GaugeMaterial;

private:
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> DynamicGaugeMaterial;
};
