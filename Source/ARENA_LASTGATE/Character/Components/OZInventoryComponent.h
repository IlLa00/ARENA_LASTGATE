#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OZInventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EOZItemType : uint8
{
    None,
    Battle,
    Buff
};

USTRUCT(BlueprintType)
struct FOZInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 ItemID = 0;

    UPROPERTY(BlueprintReadWrite)
    EOZItemType ItemType = EOZItemType::None;

    UPROPERTY(BlueprintReadWrite)
    int32 Quantity = 0;

    bool IsEmpty() const { return ItemID == 0 || Quantity <= 0; }

    void Clear()
    {
        ItemID = 0;
        ItemType = EOZItemType::None;
        Quantity = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARENA_LASTGATE_API UOZInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOZInventoryComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable)
    FOnInventoryUpdated OnInventoryUpdated;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    const TArray<FOZInventorySlot>& GetAllSlots() const { return ItemSlots; }

    UFUNCTION(BlueprintPure, Category = "Inventory")
    FOZInventorySlot GetSlot(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetTotalItemQuantity(int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasEmptySlot() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 FindSlotWithItem(int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetUniqueBattleItemCount() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FText GetSlotItemName(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetSlotItemQuantity(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    FText GetSlotItemDescription(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetSlotItemPrice(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetSlotItemMaxStack(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    TSoftObjectPtr<UTexture2D> GetSlotItemIcon(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 GetItemSlotItemID(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EOZItemType GetItemSlotItemType(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(int32 ItemID, EOZItemType ItemType, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(int32 SlotIndex, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SwapSlots(int32 SlotIndexA, int32 SlotIndexB);

    UFUNCTION(BlueprintCallable, Category = "OZ|Inventory")
    TSoftObjectPtr<UTexture2D> GetBuffItemIconByID(int32 ItemID) const;

    UFUNCTION(BlueprintCallable, Category = "OZ|Inventory")
    FText GetBuffItemNameByID(int32 ItemID) const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(ReplicatedUsing = OnRep_ItemSlots, BlueprintReadOnly)
    TArray<FOZInventorySlot> ItemSlots;

    UFUNCTION()
    void OnRep_ItemSlots();

private:
    static constexpr int32 MaxSlots = 4;

    int32 FindEmptySlot() const;
};