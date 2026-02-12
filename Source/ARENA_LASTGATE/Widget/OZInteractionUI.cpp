#include "Widget/OZInteractionUI.h"
#include "Interface/OZInteractable.h"
#include "Object/OZTower.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UOZInteractionUI::NativeConstruct()
{
	SetVisibility(ESlateVisibility::Hidden);

	// Dynamic Material Instance 생성 및 B 이미지에 적용
	if (Img_GaugeB && GaugeMaterial)
	{
		DynamicGaugeMaterial = UMaterialInstanceDynamic::Create(GaugeMaterial, this);
		Img_GaugeB->SetBrushFromMaterial(DynamicGaugeMaterial);
		DynamicGaugeMaterial->SetScalarParameterValue(FName("Percent"), 0.f);
	}
}

void UOZInteractionUI::SetTextVisibility(AActor* InActor)
{
	if (!InActor)
		return;

	if (!InActor->Implements<UOZInteractable>())
		return;
}

void UOZInteractionUI::SetGaugePercent(float Percent)
{
	if (DynamicGaugeMaterial)
	{
		DynamicGaugeMaterial->SetScalarParameterValue(FName("Percent"), FMath::Clamp(Percent, 0.f, 1.f));
	}
}
