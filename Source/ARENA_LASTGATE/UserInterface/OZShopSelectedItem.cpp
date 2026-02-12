#include "UserInterface/OZShopSelectedItem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerController.h"
#include "Shop/OZShopManager.h"
#include "GameState/OZInGameGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"
#include "Subsystem/OZItemSubsystem.h"

void UOZShopSelectedItem::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Purchase)
	{
		Btn_Purchase->OnClicked.AddDynamic(this, &UOZShopSelectedItem::OnPurchaseButtonClicked);
	}
	if (Btn_ItemCountUp)
	{
		Btn_ItemCountUp->OnClicked.AddDynamic(this, &UOZShopSelectedItem::OnPurchaseItemCountUp);
	}
	if (Btn_ItemCountDown)
	{
		Btn_ItemCountDown->OnClicked.AddDynamic(this, &UOZShopSelectedItem::OnPurchaseItemCountDown);
	}
	

	ClearSelection();
}

void UOZShopSelectedItem::SetSelectedItem(int32 ItemID, EOZItemType ItemType, const FText& ItemName, const FText& ItemExplain, int32 Price, UTexture2D* Icon)
{
	SelectedItemID = ItemID;
	SelectedItemType = ItemType;
	SelectedItemPrice = Price;
	CurrentPurchaseCount = 1;

	if (Txt_SelectedItemName)
	{
		Txt_SelectedItemName->SetText(ItemName);
	}

	if (Txt_SelectedItemExplain)
	{
		FString ExplainString = ItemExplain.ToString();

		if (UOZItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UOZItemSubsystem>())
		{
			if (ItemType == EOZItemType::Buff)
			{
				if (FOZBuffItemData* BuffData = ItemSubsystem->GetBuffItemData(ItemID))
				{
					ExplainString = ExplainString.Replace(TEXT("(Item_Duration)"), *FString::FromInt(BuffData->Item_Duration));
					int32 ValuePercent = FMath::RoundToInt(BuffData->Item_Value * 100.0f);
					ExplainString = ExplainString.Replace(TEXT("(100*Item_Value)"), *FString::FromInt(ValuePercent));
				}
			}
			else if (ItemType == EOZItemType::Battle)
			{
				if (FOZBattleItemData* BattleData = ItemSubsystem->GetBattleItemData(ItemID))
				{
					ExplainString = ExplainString.Replace(TEXT("(Item_Duration)"), *FString::FromInt(BattleData->Item_Duration));
					int32 DamagePercent = FMath::RoundToInt(BattleData->Item_Damage * 100.0f);
					ExplainString = ExplainString.Replace(TEXT("(100*Item_Damage)"), *FString::FromInt(DamagePercent));
					ExplainString = ExplainString.Replace(TEXT("(Damage_Over_Time)"), *FString::Printf(TEXT("%.1f"), BattleData->Damage_Over_Time));
				}
			}
		}

		Txt_SelectedItemExplain->SetText(FText::FromString(ExplainString));
	}

	if (Txt_SelectedItemPrice)
	{
		FText PriceText = UKismetTextLibrary::Conv_IntToText(Price);
		Txt_SelectedItemPrice->SetText(PriceText);
	}

	if (Img_SelectedItemIcon && Icon)
	{
		Img_SelectedItemIcon->SetBrushFromTexture(Icon);
		Img_SelectedItemIcon->SetVisibility(ESlateVisibility::Visible);
	}

	UpdateWarningState();
	UpdateItemCountDisplay();
	UpdatePurchaseButton();

	SetVisibility(ESlateVisibility::Visible);
}

