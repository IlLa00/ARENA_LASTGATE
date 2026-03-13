#pragma once

#include "CoreMinimal.h"
#include "Character/Components/OZInventoryComponent.h"
#include "Data/OZConvertData.h"
#include "GameplayTagContainer.h"
#include "OZViewModelTypes.generated.h"

USTRUCT(BlueprintType)
struct FOZInventorySlotDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 Quantity = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadOnly)
	EOZItemType ItemType = EOZItemType::None;

	bool IsEmpty() const { return ItemID == 0 || Quantity <= 0; }
};

USTRUCT(BlueprintType)
struct FOZConvertSlotDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	EConvertGrade Grade = EConvertGrade::None;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	bool IsValid() const { return Grade != EConvertGrade::None && IconTexture != nullptr; }
};

USTRUCT(BlueprintType)
struct FOZBuffDisplayData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 ItemID = 0;

	UPROPERTY(BlueprintReadOnly)
	FText BuffName;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> BuffIcon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.f;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag BuffTag;
};
