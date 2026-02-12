#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZShopSelectedItem.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZShopSelectedItem : public UOZUI
{
	GENERATED_BODY()

public:
	// 선택된 아이템 정보 설정
	void SetSelectedItem(int32 ItemID, EOZItemType ItemType, const FText& ItemName, const FText& ItemExplain, int32 Price, UTexture2D* Icon);

	// 선택 해제
	void ClearSelection();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SelectedItemName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SelectedItemExplain;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_Purchase;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_SelectedItemPrice;

	UPROPERTY(meta = (BindWidget))
	class UImage* Img_SelectedItemIcon;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* WS_Switcher;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_ItemCountDown;

	UPROPERTY(meta = (BindWidget))
	class UButton* Btn_ItemCountUp;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_ItemCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Txt_Warning;
	

	int32 SelectedItemID = 0;
	EOZItemType SelectedItemType = EOZItemType::None;
	int32 SelectedItemPrice = 0;
	int32 CurrentPurchaseCount = 1;

	UFUNCTION()
	void OnPurchaseButtonClicked();

	UFUNCTION()
	void OnPurchaseItemCountUp();

	UFUNCTION()
	void OnPurchaseItemCountDown();

	void UpdatePurchaseButton();
	void UpdateItemCountDisplay();
	void UpdateWarningState();
	int32 GetMaxPurchasableCount() const;
};