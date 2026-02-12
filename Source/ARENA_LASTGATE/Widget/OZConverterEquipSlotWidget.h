// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/OZConvertData.h"
#include "OZConverterEquipSlotWidget.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipSlotSelected, FOZConvertData, converterData, int, equipIndex);

UCLASS()
class ARENA_LASTGATE_API UOZConverterEquipSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	FOnEquipSlotSelected OnEquipSlotSelected;

	UFUNCTION(BlueprintCallable)
	void SetSlotDefault();

	UFUNCTION(BlueprintCallable)
	void SetEquipSlotConverter(FOZConvertData assingedConverter);

	UFUNCTION(BlueprintCallable)
	void BroadCastSelected();

	UPROPERTY(BlueprintReadWrite)
	int assingedConvertID = -1;

	UPROPERTY(BlueprintReadWrite)
	int assingedEquipIndex = -1;

	FOZConvertData& GetConvertData() { return assingedConvertData; }

protected:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> TierEffectImage = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> ConverterIconImage = nullptr;
	
private:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EffectImage_Legendaray;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EffectImage_Unique;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EffectImage_Rare;

	FOZConvertData assingedConvertData;
};
