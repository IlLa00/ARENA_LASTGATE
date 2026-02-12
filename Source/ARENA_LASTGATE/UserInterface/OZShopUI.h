#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZShopUI.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZShopUI : public UOZUI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshInventoryUI();

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshShopUI();

	UFUNCTION(BlueprintImplementableEvent)
	void PopupChangedAmount(int changedAmount);

	// UI가 보일 때마다 스크랩 갱신
	virtual void SetVisibility(ESlateVisibility InVisibility) override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Sell;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Exit;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_CurrentScrap;

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* Grid_BattleItemList;

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* Grid_BuffItemList;

	UPROPERTY(meta = (BindWidget))
	class UOZShopSelectedItem* ShopSelectedItemWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Module;

	UPROPERTY()
	class AOZPlayerState* PlayerState;

	UPROPERTY()
	class AOZInGameGameState* GameState;

	UPROPERTY()
	class AOZShopManager* ShopManager;

	UPROPERTY()
	class UOZInventoryComponent* InventoryComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop|Data", meta = (AllowPrivateAccess = "true"))
	UDataTable* BattleItemDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop|Data", meta = (AllowPrivateAccess = "true"))
	UDataTable* BuffItemDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop|Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UOZShopItemEntry> ShopItemEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shop|Module", meta = (AllowPrivateAccess = "true"))
	int32 ModuleCost = 200;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop|Module", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> ModuleWidgetClass;

	UPROPERTY()
	class UUserWidget* ModuleWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shop|Sell", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UOZShopSellPopupUI> SellPopupWidgetClass;

	UPROPERTY()
	class UOZShopSellPopupUI* SellPopupWidgetInstance = nullptr;

	UPROPERTY()
	int32 SelectedItemID = 0;

	UPROPERTY()
	EOZItemType SelectedItemType = EOZItemType::None;

	UPROPERTY()
	int32 SelectedItemPrice = 0;

	UPROPERTY()
	class UOZShopItemEntry* SelectedShopWidget = nullptr;

	UPROPERTY()
	int32 SelectedInvenSlotIndex = -1;

	UPROPERTY()
	class UOZInvenEntry* SelectedInvenWidget = nullptr;

	UPROPERTY()
	int32 SelectedInvenItemID = 0;

	UPROPERTY()
	EOZItemType SelectedInvenItemType = EOZItemType::None;

	UPROPERTY()
	UTexture2D* SelectedInvenItemIcon = nullptr;

	UPROPERTY()
	int32 SelectedInvenItemQuantity = 0;

	UPROPERTY()
	class UOZInGameFloorUI* InGameFloorUI = nullptr;

	bool bPlayerStateInitialized = false;
	bool bInventoryEventsConnected = false;

	UFUNCTION()
	void OnPurchaseButtonClicked();

	UFUNCTION()
	void OnSellButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnModuleButtonClicked();

	UFUNCTION()
	void HandleShopItemSelected(int32 ItemID, EOZItemType ItemType, FText ItemName, FText ItemExplain, int32 Price, UTexture2D* Icon);

	UFUNCTION()
	void HandleInvenItemSelected(int32 SlotIndex);

	UFUNCTION()
	void HandleInvenEntryClicked(int32 SlotIndex, int32 ItemID, EOZItemType ItemType, UTexture2D* Icon, int32 Quantity);

	UFUNCTION()
	void HandleItemSold(int32 ItemID, EOZItemType ItemType, int32 Quantity);

	UFUNCTION()
	void OnShopUpdated();

	UFUNCTION()
	void OnScrapChanged(int changedAmount);

	UFUNCTION()
	void OnInventoryUpdated();

	void PopulateShopItems();
	void UpdatePurchaseButton();
	void UpdateSellButton();
	void UpdateCurrencyDisplay();
	void ClearShopSelection();
};
