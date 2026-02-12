#include "OZShopItemEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/KismetTextLibrary.h"
#include "Styling/SlateBrush.h"

void UOZShopItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemButton)
	{
		ItemButton->OnClicked.AddDynamic(this, &UOZShopItemEntry::OnItemButtonClicked);
	}
}

void UOZShopItemEntry::SetupEntry(int32 InItemID, EOZItemType InItemType, const FText& InName, const FText& InExplanation, UTexture2D* InIcon, int32 InPrice)
{
	ItemID = InItemID;
	ItemType = InItemType;
	ItemName = InName;
	ItemExplain = InExplanation;
	Price = InPrice;
	ItemIcon = InIcon;

	FButtonStyle NewStyle = ItemButton->GetStyle();
	NewStyle.Normal.SetResourceObject(InIcon);
	NewStyle.Hovered.SetResourceObject(InIcon);
	NewStyle.Pressed.SetResourceObject(InIcon);

	ItemButton->SetStyle(NewStyle);
}

void UOZShopItemEntry::OnItemButtonClicked()
{
	OnEntryClicked.Broadcast(ItemID, ItemType, ItemName, ItemExplain, Price, ItemIcon);
}
