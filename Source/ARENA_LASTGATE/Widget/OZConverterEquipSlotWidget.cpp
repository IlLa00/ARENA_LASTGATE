// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZConverterEquipSlotWidget.h"
#include "Components/Image.h"

void UOZConverterEquipSlotWidget::SetSlotDefault()
{
	assingedConvertID = -1;

	TierEffectImage->SetVisibility(ESlateVisibility::Hidden);
	ConverterIconImage->SetVisibility(ESlateVisibility::Hidden);
}

void UOZConverterEquipSlotWidget::SetEquipSlotConverter(FOZConvertData assingedConverter)
{
	if (assingedConverter.ID <= 0)
	{
		SetSlotDefault();
		return;
	}
		

	if (TierEffectImage == nullptr || ConverterIconImage == nullptr)
		return;

	EConvertGrade targetGrade = assingedConverter.Grade;
	UTexture2D* tierEffectImage;

	switch (targetGrade)
	{
	case EConvertGrade::None:
		return;
	case EConvertGrade::Rare:
		tierEffectImage = EffectImage_Rare;
		break;
	case EConvertGrade::Unique:
		tierEffectImage = EffectImage_Unique;
		break;
	case EConvertGrade::Legendary:
		tierEffectImage = EffectImage_Legendaray;
		break;
	default:
		return;
	}

	TierEffectImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	ConverterIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	TierEffectImage->SetBrushFromTexture(tierEffectImage);
	ConverterIconImage->SetBrushFromTexture(assingedConverter.Icon.Get());

	assingedConvertData = assingedConverter;
	assingedConvertID = assingedConverter.ID;
}

void UOZConverterEquipSlotWidget::BroadCastSelected()
{
	if (assingedConvertID == -1)
		return;

	OnEquipSlotSelected.Broadcast(assingedConvertData, assingedEquipIndex);
}
