// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/OZPracticeModuleInfoBar.h"
#include "OZPracticeCustomAvilityUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZPracticeCustomAvilityUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnInitializeEnd();

public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleInfos")
	TArray<TObjectPtr<UOZPracticeModuleInfoBar>> CustomModules;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D HP_Amount = FVector2D(0, 3500);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D Stemina_Amount = FVector2D(0, 350);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D WalkSpeed_Amount = FVector2D(100, 500);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D SprintSpeed_Amount = FVector2D(100, 1000);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D EvLDistance_Amount = FVector2D(0, 100);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D Shield_Amount = FVector2D(0, 100);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModuleMinMAx")
	FVector2D Armor_Amount = FVector2D(0, 50);

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleHP_Value = 10;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleStamina_Value = 3;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleWalkSpeed_Value = 5;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleSprintSpeed_Value = 5;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleEvLDistance_Value = 2;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleShield_Value = 10;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ModulePerValue")
	int ModuleArmor_Value = 3;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|ConverterWidget")
	TSubclassOf<class UOZTraningConverterWidget> ConverterWidget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|ConverterScrollBox")
	int ConverterNumperLine = 5;

	UPROPERTY(BlueprintReadWrite)
	TMap<int32, TObjectPtr<class UOZTraningConverterWidget>> ConverterWidgetMap;

	UFUNCTION(BlueprintCallable)
	void ShowCustomAvilityUI();

	UFUNCTION(BlueprintImplementableEvent)
	void OnShowCustomAvilityUI();

	UFUNCTION(BlueprintCallable)
	void HideCustomAvilityUI();

	UFUNCTION(BlueprintImplementableEvent)
	void OnHideCustomAvilityUI();


	UPROPERTY(BlueprintReadWrite)
	TArray<int32> savedModuleAmountArray;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> savedConverterIDArray;

private:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	TObjectPtr<class UScrollBox> ConverterListScrollBox;

	UPROPERTY()
	TObjectPtr<class UOZConvertSubsystem> converterSubsystem;

	void InitConverterScrollBox();

};
