// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZMinimapWidget.h"
#include "Engine/Texture2D.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Interface/OZIMinimapVisibleActor.h"
#include "Slate/SlateBrushAsset.h"
#include "Rendering/DrawElements.h"
#include "Character/OZPlayer.h"
#include "Character/Components/OZVisionComponent.h"
#include "Character/OZPlayerState.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "GameState/OZInGameGameState.h"
#include "Widget/OZFollowMinimap.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"

void UOZMinimapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitMinimap();

	if (MinimapMaterial)
	{
		MinimapMaskInstance = UMaterialInstanceDynamic::Create(MinimapMaterial, this);
		MinimapVisionMaskRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, minimapSize.X, minimapSize.Y, ETextureRenderTargetFormat::RTF_RGBA8);

		MinimapMaskInstance->SetTextureParameterValue("VisionMask", MinimapVisionMaskRenderTarget);
		MinimapMaskInstance->SetVectorParameterValue("NoneVisionColorMultiply", MinimapNoneVisibleAreaMult);

		SetMinimapMaterial(MinimapMaskInstance);
	}
}

void UOZMinimapWidget::InitMinimap()
{
	const int32 PoolSize = 20;

	if (MinimapSizeRef != nullptr)
	{
		minimapSize = FVector2D(MinimapSizeRef->SizeX, MinimapSizeRef->SizeY);
	}


	for (int32 i = 0; i < PoolSize; ++i)
	{
		UOZMinimapIcon* Icon = CreateWidget<UOZMinimapIcon>(GetWorld(), MinimapIconWidgetClass);
		FVector2D iconSize = Icon->IconSize * iconRatio;  
		//FVector2D iconSize = Icon->IconSize * FVector2D(0.5f, 0.5f);

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
}

FVector2D UOZMinimapWidget::GetPlayerMinimapPos() const
{
	if(PlayerActor == nullptr)
		return FVector2D(-1, -1);

	return WorldToMinimap(PlayerActor->GetActorLocation());
}

void UOZMinimapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateMinimapVision();
	UpdateMinimapBlackSmoge(InDeltaTime);

	FollowMinimap->UpdateFollowMinimap();
}

UOZMinimapIcon* UOZMinimapWidget::GetMinimapIcon()
{
	for (UOZMinimapIcon* Icon : IconPool)
	{
		if (!Icon)
			continue;

		if (Icon->GetVisibility() == ESlateVisibility::Collapsed)
		{
			Icon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			return Icon;
		}
	}

	return nullptr;
}

void UOZMinimapWidget::ReturnMinimapIcon(UOZMinimapIcon* icon)
{
	if (!icon)
		return;

	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(icon->Slot))
	{
		PanelSlot->SetPosition(FVector2D::ZeroVector);
	}

	icon->ResetIcon();
	icon->SetVisibility(ESlateVisibility::Collapsed);
}

void UOZMinimapWidget::SetWorldPresetParmas(FVector worldCenter, FVector mapSize, float fogInitSize, TWeakObjectPtr<class UOZFollowMinimap> followMinimapRef)
{
	WorldCenter = worldCenter;
	MapSize = mapSize;

	WorldToMinimapRatio.X = minimapSize.X * 0.5 / MapSize.X;
	WorldToMinimapRatio.Y = minimapSize.Y * 0.5 / MapSize.Y;

	// 검은안개 7100기준, 대략적인 미니맵 반지름 상수 - 머터리얼 디버그 기능 연결해서 업데이트 할 것
	FogToMinimapRatio = MinimapFullSizeRadius / fogInitSize;

	FollowMinimap = followMinimapRef;

	FollowMinimap->SetRefFromMinimap(MinimapVisionMaskRenderTarget, this);

}

void UOZMinimapWidget::SetMinimapTowerVisionArea(TArray<TPair<FVector, float>> towerVisions)
{
	MinimapTowerVisions.Empty();

	for (TPair<FVector, float>& towerVisionInfo : towerVisions)
	{
		FVector TowerWorldPosition = towerVisionInfo.Key;
		float TowerVisionWorldRadius = towerVisionInfo.Value;

		TPair<FVector2D, float> minimapVision = MakeTuple(WorldToMinimap(TowerWorldPosition), TowerVisionWorldRadius * WorldToMinimapRatio.X);

		MinimapTowerVisions.Add(minimapVision);
	}


}

