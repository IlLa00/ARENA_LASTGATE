// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/SLoadingScreen.h"
#include "SlateOptMacros.h"
#include "SlateExtras.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Math/UnrealMathUtility.h"
#include "Rendering/SlateRenderTransform.h"
#include "Math/TransformCalculus2D.h"
#include "Misc/Optional.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLoadingScreen::Construct(const FArguments& InArgs)
{
	//초기화
	BackGroundTextures = InArgs._BackGroundTextures;
	HintTextures = InArgs._HintTextures;

	LogoTexture = InArgs._LogoTexture;
	TrobberTexture = InArgs._TrobberTexture;
	CoreTexture = InArgs._CoreTexture;


	BackGroundBrush = MakeShareable(new FSlateBrush());

	LogoBrush = MakeShareable(new FSlateBrush());
	LogoBrush->SetResourceObject(LogoTexture);

	TrobberBrush = MakeShareable(new FSlateBrush());
	TrobberBrush->SetResourceObject(TrobberTexture);

	CoreBrush = MakeShareable(new FSlateBrush());
	CoreBrush->SetResourceObject(CoreTexture);

	RegisterActiveTimer(0.f,FWidgetActiveTimerDelegate::CreateSP(this, &SLoadingScreen::UpdateLoading));


	ChildSlot
		[
			SNew(SConstraintCanvas)

				//전체 배경 이미지
				+ SConstraintCanvas::Slot()
				.Offset(FMargin(0.f, 0.f, 0.f, 0.f))
				.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
				[
					SNew(SImage)
						//.Image(BackGroundBrush.Get())
						.Image(this, &SLoadingScreen::GetBackgroundBrush)
				]


				//좌상단 LastGate 이미지
				+SConstraintCanvas::Slot()
				.Offset(FMargin(57.f, 64.f, 387.f, 245.f))
				.Anchors(FAnchors(0.f, 0.f))
				.Alignment(FVector2D(0.f, 0.f))
				[
					SNew(SImage)
						.Image(LogoBrush.Get())
				]

				//하단 힌트 텍스트
				+ SConstraintCanvas::Slot()
				.Offset(FMargin(0.f, -80.f, -100.f, 60.f))
				.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
				.Alignment(FVector2D(0.5f, 1.f))
				[
					SNew(STextBlock)
						.Text(this, &SLoadingScreen::GetHintText)
						.Font(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 32))
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
				]


				+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(1.f, 1.f))
				.Alignment(FVector2D(1.f, 1.f))
				.Offset(FMargin(-67.0f, -39.0f, 172.8f, 172.8f))
				[
					SNew(SOverlay)

						//트로버
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(this, &SLoadingScreen::GetTrobberBrush)
								.RenderTransform(this, &SLoadingScreen::GetThrobberTransform)
								.RenderTransformPivot(FVector2D(0.5f, 0.5f))
						]

						//코어
						+ SOverlay::Slot()
						[
							SNew(SImage)
								.Image(CoreBrush.Get())
						]
				]
		];


	SelectBackGroundInfo();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

EActiveTimerReturnType SLoadingScreen::UpdateLoading(double InCurrentTime, float InDeltaTime)
{
	//초당 회전
	Throbber_Angle += InDeltaTime * SecondPerRotate;
	//Throbber_Angle += InDeltaTime;

	if (Throbber_Angle > 360.f)
		Throbber_Angle -= 360.f;

	ElapsedTime += InDeltaTime;

	if (ElapsedTime > ChangeTime)
	{
		ElapsedTime = 0.0f;
		SelectBackGroundInfo();
	}


	return EActiveTimerReturnType::Continue;
}

void SLoadingScreen::SelectBackGroundInfo()
{
	int randIndex = 0;
	for (int i = 0; i < 10; i++)
	{
		randIndex = FMath::RandRange(0, BackGroundTextures.Num() - 1);

		if (randIndex != pastSellected)
			break;
	}
	
	pastSellected = randIndex;

	BackGroundBrush->SetResourceObject(BackGroundTextures[pastSellected]);
	CurrentHintText = FText::FromString(HintTextures[pastSellected]);
}

const FSlateBrush* SLoadingScreen::GetBackgroundBrush() const
{
	return BackGroundBrush.Get();
}

const FSlateBrush* SLoadingScreen::GetTrobberBrush() const
{
	return TrobberBrush.Get();
}

FText SLoadingScreen::GetHintText() const
{
	return CurrentHintText;
}

TOptional<FSlateRenderTransform> SLoadingScreen::GetThrobberTransform() const
{
	const float AngleRadians = FMath::DegreesToRadians(Throbber_Angle);

	FQuat2D RotationQuat(AngleRadians);

	FSlateRenderTransform Transform(RotationQuat);

	return Transform;
}