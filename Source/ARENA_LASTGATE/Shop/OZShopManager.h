#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZShopManager.generated.h"

class AOZPlayerState;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrapChanged, int, ChangedAmount);

UCLASS()
class ARENA_LASTGATE_API AOZShopManager : public AActor
{
    GENERATED_BODY()

public:
    AOZShopManager();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(BlueprintAssignable)
    FOnShopUpdated OnShopUpdated;

    UPROPERTY(BlueprintAssignable)
    FOnScrapChanged OnScrapChanged;

    UFUNCTION(BlueprintPure, Category = "Shop")
    bool CanPurchaseItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    bool CanSellItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetItemPrice(int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Shop")
    void Server_PurchaseItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType, int32 Quantity = 1);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Shop")
    void Server_SellItem(AOZPlayerState* BuyerPS, int32 ItemID, EOZItemType ItemType, int32 Quantity = 1);

    UFUNCTION(BlueprintPure, Category = "Shop")
    int32 GetItemMaxStack(int32 ItemID, EOZItemType ItemType) const;

    UFUNCTION(BlueprintCallable, Category = "Shop")
    void RefreshShop();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    class UOZItemSubsystem* ItemSubSystem;

    UOZItemSubsystem* GetItemSubSystem() const;
};