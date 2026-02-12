// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UserInterface/OZTrainingRoomCustomUI.h"
#include "OZTrainingCustomModuelWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZTrainingCustomModuelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetData(FOZCustomModuleData moduleData);

protected:

	UPROPERTY(BlueprintReadWrite)
	int Module_Value = 0;

	UPROPERTY(BlueprintReadWrite)
	int Module_Max = 0;

	UPROPERTY(BlueprintReadWrite)
	int Module_Min = 0;

	UPROPERTY(BlueprintReadWrite)
	int Module_ID = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void SetName(const FText& moduleName);

	UFUNCTION(BlueprintCallable)
	int Call_GetAmount();

	UFUNCTION(BlueprintImplementableEvent)
	int GetAmount();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_ModuelIcon;

};
