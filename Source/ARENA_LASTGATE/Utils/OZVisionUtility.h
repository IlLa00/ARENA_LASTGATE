#pragma once

#include "CoreMinimal.h"
#include "Engine/Canvas.h"
#include "OZVisionUtility.generated.h"

USTRUCT()
struct ARENA_LASTGATE_API FOZVisionUtility
{
	GENERATED_BODY()

public:
	static TArray<FVector> CreateVisionCone(
		UWorld* World,
		const FVector& Origin,
		const FVector& ForwardVector,
		float Range,
		int32 NumTraces,
		float DegreePerTrace,
		AActor* IgnoreActor,
		TSet<class AActor*>* OutDetectedMinimapVisibleObjects = nullptr,
		TSet<class AOZPlayer*>* OutDetectedPlayers = nullptr
	);

	static TArray<FCanvasUVTri> PrepareTriangles(
		const TArray<FVector>& TraceResults,
		const FVector& CenterLocation,
		float TraceRange
	);

	// 원형 영역을 삼각형으로 생성 (타워 시야용)
	static TArray<FCanvasUVTri> CreateCircleTriangles(
		const FVector& CenterLocation,
		float Radius,
		int32 Segments = 32
	);

	static void DrawToRenderTarget(
		UWorld* World,
		UTextureRenderTarget2D* RenderTarget,
		const TArray<FCanvasUVTri>& Triangles,
		bool bClearFirst = true
	);
};
