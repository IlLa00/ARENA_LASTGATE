#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "Character/Components/OZInventoryComponent.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "OZInvenEntry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnInvenEntryClicked, int32, SlotIndex, int32, ItemID, EOZItemType, ItemType, UTexture2D*, Icon, int32, Quantity);

UCLASS()
class ARENA_LASTGATE_API UOZInvenEntry : public UOZUI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetupEntry(const FText& InName, UTexture2D* InIcon, int32 InQuantity, int32 InSlotIndex, int32 InItemID, EOZItemType InItemType);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearEntry();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RestoreOpacity();

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInvenEntryClicked OnEntryClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UTexture2D* DefaultSlotIcon = nullptr;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Icon;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Quantity;

	UPROPERTY()
	UTexture2D* ItemIcon = nullptr;

	UPROPERTY()
	int32 ItemQuantity = 0;

	UPROPERTY()
	int32 SlotIndex = -1;

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	EOZItemType ItemType = EOZItemType::None;

	bool bIsPressed = false;
};
