// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZFollowMinimap.h"
#include "Widget/OZMinimapWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Canvas.h"
#include "Widget/OZMinimapIcon.h"
#include "Components/Image.h"
#include "Kismet/KismetRenderingLibrary.h"

void UOZFollowMinimap::NativeConstruct()
{
	Super::NativeConstruct();

	const int32 PoolSize = 20;

	for (int32 i = 0; i < PoolSize; ++i)
	{
		UOZMinimapIcon* Icon = CreateWidget<UOZMinimapIcon>(GetWorld(), MinimapIconWidgetClass);
		FVector2D iconSize = Icon->IconSize * ReduceRatio;

		if (UCanvasPanelSlot* iconSlot = IconDisplayedPanel->AddChildToCanvas(Icon))
		{
			iconSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			iconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			iconSlot->SetSize(FVector2D(iconSize.X, iconSize.Y));
			iconSlot->SetPosition(FVector2D::ZeroVector);
		}

		Icon->SetVisibility(ESlateVisibility::Collapsed);
		IconPool.Add(Icon);
	}

	if (FollowMinimapMaterial)
	{
		FollowMinimapMaskInstance = UMaterialInstanceDynamic::Create(FollowMinimapMaterial, this);
		MinimapVisionMaskRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, FollowMinimapSize.X, FollowMinimapSize.Y, ETextureRenderTargetFormat::RTF_RGBA8);

		//FollowMinimapMaskInstance->SetTextureParameterValue("VisionMask", MinimapVisionMaskRenderTarget);
		FollowMinimapMaskInstance->SetVectorParameterValue("NoneVisionColorMultiply", FollowMinimapNoneVisibleAreaMult);
		FollowMinimapMaskInstance->SetVectorParameterValue("FollowMinimapReduceRatio", FVector4(FollowMinimapReduceRatio.X, FollowMinimapReduceRatio.Y));
		

		//SetFollowMinimapMaterial(FollowMinimapMaskInstance);
	}
}

void UOZFollowMinimap::SetRefFromMinimap(TObjectPtr<UTextureRenderTarget2D> minimapVisionMaskRenderTarget, TObjectPtr<class UOZMinimapWidget> minimapRef)
{
	if (minimapVisionMaskRenderTarget == nullptr || minimapRef == nullptr)
		return;

	MinimapReference = minimapRef;
	MinimapVisionMaskRenderTarget = minimapVisionMaskRenderTarget;

	FollowMinimapMaskInstance->SetTextureParameterValue("VisionMask", MinimapVisionMaskRenderTarget);
	SetFollowMinimapMaterial(FollowMinimapMaskInstance);
}

void UOZFollowMinimap::UpdateFollowMinimap()
{
	if (bISVisible == false)
		return;

	FVector2D Center = MinimapReference->GetPlayerMinimapPos();
	FVector2D normalizedPlayerPos = Center / MinimapReference->minimapSize;

	FollowMinimapMaskInstance->SetVectorParameterValue("PlayerPos", FVector4(normalizedPlayerPos.X, normalizedPlayerPos.Y, 0, 0));

	//마스크 영역 이동
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(FollowMinimapImage->Slot))
	{
		// 정규화된 좌표를 이미지 크기 기준으로 변환 (픽셀)
		FVector2D CenterOffsetRatio = normalizedPlayerPos - FVector2D(0.5f, 0.5f);

		// 원점 위치에 오프셋 더해서 새로운 위치 계산
		FVector2D UpdatedPosition = Origin - CenterOffsetRatio * MinimapReference->minimapSize;

		CanvasSlot->SetPosition(UpdatedPosition);
	}

	
	FollowMinimapMaskInstance->SetVectorParameterValue("SafeZoneCenter", MinimapReference->GetMinimap_FogPrevCenterParam4());
	FollowMinimapMaskInstance->SetScalarParameterValue("SafeZoneRadius", MinimapReference->GetMinimap_FogPrevRadius());

	FollowMinimapMaskInstance->SetVectorParameterValue("NextSafeZoneCenter", MinimapReference->GetMinimap_FogNextCenterParam4());
	FollowMinimapMaskInstance->SetScalarParameterValue("NextSafeZoneRadius", MinimapReference->GetMinimap_FogNextRadius());
}

void UOZFollowMinimap::AwakeFollowMinimap()
{
	//ForceLayoutPrepass();
	InvalidateLayoutAndVolatility();

	if (FollowMinimapImage && FollowMinimapMaskInstance)
	{
		SetFollowMinimapMaterial(FollowMinimapMaskInstance);
	}

	UpdateFollowMinimap();
}
