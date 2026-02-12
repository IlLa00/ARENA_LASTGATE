#include "OZInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"
#include "Engine/GameInstance.h"

UOZInventoryComponent::UOZInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);
}

void UOZInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    ItemSlots.SetNum(MaxSlots);
}

void UOZInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UOZInventoryComponent, ItemSlots);
}

void UOZInventoryComponent::OnRep_ItemSlots()
{
    OnInventoryUpdated.Broadcast();
}

FOZInventorySlot UOZInventoryComponent::GetSlot(int32 SlotIndex) const
{
    if (ItemSlots.IsValidIndex(SlotIndex))
        return ItemSlots[SlotIndex];

    return FOZInventorySlot();
}

int32 UOZInventoryComponent::GetTotalItemQuantity(int32 ItemID, EOZItemType ItemType) const
{
    int32 Total = 0;

    for (const FOZInventorySlot& Slot : ItemSlots)
    {
        if (Slot.ItemID == ItemID && Slot.ItemType == ItemType)
            Total += Slot.Quantity;
    }

    return Total;
}

bool UOZInventoryComponent::HasEmptySlot() const
{
    return FindEmptySlot() != INDEX_NONE;
}

int32 UOZInventoryComponent::FindSlotWithItem(int32 ItemID, EOZItemType ItemType) const
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i].ItemID == ItemID && ItemSlots[i].ItemType == ItemType)
            return i;
    }

    return INDEX_NONE;
}

int32 UOZInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < ItemSlots.Num(); ++i)
    {
        if (ItemSlots[i].IsEmpty())
            return i;
    }

    return INDEX_NONE;
}

int32 UOZInventoryComponent::GetUniqueBattleItemCount() const
{
    TSet<int32> UniqueBattleItems;

    for (const FOZInventorySlot& Slot : ItemSlots)
    {
        if (Slot.ItemType == EOZItemType::Battle && !Slot.IsEmpty())
        {
            UniqueBattleItems.Add(Slot.ItemID);
        }
    }

    return UniqueBattleItems.Num();
}

bool UOZInventoryComponent::AddItem(int32 ItemID, EOZItemType ItemType, int32 Amount)
{
    int32 ExistingSlot = FindSlotWithItem(ItemID, ItemType);

    if (ExistingSlot != INDEX_NONE)
    {
        ItemSlots[ExistingSlot].Quantity += Amount;
        OnRep_ItemSlots();
        return true;
    }

    // 배틀아이템 3종류 제한 체크
    if (ItemType == EOZItemType::Battle)
    {
        int32 UniqueBattleItemCount = GetUniqueBattleItemCount();
        if (UniqueBattleItemCount >= 3)
            return false;
    }

    int32 EmptySlot = FindEmptySlot();
    if (EmptySlot != INDEX_NONE)
    {
        ItemSlots[EmptySlot].ItemID = ItemID;
        ItemSlots[EmptySlot].ItemType = ItemType;
        ItemSlots[EmptySlot].Quantity = Amount;
        OnRep_ItemSlots();
        return true;
    }

    return false;
}

bool UOZInventoryComponent::RemoveItem(int32 SlotIndex, int32 Amount)
{
    if (!ItemSlots.IsValidIndex(SlotIndex)) return false;

    FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    if (Slot.IsEmpty()) return false;

    Slot.Quantity -= Amount;

    if (Slot.Quantity <= 0)
        Slot.Clear();

    OnRep_ItemSlots();

    return true;
}

FText UOZInventoryComponent::GetSlotItemName(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return FText::GetEmpty();

    const FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return FText::GetEmpty();

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return FText::GetEmpty();

    switch (Slot.ItemType)
    {
        case EOZItemType::Battle:
            if (FOZBattleItemData* Data = ItemSubsystem->GetBattleItemData(Slot.ItemID))
                return Data->Item_Name;
            break;

        case EOZItemType::Buff:
            if (FOZBuffItemData* Data = ItemSubsystem->GetBuffItemData(Slot.ItemID))
                return Data->Item_Name;
            break;
    }

    return FText::GetEmpty();
}

int32 UOZInventoryComponent::GetSlotItemQuantity(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return 0;

    return ItemSlots[SlotIndex].Quantity;
}

