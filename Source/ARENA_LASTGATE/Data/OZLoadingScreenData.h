// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OZLoadingScreenData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class ARENA_LASTGATE_API UOZLoadingScreenData : public UDataAsset
{
	GENERATED_BODY()
	
public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<UTexture2D*> BackGroundTextures;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FString> HintTexts;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* LogoTexture;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* TrobberTexture;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* CoreTexture;
};