void UOZMinimapWidget::UpdateMinimapIcons(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor)
{

	for (AActor* addedActor : AddedActor)
	{
		if (ActorIconMap.Contains(addedActor))
			continue;

		UOZMinimapIcon* assingedIcon = GetMinimapIcon();

		if (assingedIcon)
		{
			if (addedActor->GetClass()->ImplementsInterface(UOZIMinimapVisibleActor::StaticClass()))
			{
				UTexture2D* IconTexture = IOZIMinimapVisibleActor::Execute_GetMinimapIcon(addedActor);
				EMinimapActorCategory actorCategory = IOZIMinimapVisibleActor::Execute_GetMinimapCategory(addedActor);

				iconColor = FLinearColor::White;

				if (actorCategory == EMinimapActorCategory::Character)
					iconColor = FLinearColor::Red;
				
				assingedIcon->SetIconTexture(IconTexture, iconColor);

				if (!GetOwningPlayer()->HasAuthority())
				{
					AActor* myPawn = GetOwningPlayer()->GetPawn();
					TSet<AActor*> test = GetOwningPlayerState<AOZPlayerState>()->CurrentViewObjects;
					UE_LOG(LogTemp, Log, TEXT("MyPawn : %p, AddActor : %p"), myPawn, addedActor);
				}

				ActorIconMap.Add(addedActor, assingedIcon);
			}
		}
	}

	for (AActor* removedActor : RemovedActor)
	{
		if (!ActorIconMap.Contains(removedActor))
			continue;

		ActorIconMap[removedActor]->ResetIcon();
		ActorIconMap[removedActor]->SetVisibility(ESlateVisibility::Collapsed);
		ActorIconMap.Remove(removedActor);

		if (!GetOwningPlayer()->HasAuthority())
		{
			AActor* myPawn = GetOwningPlayer()->GetPawn();
			TSet<AActor*> test = GetOwningPlayerState<AOZPlayerState>()->CurrentViewObjects;
			UE_LOG(LogTemp, Log, TEXT("MyPawn : %p, RemoveActor : %p"), myPawn, removedActor);
		}
	}
}

void UOZMinimapWidget::PrepareMinimap(TObjectPtr<class AOZInGameGameState> ozIngameState, int32 numPlayers)
{
	this->ozInGameState = ozIngameState;

	this->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	EraseMinimap();
	SetControlledPlayerMinimapIcon();
	GetVisionCompResultData();

	if (PlayerActor)
	{
		bCanUpdateMinimap = true;
	}
	else
	{
		bCanUpdateMinimap = false;
	}

	SetVisibleSurvivalIconInfo(true);

	SetSurvivalPlayersNum(numPlayers);
}

void UOZMinimapWidget::SetControlledPlayerMinimapIcon()
{
	PlayerActor = GetOwningPlayer()->GetPawn();

	if (!PlayerActor)
		return;

	TSet<AActor*> test = GetOwningPlayerState<AOZPlayerState>()->CurrentViewObjects;
	GetOwningPlayerState<AOZPlayerState>()->CurrentViewObjects.Empty();

	UOZMinimapIcon* assingedIcon = GetMinimapIcon();

	if (assingedIcon)
	{
		if (PlayerActor->GetClass()->ImplementsInterface(UOZIMinimapVisibleActor::StaticClass()))
		{
			UTexture2D* IconTexture = IOZIMinimapVisibleActor::Execute_GetMinimapIcon(PlayerActor);

			assingedIcon->SetIconTexture(IconTexture, FLinearColor::Yellow);
			ActorIconMap.Add(PlayerActor, assingedIcon);

			if (!GetOwningPlayer()->HasAuthority())
			{
				UE_LOG(LogTemp, Log, TEXT("Init Add MyPawn : %p"), PlayerActor.Get());
			}
		}
	}
}

FVector2D UOZMinimapWidget::WorldToMinimap(const FVector& WorldPos) const
{
	FVector Delta = WorldPos - WorldCenter;

	float MapY = Delta.X * WorldToMinimapRatio.X * -1.f;
	float MapX = Delta.Y * WorldToMinimapRatio.Y;

	MapY += minimapSize.Y * 0.5f;
	MapX += minimapSize.X * 0.5f;

	return FVector2D(MapX, MapY);
}

