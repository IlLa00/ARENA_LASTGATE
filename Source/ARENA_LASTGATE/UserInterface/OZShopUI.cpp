#include "OZShopUI.h"
#include "OZShopItemEntry.h"
#include "OZInvenEntry.h"
#include "OZShopSelectedItem.h"
#include "OZShopSellPopupUI.h"
#include "OZInGameFloorUI.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Character/OZPlayerState.h"
#include "GameState/OZInGameGameState.h"
#include "Shop/OZShopManager.h"
#include "Data/OZBattleItemData.h"
#include "HUD/OZInGameHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"

void UOZShopUI::SetVisibility(ESlateVisibility InVisibility)
{
	Super::SetVisibility(InVisibility);

	if (InVisibility == ESlateVisibility::Visible || InVisibility == ESlateVisibility::SelfHitTestInvisible)
	{
		if (!PlayerState)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				PlayerState = PC->GetPlayerState<AOZPlayerState>();
			}
		}

		UpdateCurrencyDisplay();
	}
}

void UOZShopUI::NativeConstruct()
{
	Super::NativeConstruct();

	GameState = Cast<AOZInGameGameState>(UGameplayStatics::GetGameState(this));
	if (GameState)
	{
		ShopManager = GameState->GetShopManager();
		if (ShopManager)
		{
			ShopManager->OnShopUpdated.AddDynamic(this, &UOZShopUI::OnShopUpdated);
			ShopManager->OnScrapChanged.AddDynamic(this, &UOZShopUI::OnScrapChanged);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OZShopUI] GameState is NULL!"));
	}

	if (Btn_Sell)
	{
		Btn_Sell->OnClicked.AddDynamic(this, &UOZShopUI::OnSellButtonClicked);
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UOZShopUI::OnExitButtonClicked);
	}

	if (Btn_Module)
	{
		Btn_Module->OnClicked.AddDynamic(this, &UOZShopUI::OnModuleButtonClicked);
	}
	


	UOZItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UOZItemSubsystem>();
	if (ItemSubsystem)
	{
		BattleItemDataTable = ItemSubsystem->GetBattleItemDataTable();
		BuffItemDataTable = ItemSubsystem->GetBuffItemDataTable();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OZShopUI] ItemSubsystem is NULL!"));
	}

	PopulateShopItems();
	RefreshInventoryUI();
	UpdateCurrencyDisplay();
	UpdatePurchaseButton();
	UpdateSellButton();
}

void UOZShopUI::NativeDestruct()
{
	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UOZShopUI::OnInventoryUpdated);
	}

	if (ShopManager)
	{
		ShopManager->OnShopUpdated.RemoveDynamic(this, &UOZShopUI::OnShopUpdated);
		ShopManager->OnScrapChanged.RemoveDynamic(this, &UOZShopUI::OnScrapChanged);
	}

	// 인벤토리 엔트리 이벤트 정리
	if (InGameFloorUI)
	{
		TArray<UOZInvenEntry*> InvenEntries = {
			InGameFloorUI->WBP_InvenEntry_1,
			InGameFloorUI->WBP_InvenEntry_2,
			InGameFloorUI->WBP_InvenEntry_3,
			InGameFloorUI->WBP_InvenEntry_4
		};

		for (UOZInvenEntry* Entry : InvenEntries)
		{
			if (Entry)
			{
				Entry->OnEntryClicked.RemoveDynamic(this, &UOZShopUI::HandleInvenEntryClicked);
			}
		}
	}

	// 모듈 위젯 정리
	if (ModuleWidgetInstance)
	{
		ModuleWidgetInstance->RemoveFromParent();
		ModuleWidgetInstance = nullptr;
	}

	// 판매 팝업 위젯 정리
	if (SellPopupWidgetInstance)
	{
		SellPopupWidgetInstance->RemoveFromParent();
		SellPopupWidgetInstance = nullptr;
	}

	Super::NativeDestruct();
}

void UOZShopUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bPlayerStateInitialized)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			PlayerState = PC->GetPlayerState<AOZPlayerState>();
			if (PlayerState)
			{
				UE_LOG(LogTemp, Log, TEXT("[OZShopUI] PlayerState found! OwningScraps: %d"), PlayerState->OwningScraps);

				InventoryComp = PlayerState->FindComponentByClass<UOZInventoryComponent>();
				if (InventoryComp)
				{
					InventoryComp->OnInventoryUpdated.AddDynamic(this, &UOZShopUI::OnInventoryUpdated);
					UE_LOG(LogTemp, Log, TEXT("[OZShopUI] InventoryComp delegate bound!"));
				}

				RefreshInventoryUI();
				UpdateCurrencyDisplay();
				UpdatePurchaseButton();
				UpdateSellButton();

				// 초기화 완료 플래그
				bPlayerStateInitialized = true;
			}
			else
			{
				static int32 LogCount = 0;
				if (LogCount++ % 60 == 0) 
				{
					UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] Waiting for PlayerState... (%.1f seconds)"), LogCount / 60.0f);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[OZShopUI] PlayerController is NULL!"));
		}
	}
}

void UOZShopUI::OnPurchaseButtonClicked()
{
	if (!ShopManager || !PlayerState)
		return;

	if (SelectedItemID > 0 && SelectedItemType != EOZItemType::None)
	{
		ShopManager->Server_PurchaseItem(PlayerState, SelectedItemID, SelectedItemType);
		ClearShopSelection();
	}
}

void UOZShopUI::OnSellButtonClicked()
{
	if (!SellPopupWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] SellPopupWidgetClass is not set!"));
		return;
	}

	if (!SellPopupWidgetInstance)
	{
		SellPopupWidgetInstance = CreateWidget<UOZShopSellPopupUI>(this, SellPopupWidgetClass);
		if (SellPopupWidgetInstance)
		{
			SellPopupWidgetInstance->OnItemSold.AddDynamic(this, &UOZShopUI::HandleItemSold);
		}
	}

	if (SellPopupWidgetInstance)
	{
		SellPopupWidgetInstance->AddToViewport(10);
		UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Sell popup opened."));
	}
}

void UOZShopUI::OnExitButtonClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return;

	AHUD* hud = PC->GetHUD();
	if (!hud)
		return;

	if (AOZInGameHUD* inGameHUD = Cast<AOZInGameHUD>(hud))
	{
		AOZInGameGameState* InGameState = GetWorld()->GetGameState<AOZInGameGameState>();

		if (InGameState == nullptr)
			return;

		if (InGameState->CurrentRoundState == EGameStateType::STATE_PREP)
		{
			inGameHUD->ToggleShopUI();
		}
	}
}

void UOZShopUI::OnModuleButtonClicked()
{
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] PlayerState is NULL! Cannot open module."));
		return;
	}

	if (PlayerState->OwningScraps < ModuleCost)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] Not enough scraps! Need %d, have %d"), ModuleCost, PlayerState->OwningScraps);
		return;
	}

	if (!ModuleWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] ModuleWidgetClass is not set!"));
		return;
	}

	PlayerState->OwningScraps -= ModuleCost;
	UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Module opened! Cost: %d, Remaining scraps: %d"), ModuleCost, PlayerState->OwningScraps);

	// 재화 UI 갱신
	UpdateCurrencyDisplay();

	// 위젯 생성 및 표시
	if (!ModuleWidgetInstance)
	{
		ModuleWidgetInstance = CreateWidget<UUserWidget>(this, ModuleWidgetClass);
		ModuleWidgetInstance->AddToViewport();
	}

	if (ModuleWidgetInstance)
	{
		ModuleWidgetInstance->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Module widget displayed."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OZShopUI] Failed to create module widget!"));
	}
}

void UOZShopUI::HandleShopItemSelected(int32 ItemID, EOZItemType ItemType, FText ItemName, FText ItemExplain, int32 Price, UTexture2D* Icon)
{
	// ShopSelectedItemWidget에 선택된 아이템 정보 전달
	if (ShopSelectedItemWidget)
	{
		ShopSelectedItemWidget->SetSelectedItem(ItemID, ItemType, ItemName, ItemExplain, Price, Icon);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] ShopSelectedItemWidget is NULL!"));
	}
}

