// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZFlyingRewordCoreWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlyingAnimEnd, int, assingedIndex);

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZFlyingRewordCoreWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;
	
public:

	UPROPERTY(BlueprintAssignable, Category = "Init")
	FOnFlyingAnimEnd OnFlyingAnimEnd;

	UPROPERTY(BlueprintReadWrite)
	FVector2D InitOffset;

	UFUNCTION(BlueprintCallable)
	void OnActivated();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStarted();

	UPROPERTY(BlueprintReadWrite)
	int assingedIndex = -1;

	UFUNCTION(BlueprintCallable)
	void BroadCast_WidgetOnTarget();
};
