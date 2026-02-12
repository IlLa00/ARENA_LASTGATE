// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZAbilityWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZAbilityWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//UPROPERTY(BlueprintReadOnly, Category = "OZ|Button", meta = (BindWidget))
	//TObjectPtr<class UButton> Button_Avility;

	//UPROPERTY(BlueprintReadOnly, Category = "OZ|AvilityImage", meta = (BindWidget))


	//TObjectPtr<class UImage> Image_AvilityImage;

	//UPROPERTY(BlueprintReadOnly, Category = "OZ|AvilityName", meta = (BindWidget))
	//TObjectPtr<class UTextBlock> Text_AvilityName;

	//UPROPERTY(BlueprintReadOnly, Category = "OZ|AvilityExplain", meta = (BindWidget))
	//TObjectPtr<class UTextBlock> Text_AvilityExplain;

	UFUNCTION(BlueprintImplementableEvent)
	void SelectAutoRandomConvert();

};