void UOZShopUI::HandleInvenItemSelected(int32 SlotIndex)
{
	SelectedInvenSlotIndex = SlotIndex;

	ClearShopSelection();
	UpdateSellButton();
}

void UOZShopUI::HandleInvenEntryClicked(int32 SlotIndex, int32 ItemID, EOZItemType ItemType, UTexture2D* Icon, int32 Quantity)
{
	SelectedInvenSlotIndex = SlotIndex;
	SelectedInvenItemID = ItemID;
	SelectedInvenItemType = ItemType;
	SelectedInvenItemIcon = Icon;
	SelectedInvenItemQuantity = Quantity;

	ClearShopSelection();
	UpdateSellButton();

	if (SellPopupWidgetInstance)
	{
		SellPopupWidgetInstance->SetItemData(ItemID, ItemType, Icon, Quantity);
		UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Item data set in sell popup. ItemID: %d, Quantity: %d"), ItemID, Quantity);
	}
}

void UOZShopUI::HandleItemSold(int32 ItemID, EOZItemType ItemType, int32 Quantity)
{
	if (!ShopManager || !PlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] Cannot sell item - ShopManager or PlayerState is null"));
		return;
	}

	ShopManager->Server_SellItem(PlayerState, ItemID, ItemType, Quantity);
	UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Item sold. ItemID: %d, Type: %d, Quantity: %d"), ItemID, (int32)ItemType, Quantity);

	SelectedInvenSlotIndex = -1;
	SelectedInvenItemID = 0;
	SelectedInvenItemType = EOZItemType::None;
	SelectedInvenItemIcon = nullptr;
	SelectedInvenItemQuantity = 0;
}

void UOZShopUI::OnShopUpdated()
{
	RefreshShopUI();
}

void UOZShopUI::OnScrapChanged(int changedAmount)
{
	PopupChangedAmount(changedAmount);
}

void UOZShopUI::OnInventoryUpdated()
{
	RefreshInventoryUI();
	UpdateCurrencyDisplay();
	UpdatePurchaseButton();
	UpdateSellButton();
}

void UOZShopUI::PopulateShopItems()
{
	const int32 ItemsPerRow = 5; // 한 줄에 표시할 아이템 개수

	if (Grid_BattleItemList && BattleItemDataTable)
	{
		Grid_BattleItemList->ClearChildren();

		TSet<int32> AddedBattleItemIDs;
		TArray<FName> RowNames = BattleItemDataTable->GetRowNames();
		int32 Index = 0;
		for (const FName& RowName : RowNames)
		{
			FOZBattleItemData* ItemData = BattleItemDataTable->FindRow<FOZBattleItemData>(RowName, TEXT(""));
			if (ItemData && ShopItemEntryClass)
			{
				if (AddedBattleItemIDs.Contains(ItemData->Item_ID))
					continue;

				AddedBattleItemIDs.Add(ItemData->Item_ID);

				UOZShopItemEntry* Entry = CreateWidget<UOZShopItemEntry>(this, ShopItemEntryClass);
				if (Entry)
				{
					UTexture2D* Icon = ItemData->ItemIcon.LoadSynchronous();
					Entry->SetupEntry(ItemData->Item_ID, EOZItemType::Battle, ItemData->Item_Name, ItemData->Explanation, Icon, ItemData->Item_Cost);
					Entry->OnEntryClicked.AddDynamic(this, &UOZShopUI::HandleShopItemSelected);

					int32 Row = Index / ItemsPerRow;
					int32 Column = Index % ItemsPerRow;
					Grid_BattleItemList->AddChildToUniformGrid(Entry, Row, Column);
					Index++;
				}
			}
		}
	}

	if (Grid_BuffItemList && BuffItemDataTable)
	{
		Grid_BuffItemList->ClearChildren();

		TSet<int32> AddedBuffItemIDs;
		TArray<FName> RowNames = BuffItemDataTable->GetRowNames();
		int32 Index = 0;
		for (const FName& RowName : RowNames)
		{
			FOZBuffItemData* ItemData = BuffItemDataTable->FindRow<FOZBuffItemData>(RowName, TEXT(""));

			if (ItemData && ShopItemEntryClass)
			{
				if (AddedBuffItemIDs.Contains(ItemData->Item_ID))
					continue;

				AddedBuffItemIDs.Add(ItemData->Item_ID);

				UOZShopItemEntry* Entry = CreateWidget<UOZShopItemEntry>(this, ShopItemEntryClass);
				if (Entry)
				{
					UTexture2D* Icon = ItemData->ItemIcon.LoadSynchronous();
					Entry->SetupEntry(ItemData->Item_ID, EOZItemType::Buff, ItemData->Item_Name, ItemData->Explanation, Icon, ItemData->Item_Cost);
					Entry->OnEntryClicked.AddDynamic(this, &UOZShopUI::HandleShopItemSelected);

					int32 Row = Index / ItemsPerRow;
					int32 Column = Index % ItemsPerRow;
					Grid_BuffItemList->AddChildToUniformGrid(Entry, Row, Column);
					Index++;
				}
			}
		}
	}
}

