#include "Utils/OZVisionUtility.h"
#include "Character/OZPlayer.h"
#include "Interface/OZIMinimapVisibleActor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "CollisionQueryParams.h"

TArray<FVector> FOZVisionUtility::CreateVisionCone(
	UWorld* World,
	const FVector& Origin,
	const FVector& ForwardVector,
	float Range,
	int32 NumTraces,
	float DegreePerTrace,
	AActor* IgnoreActor,
	TSet<AActor*>* OutDetectedMinimapVisibleObjects,
	TSet<AOZPlayer*>* OutDetectedPlayers)
{
	TArray<FVector> TraceResults;

	if (!World)
		return TraceResults;

	TraceResults.Empty();

	if (OutDetectedPlayers)
	{
		OutDetectedPlayers->Empty();
	}

	float StartAngle = -(NumTraces * DegreePerTrace) / 2.0f;

	for (int32 i = 0; i < NumTraces; i++)
	{
		float CurrentAngle = StartAngle + (DegreePerTrace * i);
		FRotator Rotation = FRotator(0.0f, CurrentAngle, 0.0f);
		FVector RotatedDirection = Rotation.RotateVector(ForwardVector);
		FVector Start = Origin;
		FVector End = Origin + (RotatedDirection * Range);

		TArray<FHitResult> HitResults;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;

		if (IgnoreActor)
		{
			QueryParams.AddIgnoredActor(IgnoreActor);
		}

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bHit = World->LineTraceMultiByObjectType(
			HitResults,
			Start,
			End,
			ObjectParams,
			QueryParams
		);

		FVector TraceEndPoint = End + (RotatedDirection * 10.0f);
		bool bFoundWorldStatic = false;

		if (bHit && HitResults.Num() > 0)
		{
			// WorldStatic 먼저 찾기
			for (const FHitResult& Hit : HitResults)
			{
				if (Hit.Component.IsValid() &&
					Hit.Component->GetCollisionObjectType() == ECC_WorldStatic)
				{
					TraceEndPoint = Hit.Location + (RotatedDirection * 10.0f);
					bFoundWorldStatic = true;
					break;
				}
			}

			if (OutDetectedMinimapVisibleObjects)
			{
				for (const FHitResult& Hit : HitResults)
				{
					AActor* hittedActor = Hit.GetActor();
					if (hittedActor && hittedActor->GetClass()->ImplementsInterface(UOZIMinimapVisibleActor::StaticClass()))
					{
						// WorldStatic이 없거나, 플레이어가 그 앞에 있으면 감지
						if (!bFoundWorldStatic || Hit.Distance < (TraceEndPoint - Start).Size())
						{
							OutDetectedMinimapVisibleObjects->Add(hittedActor);
						}
					}
				}
			}


			// WorldStatic 앞에 있는 플레이어 감지
			if (OutDetectedPlayers)
			{
				for (const FHitResult& Hit : HitResults)
				{
					if (AOZPlayer* Player = Cast<AOZPlayer>(Hit.GetActor()))
					{
						// WorldStatic이 없거나, 플레이어가 그 앞에 있으면 감지
						if (!bFoundWorldStatic || Hit.Distance < (TraceEndPoint - Start).Size())
						{
							OutDetectedPlayers->Add(Player);
						}
					}
				}
			}
		}

		TraceResults.Add(TraceEndPoint);
	}

	return TraceResults;
}

