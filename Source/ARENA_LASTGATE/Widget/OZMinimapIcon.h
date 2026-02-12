// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZMinimapIcon.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZMinimapIcon : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> DefaultIconTexture = nullptr;
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_icon;

public:
	void SetIconTexture(UTexture2D* NewTexture, FLinearColor customColor = FLinearColor::White);
	void ResetIcon();

	UPROPERTY(EditAnywhere)
	FVector2D IconSize = FVector2D(50.0f, 50.0f);
};