void UOZMinimapWidget::DrawMakedVisionMinimap()
{
	if (!MinimapVisionMaskRenderTarget || !VisionTraceResults || VisionTraceResults->Num() < 2)
		return;

	UKismetRenderingLibrary::ClearRenderTarget2D(
		this,
		MinimapVisionMaskRenderTarget,
		FLinearColor(0, 0, 0, 0)
	);

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;

	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
		this,
		MinimapVisionMaskRenderTarget,
		Canvas,
		Size,
		Context
	);

	DrawVisionMaskRenderTarget(Canvas);

	if (bIsActivateTower)
	{
		for (const TPair<FVector2D, float>& Vision : MinimapTowerVisions)
		{
			const FVector2D& Center = Vision.Key;
			const float RadiusValue = Vision.Value;

			Canvas->K2_DrawPolygon(
				nullptr,               
				Center,                
				FVector2D(RadiusValue),
				32,                    
				FLinearColor::White    
			);
		}
	}

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(
		this,
		Context
	);
}

void UOZMinimapWidget::DrawVisionMaskRenderTarget(TObjectPtr<UCanvas> canvas)
{
	if (!PlayerActor)
		return;

	const FVector2D Center = WorldToMinimap(PlayerActor->GetActorLocation());

	TArray<FCanvasUVTri> Tris;

	if (VisionTraceResults->Num() != OZPlayer->VisionComp->NumTraces)
		return;

	for (int32 i = 0; i < VisionTraceResults->Num() - 1; ++i)
	{
		FVector2D P0 = WorldToMinimap((*VisionTraceResults)[i]);
		FVector2D P1 = WorldToMinimap((*VisionTraceResults)[i + 1]);

		FCanvasUVTri Tri;
		Tri.V0_Pos = Center;
		Tri.V1_Pos = P0;
		Tri.V2_Pos = P1;

		Tri.V0_Color = FLinearColor::White;
		Tri.V1_Color = FLinearColor::White;
		Tri.V2_Color = FLinearColor::White;

		Tris.Add(Tri);
	}

	canvas->K2_DrawTriangle(nullptr, Tris);
}

void UOZMinimapWidget::EraseMinimap()
{
	for (int i = 0; i < IconPool.Num(); i++)
	{
		UOZMinimapIcon* currMinimapIcon = IconPool[i];
		currMinimapIcon->ResetIcon();
		currMinimapIcon->SetVisibility(ESlateVisibility::Collapsed);
	}


	if (!GetOwningPlayer()->HasAuthority())
	{
		AActor* myPawn = GetOwningPlayer()->GetPawn();
		TSet<AActor*> test = GetOwningPlayerState<AOZPlayerState>()->CurrentViewObjects;
		UE_LOG(LogTemp, Log, TEXT("Reset All MyPawn : %p"), myPawn);
	}

	ActorIconMap.Empty();
}

void UOZMinimapWidget::GetVisionCompResultData()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (OZPlayer = Cast<AOZPlayer>(PC->GetPawn()))
		{
			if (OZPlayer->VisionComp)
			{
				VisionTraceResults = &OZPlayer->VisionComp->GetVisionResults();
			}
		}
	}
}

void UOZMinimapWidget::OnPlayerDie()
{
	EraseMinimap();

	{
		AActor* myPawn = GetOwningPlayer()->GetPawn();
		UE_LOG(LogTemp, Log, TEXT("MyPawn : %p PlayerDie"), myPawn);
	}

	UKismetRenderingLibrary::ClearRenderTarget2D(
		this,
		MinimapVisionMaskRenderTarget,
		FLinearColor::Black
	);

	bCanUpdateMinimap = false;

	//Text_NumSurvival->SetText(FText::FromString(FString::FromInt(0)));
	SetVisibleSurvivalIconInfo(false);

	this->SetVisibility(ESlateVisibility::Hidden);
}

void UOZMinimapWidget::SetSurvivalPlayersNum(int survivalPlayers)
{
	Text_NumSurvival->SetText(FText::FromString(FString::FromInt(survivalPlayers)));
}

void UOZMinimapWidget::SetVisibleSurvivalIconInfo(bool bIsVisible)
{
	ESlateVisibility visibility = ESlateVisibility::SelfHitTestInvisible;

	if (bIsVisible == false)
	{
		visibility = ESlateVisibility::Hidden;
	}

	Text_NumSurvival->SetVisibility(visibility);
	Image_SurvivalIcon->SetVisibility(visibility);
}

void UOZMinimapWidget::UpdateMinimapVision()
{
	for (auto& Pair : ActorIconMap)
	{
		AActor* Actor = Pair.Key;
		UOZMinimapIcon* Icon = Pair.Value;

		if (!IsValid(Actor) || !IsValid(Icon))
			continue;

		FVector ActorLocation = Actor->GetActorLocation();
		FVector Delta = ActorLocation - WorldCenter;

		float MapY = Delta.X * WorldToMinimapRatio.X * -1;
		float MapX = Delta.Y * WorldToMinimapRatio.Y;

		if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Icon->Slot))
		{
			PanelSlot->SetPosition(FVector2D(MapX, MapY));
		}
	}

	if (bCanUpdateMinimap)
		DrawMakedVisionMinimap();
}

