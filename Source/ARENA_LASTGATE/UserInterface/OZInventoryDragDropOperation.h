#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZInventoryDragDropOperation.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	int32 DraggedSlotIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	int32 DraggedItemID = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	EOZItemType DraggedItemType = EOZItemType::None;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	int32 DraggedQuantity = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	UTexture2D* DraggedIcon = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Drag Drop")
	class UOZInvenEntry* SourceWidget = nullptr;
};