void UOZShopSelectedItem::ClearSelection()
{
	SelectedItemID = 0;
	SelectedItemType = EOZItemType::None;
	SelectedItemPrice = 0;
	CurrentPurchaseCount = 1;

	if (Txt_SelectedItemName)
	{
		Txt_SelectedItemName->SetText(FText::FromString(TEXT("")));
	}

	if (Txt_SelectedItemExplain)
	{
		Txt_SelectedItemExplain->SetText(FText::FromString(TEXT("")));
	}

	if (Txt_SelectedItemPrice)
	{
		Txt_SelectedItemPrice->SetText(FText::FromString(TEXT("")));
	}

	if (Txt_ItemCount)
	{
		Txt_ItemCount->SetText(FText::FromString(TEXT("")));
	}

	if (Btn_Purchase)
	{
		Btn_Purchase->SetIsEnabled(false);
	}

	if (Img_SelectedItemIcon)
	{
		Img_SelectedItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UOZShopSelectedItem::OnPurchaseButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	AOZPlayerController* OZPlayerController = Cast<AOZPlayerController>(PC);
	if (!OZPlayerController)
		return;

	if (SelectedItemID > 0 && SelectedItemType != EOZItemType::None && CurrentPurchaseCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[OZShopSelectedItem] Purchasing Item ID: %d, Type: %d, Quantity: %d, TotalPrice: %d"),
			SelectedItemID, static_cast<int32>(SelectedItemType), CurrentPurchaseCount, SelectedItemPrice * CurrentPurchaseCount);

		OZPlayerController->Server_PurchaseItem(SelectedItemID, SelectedItemType, CurrentPurchaseCount);

		ClearSelection();
	}
}

void UOZShopSelectedItem::OnPurchaseItemCountUp()
{
	int32 MaxPurchasable = GetMaxPurchasableCount();
	if (CurrentPurchaseCount < MaxPurchasable)
	{
		CurrentPurchaseCount++;

		UpdateWarningState();
		UpdateItemCountDisplay();
		UpdatePurchaseButton();
	}
}

void UOZShopSelectedItem::OnPurchaseItemCountDown()
{
	if (CurrentPurchaseCount > 1)
	{
		CurrentPurchaseCount--;

		UpdateWarningState();
		UpdateItemCountDisplay();
		UpdatePurchaseButton();
	}
}

void UOZShopSelectedItem::UpdatePurchaseButton()
{
	if (!Btn_Purchase)
		return;

	AOZInGameGameState* GameState = Cast<AOZInGameGameState>(UGameplayStatics::GetGameState(this));
	if (!GameState)
		return;

	AOZShopManager* ShopManager = GameState->GetShopManager();
	if (!ShopManager)
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	AOZPlayerState* PlayerState = PC->GetPlayerState<AOZPlayerState>();
	if (!PlayerState)
		return;

	UOZInventoryComponent* InvComp = PlayerState->InventoryComp;
	if (!InvComp)
		return;

	bool bInventoryFull = !InvComp->HasEmptySlot();
	int32 ExistingSlot = InvComp->FindSlotWithItem(SelectedItemID, SelectedItemType);
	bool bCanStackToExisting = (ExistingSlot >= 0);

	bool bCanPurchase = false;
	if (SelectedItemID > 0 && SelectedItemType != EOZItemType::None && CurrentPurchaseCount > 0)
	{
		int32 TotalPrice = SelectedItemPrice * CurrentPurchaseCount;

		// 가득 찼고 스택할 수 없으면 구매 불가
		bool bInventoryOk = !bInventoryFull || bCanStackToExisting;
		bCanPurchase = bInventoryOk && PlayerState->OwningScraps >= TotalPrice &&
					   ShopManager->CanPurchaseItem(PlayerState, SelectedItemID, SelectedItemType);
	}

	Btn_Purchase->SetIsEnabled(bCanPurchase);

	if (Btn_ItemCountUp)
	{
		int32 MaxPurchasable = GetMaxPurchasableCount();
		Btn_ItemCountUp->SetIsEnabled(CurrentPurchaseCount < MaxPurchasable);
	}

	if (Btn_ItemCountDown)
	{
		Btn_ItemCountDown->SetIsEnabled(CurrentPurchaseCount > 1);
	}
}

