#include "UserInterface/OZShopSellPopupUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/KismetTextLibrary.h"

void UOZShopSellPopupUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_ItemCountUp)
	{
		Btn_ItemCountUp->OnClicked.AddDynamic(this, &UOZShopSellPopupUI::OnCountUpClicked);
	}

	if (Btn_ItemCountDown)
	{
		Btn_ItemCountDown->OnClicked.AddDynamic(this, &UOZShopSellPopupUI::OnCountDownClicked);
	}

	if (Btn_Sell)
	{
		Btn_Sell->OnClicked.AddDynamic(this, &UOZShopSellPopupUI::OnSellClicked);
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UOZShopSellPopupUI::OnCloseClicked);
	}

	ResetPopup();
}

void UOZShopSellPopupUI::SetItemData(int32 InItemID, EOZItemType InItemType, UTexture2D* InIcon, int32 InMaxQuantity)
{
	ItemID = InItemID;
	ItemType = InItemType;
	MaxCount = InMaxQuantity;
	CurrentCount = 1;

	if (Img_ItemIcon && InIcon)
	{
		Img_ItemIcon->SetBrushFromTexture(InIcon);
	}

	UpdateCountDisplay();
}

void UOZShopSellPopupUI::ResetPopup()
{
	ItemID = 0;
	ItemType = EOZItemType::None;
	CurrentCount = 1;
	MaxCount = 1;

	if (Img_ItemIcon)
	{
		Img_ItemIcon->SetBrushFromTexture(nullptr);
	}

	UpdateCountDisplay();
}

void UOZShopSellPopupUI::OnCountUpClicked()
{
	if (CurrentCount < MaxCount)
	{
		CurrentCount++;
		UpdateCountDisplay();
	}
}

void UOZShopSellPopupUI::OnCountDownClicked()
{
	if (CurrentCount > 1)
	{
		CurrentCount--;
		UpdateCountDisplay();
	}
}

void UOZShopSellPopupUI::OnSellClicked()
{
	if (ItemID > 0 && ItemType != EOZItemType::None && CurrentCount > 0)
	{
		OnItemSold.Broadcast(ItemID, ItemType, CurrentCount);
		ResetPopup();
		RemoveFromParent();
	}
}

void UOZShopSellPopupUI::OnCloseClicked()
{
	ResetPopup();
	RemoveFromParent();
}

void UOZShopSellPopupUI::UpdateCountDisplay()
{
	if (Txt_ItemCount)
	{
		FText CountText = UKismetTextLibrary::Conv_IntToText(CurrentCount);
		Txt_ItemCount->SetText(CountText);
	}
}
