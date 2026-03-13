// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class ARENA_LASTGATE_API SLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreen) : _LogoTexture(nullptr), _TrobberTexture(nullptr), _CoreTexture(nullptr){}
		SLATE_ARGUMENT(TArray<UTexture2D*>, BackGroundTextures)
		SLATE_ARGUMENT(TArray<FString>, HintTextures)
		SLATE_ARGUMENT(UTexture2D*, LogoTexture)
		SLATE_ARGUMENT(UTexture2D*, TrobberTexture)
		SLATE_ARGUMENT(UTexture2D*, CoreTexture)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	
	TArray<UTexture2D*> BackGroundTextures;
	TArray<FString> HintTextures;
	UTexture2D* LogoTexture;
	UTexture2D* TrobberTexture;
	UTexture2D* CoreTexture;

	const FSlateBrush* GetBackgroundBrush() const;
	const FSlateBrush* GetTrobberBrush() const;
	TOptional<FSlateRenderTransform> GetThrobberTransform() const;
	FText GetHintText() const;

	TSharedPtr<FSlateBrush> BackGroundBrush;
	TSharedPtr<FSlateBrush> LogoBrush;
	TSharedPtr<FSlateBrush> TrobberBrush;
	TSharedPtr<FSlateBrush> CoreBrush;
	FText CurrentHintText;

	EActiveTimerReturnType UpdateLoading(double InCurrentTime, float InDeltaTime);

	float Throbber_Angle = 0.0f;
	int SecondPerRotate = 90.f;

	float ElapsedTime = 0.0f;

	float ChangeTime = 3.0f;
	
	void SelectBackGroundInfo();

	int sellectedIndex = -1;
	int pastSellected = 0;

};
