// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/OZConvertData.h"
#include "Widget/OZTrainingConverterListWidget.h"
#include "Widget/OZConverterEquipSlotWidget.h"
#include "Subsystem/OZModuleSubsystem.h"
#include "HUD/OZPracticeHUD.h"
#include "OZTrainingRoomCustomUI.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FOZCustomModuleData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ModuleID = 0;

	UPROPERTY()
	TObjectPtr<UTexture2D> ModuleIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Module_Min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Module_Max = 0;

	UPROPERTY()
	int32 Module_PerValue = 0;

	UPROPERTY()
	FText Module_Name;
};

UCLASS()
class ARENA_LASTGATE_API UOZTrainingRoomCustomUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "OZ|ListSlotsPadd")
	float ListSlotPadd = 30.0f;

//Moduel Info
protected:
	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_HP;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_Stamina;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_WalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_SprintSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_EvLDistance;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_Shield;

	UPROPERTY(EditDefaultsOnly, Category = "OZ|CustomModule")
	FOZCustomModuleData Module_Armor;

	void ApplyCustomModule(const FOZModuleData& moduleData, FOZCustomModuleData& cumtomModuleData);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_HP = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_Stamina = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_WalkSpeed = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_SprintSpeed = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_EvLDistance = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_Shield = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTrainingCustomModuelWidget> CustomModuleWidget_Armor = nullptr;

protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnNativeConstructEnd();

	void PrepareCustomModule();

	UFUNCTION(BlueprintCallable)
	void PrepareSlots();

	UPROPERTY(BlueprintReadWrite)
	TArray<UOZConverterEquipSlotWidget*> equipSlotArray;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AOZPracticeHUD> OZPracticeHUD = nullptr;


public:
	UPROPERTY(BlueprintReadWrite)
	TMap<int, TObjectPtr<class UOZConverterListSlotWidget>> totalConverterMap;

	UPROPERTY(BlueprintReadWrite)
	TMap<EConvertGrade, TObjectPtr<class UOZTrainingConverterListWidget>> TierSlot_Map;

	UFUNCTION(BlueprintCallable)
	TArray<FOZConvertData> GetEquipSlotConvertData();

	UFUNCTION(BlueprintCallable)
	void SetOwnerHUD(class AOZPracticeHUD* owningHUD);

private:
	UPROPERTY()
	TObjectPtr<class UOZConvertSubsystem> converterSubsystem;

	UPROPERTY()
	TObjectPtr<class UOZModuleSubsystem> moduleSubsystem;

	void InitModuleWidget();
	void InitListLayout();

	UPROPERTY(EditDefaultsOnly, Category = "OZ|ListSlotWidget")
	TSubclassOf<class UOZConverterListSlotWidget> ListSlotWidget;

	UFUNCTION()
	void OnListSlotSelected(class UOZConverterListSlotWidget* clickedWidget, bool bIsLeftMouseClick);

	UFUNCTION()
	void OnEquipSlotSelected(FOZConvertData assingedConvertData, int equipIndex);

	void MakeListSlot();
	void MakeEquipSlot();

	bool CheckOverlap(int convertID);

	bool AddEquip(FOZConvertData addedConvertData);

	/*UPROPERTY(EditDefaultsOnly, Category = "OZ|EquipSlotWidget")
	TSubclassOf<class UOZConverterListSlotWidget> ListSlotWidget;*/
};
