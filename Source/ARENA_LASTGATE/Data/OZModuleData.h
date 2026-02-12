#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZModuleData.generated.h"

USTRUCT(BlueprintType)
struct FOZModuleData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Module_ID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ValueMin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ValueMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ValueStep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ScaleMulti;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText description;

	// CSV에서 에셋 경로 문자열로 저장 가능
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class UGameplayEffect> Module_Asset;

	UPROPERTY(BlueprintReadOnly)
	float FinalValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ScaledMinValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float ScaledMaxValue = 0.0f;
};
