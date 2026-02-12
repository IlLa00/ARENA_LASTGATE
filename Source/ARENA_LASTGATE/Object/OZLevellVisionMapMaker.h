// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZLevellVisionMapMaker.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZLevellVisionMapMaker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZLevellVisionMapMaker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ExportNavMeshBinaryPNG(UWorld* World, const FVector2D AreaMin, const FVector2D AreaMax, int32 Resolution, const FString OutputPath);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 imageResolution = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Find_Extent = FVector(20.f, 20.f, 10.f);
};
