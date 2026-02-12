// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZPlayerLobbyPriview.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ARENA_LASTGATE_API UOZPlayerLobbyPriview : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|HorizontalBox")
	TObjectPtr<class UHorizontalBox> HoriontalBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|UserName", meta = (BindWidget))
	TObjectPtr<class UTextBlock> PreviewUserNameText;

};
