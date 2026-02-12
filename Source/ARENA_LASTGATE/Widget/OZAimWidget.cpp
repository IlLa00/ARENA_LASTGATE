// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZAimWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Image.h"

void UOZAimWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Image_HeatGage)
	{
		HeatMaterial = Image_HeatGage->GetDynamicMaterial();
	}

	if (Image_StaminaGage)
	{
		StaminaMaterial = Image_StaminaGage->GetDynamicMaterial();
	}

}

void UOZAimWidget::SetHeatRatio(float ratio)
{
	if (HeatMaterial == nullptr)
		return;

	HeatMaterial->SetScalarParameterValue(TEXT("Ratio"), ratio);
}

void UOZAimWidget::SetStaminaRatio(float ratio)
{
	if (StaminaMaterial == nullptr)
		return;

	StaminaMaterial->SetScalarParameterValue(TEXT("Ratio"), ratio);
}

void UOZAimWidget::Call_CalckCenterOffset()
{
	Calck_CenterOffset();
}