void UOZShopSelectedItem::UpdateItemCountDisplay()
{
	if (Txt_ItemCount)
	{
		FText CountText = UKismetTextLibrary::Conv_IntToText(CurrentPurchaseCount);
		Txt_ItemCount->SetText(CountText);
	}

	if (Txt_SelectedItemPrice)
	{
		int32 TotalPrice = SelectedItemPrice * CurrentPurchaseCount;
		FText PriceText = UKismetTextLibrary::Conv_IntToText(TotalPrice);

		Txt_SelectedItemPrice->SetText(PriceText);
	}
}

void UOZShopSelectedItem::UpdateWarningState()
{
	if (!WS_Switcher)
		return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	AOZPlayerState* PlayerState = PC->GetPlayerState<AOZPlayerState>();
	if (!PlayerState)
		return;

	UOZInventoryComponent* InvComp = PlayerState->InventoryComp;
	if (!InvComp)
		return;

	bool bInventoryFull = !InvComp->HasEmptySlot();
	int32 ExistingSlot = InvComp->FindSlotWithItem(SelectedItemID, SelectedItemType);
	bool bCanStackToExisting = (ExistingSlot >= 0);

	bool bNotEnoughScraps = PlayerState->OwningScraps < SelectedItemPrice;

	// 인벤토리가 가득 찬 경우
	if (bInventoryFull && !bCanStackToExisting)
	{
		// 위젯 스위처를 경고 상태로 전환 (인덱스 1)
		WS_Switcher->SetActiveWidgetIndex(1);

		if (Txt_Warning)
		{
			Txt_Warning->SetText(FText::FromString(TEXT("인벤토리가 가득 차 아이템을 구매하지 못합니다")));
		}
	}
	// 고철이 부족한 경우
	else if (bNotEnoughScraps)
	{
		// 위젯 스위처를 경고 상태로 전환 (인덱스 1)
		WS_Switcher->SetActiveWidgetIndex(1);

		if (Txt_Warning)
		{
			Txt_Warning->SetText(FText::FromString(TEXT("고철이 부족해 아이템을 구매할 수 없습니다")));
		}

		if (Txt_SelectedItemPrice)
		{
			Txt_SelectedItemPrice->SetText(FText::FromString(TEXT("고철 부족")));
		}
	}
	// 구매 가능한 경우
	else
	{
		WS_Switcher->SetActiveWidgetIndex(0);

		if (Txt_SelectedItemPrice)
		{
			int32 TotalPrice = SelectedItemPrice * CurrentPurchaseCount;
			FText PriceText = UKismetTextLibrary::Conv_IntToText(TotalPrice);
			Txt_SelectedItemPrice->SetText(PriceText);
		}
	}
}

int32 UOZShopSelectedItem::GetMaxPurchasableCount() const
{
	AOZInGameGameState* GameState = Cast<AOZInGameGameState>(UGameplayStatics::GetGameState(this));
	if (!GameState)
		return 1;

	AOZShopManager* ShopManager = GameState->GetShopManager();
	if (!ShopManager)
		return 1;

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return 1;

	AOZPlayerState* PlayerState = PC->GetPlayerState<AOZPlayerState>();
	if (!PlayerState)
		return 1;

	UOZInventoryComponent* InvComp = PlayerState->InventoryComp;
	if (!InvComp)
		return 1;

	int32 MaxStack = ShopManager->GetItemMaxStack(SelectedItemID, SelectedItemType);
	if (MaxStack <= 0)
		return 1;

	int32 CurrentQuantity = InvComp->GetTotalItemQuantity(SelectedItemID, SelectedItemType);

	// 구매 가능한 최대 수량 = 최대 스택 수 - 현재 보유량
	int32 MaxPurchasable = MaxStack - CurrentQuantity;

	// 돈으로 살 수 있는 최대 수량도 고려
	if (SelectedItemPrice > 0)
	{
		int32 MaxAffordable = PlayerState->OwningScraps / SelectedItemPrice;
		MaxPurchasable = FMath::Min(MaxPurchasable, MaxAffordable);
	}

	// 최소 1개는 구매 가능하게 음수 방지
	return FMath::Max(1, MaxPurchasable);
}
