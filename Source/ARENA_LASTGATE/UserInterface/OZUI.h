// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/OZPlayerController.h"
#include "OZUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIConstructed);

UCLASS()
class ARENA_LASTGATE_API UOZUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "OZ|Events")
	FOnUIConstructed OnUIConstructed;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AOZPlayerController> OwnerPlayerController = nullptr;

	UFUNCTION()
	void OnDelayedInitialize();
	
};