void UOZMinimapWidget::SetShrinking(float shrinkingTime)
{

	ShrinkStartTime = GetOwningPlayer()->GetWorld()->GetTimeSeconds();
	ShrinkDuration = shrinkingTime;
	bIsShrinking = true;

	CachedPrevRadius = Minimap_FogPrevRadius;
	CachedPrevCenter = Minimap_FogPrevCenter;
}

void UOZMinimapWidget::UpdateMinimapBlackSmoge(float InDeltaTime)
{
	if (ozInGameState == nullptr || bCanUpdateMinimap == false)
		return;

	//중앙 - 0.5 0.5 
	//좌측 상단 - 0 0
	//우측 하단 / 1 1


	WorldFogPrevCenter = ozInGameState->RepFogPrevCenter;
	WorldFogPrevRadiusCm = ozInGameState->RepFogPrevRadiusCm;

	WorldFogNextCenter = ozInGameState->RepFogNextCenter;
	WorldFogNextRadiusCm = ozInGameState->RepFogNextRadiusCm;

	Minimap_FogPrevRadius = WorldFogPrevRadiusCm * FogToMinimapRatio;
	Minimap_FogNextRadius = WorldFogNextRadiusCm * FogToMinimapRatio;

	Minimap_FogPrevCenter = BlackSmogeToMinimapUV(WorldFogPrevCenter);
	Minimap_FogNextCenter = BlackSmogeToMinimapUV(WorldFogNextCenter);

	if (bIsShrinking)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float Alpha = (CurrentTime - ShrinkStartTime) / ShrinkDuration;
		Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

		Minimap_FogPrevRadius = FMath::Lerp(CachedPrevRadius, Minimap_FogNextRadius, Alpha);

		Minimap_FogPrevCenter = FMath::Lerp(CachedPrevCenter, Minimap_FogNextCenter, Alpha);

		if (Alpha >= 1.f)
		{
			bIsShrinking = false;

			Minimap_FogPrevRadius = Minimap_FogNextRadius;
			Minimap_FogPrevCenter = Minimap_FogNextCenter;
		}
	}

	MinimapMaskInstance->SetVectorParameterValue("SafeZoneCenter", VectorParamSender(Minimap_FogPrevCenter));
	MinimapMaskInstance->SetScalarParameterValue("SafeZoneRadius", Minimap_FogPrevRadius);

	MinimapMaskInstance->SetVectorParameterValue("NextSafeZoneCenter", VectorParamSender(Minimap_FogNextCenter));
	MinimapMaskInstance->SetScalarParameterValue("NextSafeZoneRadius", Minimap_FogNextRadius);

	//Debug_DrawMnimimapBlackSmogeCenter(Minimap_FogPrevCenter, Minimap_FogNextCenter);
}

void UOZMinimapWidget::Debug_DrawMnimimapBlackSmogeCenter(FVector2D Temp_Minimap_FogPrevCenter, FVector2D Temp_Minimap_FogNextCenter)
{
	MinimapMaskInstance->SetVectorParameterValue("DebugSafeZoneCenter", VectorParamSender(Temp_Minimap_FogPrevCenter));
	MinimapMaskInstance->SetVectorParameterValue("DebugNextSafeZoneCenter", VectorParamSender(Temp_Minimap_FogNextCenter));
}

FVector4 UOZMinimapWidget::VectorParamSender(const FVector2D& param) const
{
	/*BlackSmogeMaterialParamSender.X = param.X;
	BlackSmogeMaterialParamSender.Y = param.Y;*/

	return FVector4(param.X, param.Y, 0.f, 0.f);
}

FVector2D UOZMinimapWidget::BlackSmogeToMinimapUV(FVector& worldPos)
{
	FVector2D UV;

	UV.X = 0.5f + (worldPos.Y * FogToMinimapRatio);
	UV.Y = 0.5f - (worldPos.X * FogToMinimapRatio);

	return UV;
}

void UOZMinimapWidget::ActivateTowerVision()
{
	bIsActivateTower = true;
}

void UOZMinimapWidget::DeActivateTowerVision()
{
	bIsActivateTower = false;
}