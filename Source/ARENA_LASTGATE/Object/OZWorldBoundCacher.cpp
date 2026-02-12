// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZWorldBoundCacher.h"

#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "GameState/OZInGameGameState.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Kismet/KismetRenderingLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ImageUtils.h"

// Sets default values
AOZWorldBoundCacher::AOZWorldBoundCacher()
{
	AreaBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBounds"));
	SetRootComponent(AreaBounds);

	AreaBounds->InitBoxExtent(FVector(1024.f, 1024.f, 100.f));
	AreaBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);



	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
	Billboard->SetupAttachment(AreaBounds);

	Billboard->SetRelativeLocation(FVector(0.f, 0.f, 450.f));
	Billboard->SetUsingAbsoluteScale(true);
	Billboard->SetWorldScale3D(FVector(5.f, 5.f, 5.f));

	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapSceneCapture"));
	SceneCaptureComponent2D->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent2D->SetRelativeLocation(FVector::ZeroVector);
	SceneCaptureComponent2D->bCaptureEveryFrame = false;
	SceneCaptureComponent2D->bCaptureOnMovement = false;

	if (MinimapRenderTargetTexture == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Insert TextureTarget To WorldBoundCacher"));
	}

	SceneCaptureComponent2D->TextureTarget = MinimapRenderTargetTexture;

	//static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> MinimapRenterTargetObject(TEXT("/Game/Resources/Test_UIImage/MinimapTexture.MinimapTexture"));
	//if (MinimapRenterTargetObject.Object != nullptr)
	//{
	//	//SceneCaptureComponent2D->TextureTarget = MinimapRenderTargetTexture;
	//	SceneCaptureComponent2D->TextureTarget = MinimapRenterTargetObject.Object;
	//}



}

void AOZWorldBoundCacher::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (AreaBounds)
	{
		CachedBounds = AreaBounds->GetScaledBoxExtent();

		float OrthoWidth = FMath::Max(CachedBounds.X, CachedBounds.Y) * 2.f;
		SceneCaptureComponent2D->OrthoWidth = OrthoWidth;
	}
}

// Called when the game starts or when spawned
void AOZWorldBoundCacher::BeginPlay()
{
	Super::BeginPlay();



	if (!HasAuthority())
		return;

	if (AOZInGameGameState* ozIngameState = Cast<AOZInGameGameState>(GetWorld()->GetGameState()))
	{
		ozIngameState->SetWorldBounds(AreaBounds->GetComponentLocation(), CachedBounds);
	}

	//왜 인게임 엔진에서 light Actor를 직접 참조하면 에러가 나는거지??
	//	TakeAShot();
}

void AOZWorldBoundCacher::TakeAShot()
{

	{
		auto& PPS = SceneCaptureComponent2D->PostProcessSettings;

		/*PPS.bOverride_AutoExposureMethod = true;
		PPS.AutoExposureMethod = AEM_Manual;

		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMinBrightness = 1.f;
		PPS.AutoExposureMaxBrightness = 1.f;*/

		SceneCaptureComponent2D->ShowFlags.Fog = false;

		SceneCaptureComponent2D->PostProcessBlendWeight = 0.f;
	}

	TArray<AActor*> FoundLights;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundLights);

	TArray<AActor*> SmogActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("smog"), SmogActors);

	SceneCaptureComponent2D->HiddenActors.Reset();

	for (AActor* Actor : SmogActors)
	{
		if (IsValid(Actor))
		{
			SceneCaptureComponent2D->HiddenActors.AddUnique(Actor);
		}
	}


	ADirectionalLight* MainDirectionalLight = nullptr;
	if (FoundLights.Num() == 1)
	{
		MainDirectionalLight = Cast<ADirectionalLight>(FoundLights[0]);
	}

	if (bChangeDirectionLight)
	{
		if (MainDirectionalLight != nullptr)
		{
			FRotator directLightRotate = MainDirectionalLight->GetActorRotation();
			FRotator ModifiedRotate = FRotator(-90.0, directLightRotate.Yaw, directLightRotate.Roll);
			MainDirectionalLight->SetActorRotation(ModifiedRotate);

			GetWorld()->GetTimerManager().SetTimerForNextTick([this, directLightRotate, MainDirectionalLight]()
				{
					SceneCaptureComponent2D->CaptureScene();
					MainDirectionalLight->SetActorRotation(directLightRotate);
				});
		}

		else
			SceneCaptureComponent2D->CaptureScene();
	}

	else
		SceneCaptureComponent2D->CaptureScene();


	CreateMinimapTextureAsset();

}

void AOZWorldBoundCacher::CreateMinimapTextureAsset()
{
	const FString PackagePath = TEXT("/Game/Resources/Test_UIImage");
	const FString TextureName = TEXT("MinimapTexture");

	const FString AssetPath =
		PackagePath + TEXT("/") + TextureName + TEXT(".") + TextureName;

	// 1. 기존 Texture2D 찾기
	UTexture2D* ExistingTexture =
		LoadObject<UTexture2D>(nullptr, *AssetPath);

	if (ExistingTexture)
	{
		UpdateTextureFromRenderTarget(ExistingTexture, MinimapRenderTargetTexture);

		UE_LOG(LogTemp, Log, TEXT("MinimapTexture overwritten"));
		return;
	}

	// 2. 없으면 새로 생성
	UPackage* Package =
		CreatePackage(*FString::Printf(TEXT("%s/%s"), *PackagePath, *TextureName));

	Package->FullyLoad();

	UTexture2D* NewTexture =
		UKismetRenderingLibrary::RenderTargetCreateStaticTexture2DEditorOnly(
			MinimapRenderTargetTexture,
			TextureName,
			TC_Default,
			TMGS_FromTextureGroup
		);

	if (!NewTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create Texture2D from RenderTarget"));
		return;
	}

	NewTexture->Rename(*TextureName, Package);
	NewTexture->MarkPackageDirty();

	FAssetRegistryModule::AssetCreated(NewTexture);
	Package->SetDirtyFlag(true);
}

void AOZWorldBoundCacher::UpdateTextureFromRenderTarget(UTexture2D* Texture, UTextureRenderTarget2D* RenderTarget)
{
	if (!Texture || !RenderTarget)
		return;

	FTextureRenderTargetResource* RTResource =
		RenderTarget->GameThread_GetRenderTargetResource();

	TArray<FColor> Pixels;
	RTResource->ReadPixels(Pixels);

	Texture->Source.Init(
		RenderTarget->SizeX,
		RenderTarget->SizeY,
		1,
		1,
		TSF_BGRA8,
		reinterpret_cast<uint8*>(Pixels.GetData())
	);

	Texture->SRGB = true;

	Texture->MarkPackageDirty();
	Texture->PostEditChange();
}
