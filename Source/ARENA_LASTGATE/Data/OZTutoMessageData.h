#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OZTutoMessageData.generated.h"

USTRUCT(BlueprintType)
struct FOZTutoMessageData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Msg_ID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString MsgText;

};