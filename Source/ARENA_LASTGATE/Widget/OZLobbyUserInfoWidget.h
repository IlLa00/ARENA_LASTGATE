// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLobbyUserInfoWidget.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ARENA_LASTGATE_API UOZLobbyUserInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> InfoBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> UserNameText;
};
