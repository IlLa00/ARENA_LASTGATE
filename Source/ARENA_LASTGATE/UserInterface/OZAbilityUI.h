// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZAbilityUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZAbilityUI : public UOZUI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowAbilityUICall();

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void UpdateConverts();

	/*UPROPERTY(BlueprintReadOnly, Category = "OZ|AbilityWidget", meta = (BindWidget))
	TObjectPtr<class UOZAbilityWidget> AbilityWidget1;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|AbilityWidget", meta = (BindWidget))
	TObjectPtr<class UOZAbilityWidget> AbilityWidget2;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|AbilityWidget", meta = (BindWidget))
	TObjectPtr<class UOZAbilityWidget> AbilityWidget3;*/
	
};