TArray<FCanvasUVTri> FOZVisionUtility::PrepareTriangles(
	const TArray<FVector>& TraceResults,
	const FVector& CenterLocation,
	float TraceRange)
{
	TArray<FCanvasUVTri> CanvasTriangles;
	CanvasTriangles.Empty();

	if (TraceResults.Num() < 2)
		return CanvasTriangles;

	FVector RenderTargetCenter = FVector(900.0, 900.0, 0.0);
	float Scale = 900.f / TraceRange;

	for (int32 i = 0; i < TraceResults.Num() - 1; ++i)
	{
		// 첫 번째 점
		FVector Point0 = TraceResults[i];
		FVector RelativePoint0 = Point0 - CenterLocation;
		FVector ScaledPoint0 = RelativePoint0 * Scale;
		FVector FinalPoint0 = ScaledPoint0 + RenderTargetCenter;
		FVector2D V0_Pos = FVector2D(FinalPoint0.X, FinalPoint0.Y);

		// 두 번째 점
		FVector Point1 = TraceResults[i + 1];
		FVector RelativePoint1 = Point1 - CenterLocation;
		FVector ScaledPoint1 = RelativePoint1 * Scale;
		FVector FinalPoint1 = ScaledPoint1 + RenderTargetCenter;
		FVector2D V1_Pos = FVector2D(FinalPoint1.X, FinalPoint1.Y);

		// 중심점
		FVector2D V2_Pos = FVector2D(RenderTargetCenter.X, RenderTargetCenter.Y);

		// 삼각형 생성
		FCanvasUVTri Triangle;

		Triangle.V0_Pos = V0_Pos;
		Triangle.V0_UV = FVector2D(0.0f, 0.0f);
		Triangle.V0_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

		Triangle.V1_Pos = V1_Pos;
		Triangle.V1_UV = FVector2D(0.0f, 0.0f);
		Triangle.V1_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

		Triangle.V2_Pos = V2_Pos;
		Triangle.V2_UV = FVector2D(0.0f, 0.0f);
		Triangle.V2_Color = FLinearColor(1.0f, 1.0f, 1.0f, 0.0f);

		CanvasTriangles.Add(Triangle);
	}

	return CanvasTriangles;
}

TArray<FCanvasUVTri> FOZVisionUtility::CreateCircleTriangles(
	const FVector& CenterLocation,
	float Radius,
	int32 Segments)
{
	TArray<FCanvasUVTri> CanvasTriangles;

	if (Segments < 3 || Radius <= 0.f)
		return CanvasTriangles;

	// RenderTarget 기준 (1800x1800, 중심 900,900)
	FVector2D RenderTargetCenter = FVector2D(900.f, 900.f);
	float Scale = 900.f / Radius;

	// 월드 좌표를 RenderTarget 좌표로 변환할 때 CenterLocation이 RenderTarget 중심이 됨
	// 원형이므로 중심에서 Radius만큼 떨어진 점들을 연결

	for (int32 i = 0; i < Segments; i++)
	{
		float Angle1 = (static_cast<float>(i) / Segments) * 2.f * PI;
		float Angle2 = (static_cast<float>(i + 1) / Segments) * 2.f * PI;

		// 원 둘레의 두 점 (RenderTarget 좌표계)
		FVector2D P1 = RenderTargetCenter + FVector2D(FMath::Cos(Angle1), FMath::Sin(Angle1)) * 900.f;
		FVector2D P2 = RenderTargetCenter + FVector2D(FMath::Cos(Angle2), FMath::Sin(Angle2)) * 900.f;

		// 삼각형: 중심 - P1 - P2
		FCanvasUVTri Triangle;

		Triangle.V0_Pos = P1;
		Triangle.V0_UV = FVector2D(0.f, 0.f);
		Triangle.V0_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

		Triangle.V1_Pos = P2;
		Triangle.V1_UV = FVector2D(0.f, 0.f);
		Triangle.V1_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

		Triangle.V2_Pos = RenderTargetCenter;
		Triangle.V2_UV = FVector2D(0.f, 0.f);
		Triangle.V2_Color = FLinearColor(1.f, 1.f, 1.f, 0.f);

		CanvasTriangles.Add(Triangle);
	}

	return CanvasTriangles;
}

void FOZVisionUtility::DrawToRenderTarget(
	UWorld* World,
	UTextureRenderTarget2D* RenderTarget,
	const TArray<FCanvasUVTri>& Triangles,
	bool bClearFirst)
{
	if (!World || !RenderTarget || Triangles.Num() == 0)
		return;

	if (bClearFirst)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(World, RenderTarget);
	}

	UCanvas* Canvas;
	FVector2D Size;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, RenderTarget, Canvas, Size, Context);

	Canvas->K2_DrawTriangle(nullptr, Triangles);

	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(World, Context);
}