FText UOZInventoryComponent::GetSlotItemDescription(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return FText::GetEmpty();

    const FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return FText::GetEmpty();

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return FText::GetEmpty();

    switch (Slot.ItemType)
    {
        case EOZItemType::Battle:
            if (FOZBattleItemData* Data = ItemSubsystem->GetBattleItemData(Slot.ItemID))
                return Data->Explanation;
            break;

        case EOZItemType::Buff:
            if (FOZBuffItemData* Data = ItemSubsystem->GetBuffItemData(Slot.ItemID))
                return Data->Explanation;
            break;
    }

    return FText::GetEmpty();
}

int32 UOZInventoryComponent::GetSlotItemPrice(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return 0;

    const FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return 0;

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return 0;

    switch (Slot.ItemType)
    {
        case EOZItemType::Battle:
            return ItemSubsystem->GetBattleItemPrice(Slot.ItemID);

        case EOZItemType::Buff:
            return ItemSubsystem->GetBuffItemPrice(Slot.ItemID);
    }

    return 0;
}

int32 UOZInventoryComponent::GetSlotItemMaxStack(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return 0;

    const FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return 0;

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return 0;

    switch (Slot.ItemType)
    {
        case EOZItemType::Battle:
            return ItemSubsystem->GetBattleItemMaxStack(Slot.ItemID);

        case EOZItemType::Buff:
            return ItemSubsystem->GetBuffItemMaxStack(Slot.ItemID);
    }

    return 0;
}

TSoftObjectPtr<UTexture2D> UOZInventoryComponent::GetSlotItemIcon(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return nullptr;

    const FOZInventorySlot& Slot = ItemSlots[SlotIndex];
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return nullptr;

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return nullptr;

    switch (Slot.ItemType)
    {
        case EOZItemType::Battle:
            if (FOZBattleItemData* Data = ItemSubsystem->GetBattleItemData(Slot.ItemID))
                return Data->ItemIcon;
            break;

        case EOZItemType::Buff:
            if (FOZBuffItemData* Data = ItemSubsystem->GetBuffItemData(Slot.ItemID))
                return Data->ItemIcon;
            break;
    }

    return nullptr;
}



int32 UOZInventoryComponent::GetItemSlotItemID(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return 0;

    return ItemSlots[SlotIndex].ItemID;
}

EOZItemType UOZInventoryComponent::GetItemSlotItemType(int32 SlotIndex) const
{
    if (!ItemSlots.IsValidIndex(SlotIndex) || ItemSlots[SlotIndex].IsEmpty())
        return EOZItemType::None;

    return ItemSlots[SlotIndex].ItemType;
}

void UOZInventoryComponent::SwapSlots(int32 SlotIndexA, int32 SlotIndexB)
{
    if (!ItemSlots.IsValidIndex(SlotIndexA) || !ItemSlots.IsValidIndex(SlotIndexB))
        return;

    if (SlotIndexA == SlotIndexB)
        return;

    FOZInventorySlot TempSlot = ItemSlots[SlotIndexA];
    ItemSlots[SlotIndexA] = ItemSlots[SlotIndexB];
    ItemSlots[SlotIndexB] = TempSlot;

    OnRep_ItemSlots();
}

TSoftObjectPtr<UTexture2D> UOZInventoryComponent::GetBuffItemIconByID(int32 ItemID) const
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return nullptr;

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return nullptr;

    if (FOZBuffItemData* Data = ItemSubsystem->GetBuffItemData(ItemID))
    {
        return Data->ItemIcon;
    }

    return nullptr;
}

FText UOZInventoryComponent::GetBuffItemNameByID(int32 ItemID) const
{
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return FText::GetEmpty();

    UOZItemSubsystem* ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
    if (!ItemSubsystem) return FText::GetEmpty();

    if (FOZBuffItemData* Data = ItemSubsystem->GetBuffItemData(ItemID))
    {
        return Data->Item_Name;
    }

    UE_LOG(LogTemp, Warning, TEXT("[InvComp] this=%s World=%s Owner=%s Outer=%s"),
        *GetNameSafe(this),
        *GetNameSafe(GetWorld()),
        *GetNameSafe(GetOwner()),
        *GetNameSafe(GetOuter()));

    return FText::GetEmpty();
}
