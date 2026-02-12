// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZLevellVisionMapMaker.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
//#include "Detour/DetourNavMeshQuery.h"
#include "ImageUtils.h"

// Sets default values
AOZLevellVisionMapMaker::AOZLevellVisionMapMaker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOZLevellVisionMapMaker::BeginPlay()
{
	Super::BeginPlay();
	
    int xScale = 50;
    int yScale = 50;

    FVector2D AreaMin(-2500, -2500);
    FVector2D AreaMax(2500, 2500);

    ExportNavMeshBinaryPNG(GetWorld(), AreaMin, AreaMax, imageResolution, TEXT("C:\\Users\\inha\\Desktop\\Test\\Test.png"));
}

// Called every frame
void AOZLevellVisionMapMaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZLevellVisionMapMaker::ExportNavMeshBinaryPNG(UWorld* World, const FVector2D AreaMin, const FVector2D AreaMax, int32 Resolution, const FString OutputPath)
{
    if (!World) return;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys) return;

    ARecastNavMesh* RecastNavMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate));
    if (!RecastNavMesh) return;

    int32 Width = Resolution;
    int32 Height = Resolution;
    TArray<FColor> Pixels;
    Pixels.SetNumZeroed(Width * Height);

    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            float FX = FMath::Lerp(AreaMin.X, AreaMax.X, (float)X / (Width - 1));
            float FY = FMath::Lerp(AreaMin.Y, AreaMax.Y, (float)Y / (Height - 1));

            // Z값은 바닥의 높이로 할 것
            FVector QueryPos(FX, FY, 0.f);

            FNavLocation ProjectedLocation;

            bool bIsNavigable = NavSys->ProjectPointToNavigation(QueryPos, ProjectedLocation, Find_Extent, RecastNavMesh);

            Pixels[Y * Width + X] = bIsNavigable ? FColor::White : FColor::Black;


            if (X == 0 || Y == 0 || X == 511 || Y == 511)
            {
                DrawDebugBox(
                    World,
                    QueryPos,
                    Find_Extent,
                    FColor::Yellow,
                    false,          // 지속시간: false면 1프레임만
                    2.0f,           // 지속시간 2초
                    0,              // 깊이 우선순위
                    0.1f            // 선 두께
                );
            }

            


        }
    }

    FIntPoint Size(Width, Height);
    const TCHAR* Path = *OutputPath;

    FImageView ImageView(
        Pixels.GetData(),  // void* InData
        Width,            // int32 InSizeX
        Height,           // int32 InSizeY
        ERawImageFormat::BGRA8  // 포맷 (FColor 기준)
    );

    FImageUtils::SaveImageByExtension(*OutputPath, ImageView, 100);

    UE_LOG(LogTemp, Log, TEXT("NavMesh binary PNG saved: %s"), *OutputPath);
}

//void ExportNavMeshBinaryPNG(UWorld* World, const FVector2D AreaMin, const FVector2D AreaMax, int32 Resolution, const FString& OutputPath)
//{
//    if (!World) return;
//
//    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
//    if (!NavSys) return;
//
//    ARecastNavMesh* RecastNavMesh = Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate));
//    if (!RecastNavMesh) return;
//
//    int32 Width = Resolution;
//    int32 Height = Resolution;
//    TArray<FColor> Pixels;
//    Pixels.SetNumZeroed(Width * Height);
//
//    for (int32 Y = 0; Y < Height; ++Y)
//    {
//        for (int32 X = 0; X < Width; ++X)
//        {
//            float FX = FMath::Lerp(AreaMin.X, AreaMax.X, (float)X / (Width - 1));
//            float FY = FMath::Lerp(AreaMin.Y, AreaMax.Y, (float)Y / (Height - 1));
//
//            // Z값은 네비메쉬 높이를 고려해 적절히 설정 필요 (예: 0 또는 NavMeshBoundsVolume 중간 높이)
//            FVector QueryPos(FX, FY, 0.f);
//
//            FNavLocation ProjectedLocation;
//            FVector Extent(50.f, 50.f, 200.f); // 탐색 영역: XY 적당히, Z 충분히 크게
//
//            bool bIsNavigable = NavSys->ProjectPointToNavigation(QueryPos, ProjectedLocation, Extent, RecastNavMesh);
//
//            Pixels[Y * Width + X] = bIsNavigable ? FColor::White : FColor::Black;
//        }
//    }
//
//    FIntPoint Size(Width, Height);
//    //FImageUtils::SaveImageByExtension(OutputPath, FImageView(Pixels.GetData(), Size), 100);
//
//    UE_LOG(LogTemp, Log, TEXT("NavMesh binary PNG saved: %s"), *OutputPath);
//}

