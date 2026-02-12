// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Utils/Util.h"
#include "OZIMinimapVisibleActor.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UOZIMinimapVisibleActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARENA_LASTGATE_API IOZIMinimapVisibleActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UTexture2D* GetMinimapIcon() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EMinimapActorCategory GetMinimapCategory() const;
};
