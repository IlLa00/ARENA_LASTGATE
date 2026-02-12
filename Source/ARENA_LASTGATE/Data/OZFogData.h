#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZFogData.generated.h"

USTRUCT(BlueprintType)
struct FOZFogData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Fog_Phase = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Fog_Reduction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Fog_Move_Time = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Fog_Target_Size = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Fog_Damage_Delay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Fog_Damage = 0;
};
