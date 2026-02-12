// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZMinimapIcon.h"
#include "Components/Image.h"

void UOZMinimapIcon::SetIconTexture(UTexture2D* NewTexture, FLinearColor customColor)
{
	if (!Image_icon)
		return;

	if (NewTexture)
	{
		Image_icon->SetBrushFromTexture(NewTexture);
	}
	else if (DefaultIconTexture)
	{
		Image_icon->SetBrushFromTexture(DefaultIconTexture);
	}

	Image_icon->SetColorAndOpacity(customColor);
	
}

void UOZMinimapIcon::ResetIcon()
{
	if (Image_icon && DefaultIconTexture)
	{
		Image_icon->SetBrushFromTexture(DefaultIconTexture);
	}
}