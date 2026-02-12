// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZSessionMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionBtnClicked, uint8, btnIndex);

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZSessionMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	uint8 sessionIndex = -1;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnSessionBtnClicked OnSessionBtnClicked;
};
