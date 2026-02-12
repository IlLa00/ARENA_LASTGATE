#include "OZShopManager.h"
#include "Net/UnrealNetwork.h"
#include "Character/OZPlayerState.h"
#include "Subsystem/OZItemSubSystem.h"
#include "Subsystem/OZModuleSubsystem.h"
#include "Character/Components/OZInventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

AOZShopManager::AOZShopManager()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    bAlwaysRelevant = true;
}

void AOZShopManager::BeginPlay()
{
    Super::BeginPlay();

    if (UGameInstance* GI = Cast<UGameInstance>(UGameplayStatics::GetGameInstance(this)))
    {
        ItemSubSystem = GI->GetSubsystem<UOZItemSubsystem>();
    }
}

void AOZShopManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

UOZItemSubsystem* AOZShopManager::GetItemSubSystem() const
{
    return ItemSubSystem;
}

int32 AOZShopManager::GetItemPrice(int32 ItemID, EOZItemType ItemType) const
{
    if (!ItemSubSystem) 
        return 0;

    if (ItemType == EOZItemType::Battle)
    {
        return ItemSubSystem->GetBattleItemPrice(ItemID);
    }
    else if (ItemType == EOZItemType::Buff)
    {
        return ItemSubSystem->GetBuffItemPrice(ItemID);
    }

    return 0;
}

int32 AOZShopManager::GetItemMaxStack(int32 ItemID, EOZItemType ItemType) const
{
    if (!ItemSubSystem) return 0;

    if (ItemType == EOZItemType::Battle)
    {
        return ItemSubSystem->GetBattleItemMaxStack(ItemID);
    }
    else if (ItemType == EOZItemType::Buff)
    {
        return ItemSubSystem->GetBuffItemMaxStack(ItemID);
    }

    return 0;
}

bool AOZShopManager::CanPurchaseItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType) const
{
    if (!BuyerPS)
        return false;

    int32 Price = GetItemPrice(ItemID, ItemType);

    if (Price <= 0)
        return false;

    if (BuyerPS->OwningScraps < Price)
        return false;

    UOZInventoryComponent* InvComp = BuyerPS->InventoryComp;
    if (!InvComp)
        return false;

    int32 MaxStack = GetItemMaxStack(ItemID, ItemType);
    int32 CurrentQuantity = InvComp->GetTotalItemQuantity(ItemID, ItemType);

    if (CurrentQuantity >= MaxStack)
        return false;

    if (CurrentQuantity == 0 && !InvComp->HasEmptySlot())
        return false;

    // 새로운 배틀아이템 구매 시 3종류 제한 체크
    if (CurrentQuantity == 0 && ItemType == EOZItemType::Battle)
    {
        if (InvComp->GetUniqueBattleItemCount() >= 3)
            return false;
    }

    return true;
}

bool AOZShopManager::CanSellItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType) const
{
    if (!BuyerPS)
        return false;

    UOZInventoryComponent* InvComp = BuyerPS->InventoryComp;
    if (!InvComp)
        return false;

    int32 SlotIndex = InvComp->FindSlotWithItem(ItemID, ItemType);
    if (SlotIndex == INDEX_NONE)
        return false;

    int32 Price = GetItemPrice(ItemID, ItemType);
    if (Price <= 0)
        return false;

    return true;
}

void AOZShopManager::Server_PurchaseItem_Implementation(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
    if (!BuyerPS || Quantity <= 0)
        return;

    UOZInventoryComponent* InvComp = BuyerPS->InventoryComp;
    if (!InvComp)
        return;

    int32 Price = GetItemPrice(ItemID, ItemType);
    if (Price <= 0)
        return;

    int32 MaxStack = GetItemMaxStack(ItemID, ItemType);
    int32 CurrentQuantity = InvComp->GetTotalItemQuantity(ItemID, ItemType);
    int32 MaxPurchasable = MaxStack - CurrentQuantity;

    // 구매 가능한 최대 수량으로 제한
    int32 ActualQuantity = FMath::Min(Quantity, MaxPurchasable);
    if (ActualQuantity <= 0)
        return;

    // 총 가격 계산
    int32 TotalPrice = Price * ActualQuantity;
    if (BuyerPS->OwningScraps < TotalPrice)
        return;

    // 빈 슬롯이 필요한 경우 체크
    if (CurrentQuantity == 0 && !InvComp->HasEmptySlot())
        return;

    bool bAddSuccess = InvComp->AddItem(ItemID, ItemType, ActualQuantity);
    if (!bAddSuccess)
        return;

    BuyerPS->OwningScraps -= TotalPrice;

    OnShopUpdated.Broadcast();
    OnScrapChanged.Broadcast(-1*TotalPrice);
}


void AOZShopManager::Server_SellItem_Implementation(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
    if (!CanSellItem(BuyerPS, ItemID, ItemType))
        return;

    if (Quantity <= 0)
        return;

    UOZInventoryComponent* InvenComp = BuyerPS->InventoryComp;
    if (!InvenComp)
        return;

    int32 SlotIndex = InvenComp->FindSlotWithItem(ItemID, ItemType);
    if (SlotIndex == INDEX_NONE)
        return;

    FOZInventorySlot Slot = InvenComp->GetSlot(SlotIndex);
    int32 ActualQuantity = FMath::Min(Quantity, Slot.Quantity);

    bool bRemoved = InvenComp->RemoveItem(SlotIndex, ActualQuantity);
    if (!bRemoved)
        return;

    int32 Price = GetItemPrice(ItemID, ItemType);
    int32 TotalSellPrice = Price * 0.75 * ActualQuantity;

    BuyerPS->OwningScraps += TotalSellPrice;

    UE_LOG(LogTemp, Log, TEXT("[ShopManager] Item sold. ItemID: %d, Quantity: %d, Total Price: %d"), ItemID, ActualQuantity, TotalSellPrice);

    OnShopUpdated.Broadcast();
    OnScrapChanged.Broadcast(TotalSellPrice);
}

void AOZShopManager::RefreshShop()
{
    // TODO: ���� ���� �� ���� ���� ����
    // ���� ������ ����, ��� ���� ��

    OnShopUpdated.Broadcast();
}