#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZShopSellPopupUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnItemSold, int32, ItemID, EOZItemType, ItemType, int32, Quantity);

UCLASS()
class ARENA_LASTGATE_API UOZShopSellPopupUI : public UOZUI
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Txt_ItemCount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButton> Btn_ItemCountDown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButton> Btn_ItemCountUp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Img_ItemIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Sell;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UButton> Btn_Close;

	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnItemSold OnItemSold;

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetItemData(int32 InItemID, EOZItemType InItemType, UTexture2D* InIcon, int32 InMaxQuantity);

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ResetPopup();

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	EOZItemType ItemType = EOZItemType::None;

	UPROPERTY()
	int32 CurrentCount = 1;

	UPROPERTY()
	int32 MaxCount = 1;

	UFUNCTION()
	void OnCountUpClicked();

	UFUNCTION()
	void OnCountDownClicked();

	UFUNCTION()
	void OnSellClicked();

	UFUNCTION()
	void OnCloseClicked();

	void UpdateCountDisplay();
};
