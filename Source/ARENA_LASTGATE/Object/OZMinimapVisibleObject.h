// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/OZIMinimapVisibleActor.h"
#include "OZMinimapVisibleObject.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZMinimapVisibleObject : public AActor, public IOZIMinimapVisibleActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZMinimapVisibleObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|ObjectMinimapCategory")
	EMinimapActorCategory MinimapActorCategory = EMinimapActorCategory::Structure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|ObjectMinimapIcon")
	TObjectPtr<UTexture2D> MinimapIcon = nullptr;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMeshComponent;

	virtual UTexture2D* GetMinimapIcon_Implementation() const override
	{
		return MinimapIcon;
	}

	virtual EMinimapActorCategory GetMinimapCategory_Implementation() const override
	{
		return MinimapActorCategory;
	}
};
