// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "OZLoadingScreen.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLoadingScreen : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TObjectPtr<UTexture2D>> LoadingSceneBGImage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> LoadingSceneText;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnTimeOut();

	UFUNCTION(BlueprintImplementableEvent)
	void OnClientLoadingFail();
	
};