void UOZShopUI::RefreshInventoryUI()
{
	if (bInventoryEventsConnected)
		return;

	if (!InGameFloorUI)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC && PC->GetHUD())
		{
			AOZInGameHUD* InGameHUD = Cast<AOZInGameHUD>(PC->GetHUD());
			if (InGameHUD)
			{
				InGameFloorUI = InGameHUD->GetInGameFloorUI();
			}
		}
	}

	if (!InGameFloorUI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZShopUI] InGameFloorUI not found!"));
		return;
	}

	TArray<UOZInvenEntry*> InvenEntries = {
		InGameFloorUI->WBP_InvenEntry_1,
		InGameFloorUI->WBP_InvenEntry_2,
		InGameFloorUI->WBP_InvenEntry_3,
		InGameFloorUI->WBP_InvenEntry_4
	};

	for (UOZInvenEntry* Entry : InvenEntries)
	{
		if (Entry)
		{
			Entry->OnEntryClicked.AddDynamic(this, &UOZShopUI::HandleInvenEntryClicked);
		}
	}

	bInventoryEventsConnected = true;
	UE_LOG(LogTemp, Log, TEXT("[OZShopUI] Inventory events connected to InGameFloorUI entries."));
}

void UOZShopUI::RefreshShopUI()
{
	PopulateShopItems();
	UpdateCurrencyDisplay();
	UpdatePurchaseButton();
}

void UOZShopUI::UpdatePurchaseButton()
{
	if (!ShopManager || !PlayerState)
		return;

	bool bCanPurchase = false;

	if (SelectedItemID > 0 && SelectedItemType != EOZItemType::None)
	{
		bCanPurchase = ShopManager->CanPurchaseItem(PlayerState, SelectedItemID, SelectedItemType);
	}

}

void UOZShopUI::UpdateSellButton()
{
	if (!Btn_Sell || !ShopManager || !PlayerState || !InventoryComp)
		return;

	bool bCanSell = true;

	if (SelectedInvenSlotIndex >= 0)
	{
		FOZInventorySlot InvenSlot = InventoryComp->GetSlot(SelectedInvenSlotIndex);
		if (!InvenSlot.IsEmpty())
		{
			bCanSell = ShopManager->CanSellItem(PlayerState, InvenSlot.ItemID, InvenSlot.ItemType);
		}
	}

	Btn_Sell->SetIsEnabled(bCanSell);
}

void UOZShopUI::UpdateCurrencyDisplay()
{
	if (!Txt_CurrentScrap || !PlayerState)
		return;

	FText CurrencyText = UKismetTextLibrary::Conv_IntToText(PlayerState->OwningScraps);
	Txt_CurrentScrap->SetText(CurrencyText);
}

void UOZShopUI::ClearShopSelection()
{
	SelectedItemID = 0;
	SelectedItemType = EOZItemType::None;
	SelectedItemPrice = 0;
	SelectedShopWidget = nullptr;

	UpdatePurchaseButton();
}

