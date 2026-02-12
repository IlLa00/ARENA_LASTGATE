#include "OZInvenEntry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"
#include "Styling/SlateBrush.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "OZInventoryDragDropOperation.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"

void UOZInvenEntry::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Visible);

	if (Btn_Icon)
	{
		// 버튼은 시각적으로만 사용하고, 마우스 이벤트는 위젯에서 처리
		Btn_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UOZInvenEntry::SetupEntry(const FText& InName, UTexture2D* InIcon, int32 InQuantity, int32 InSlotIndex, int32 InItemID, EOZItemType InItemType)
{
	ItemIcon = InIcon;
	ItemQuantity = InQuantity;
	SlotIndex = InSlotIndex;
	ItemID = InItemID;
	ItemType = InItemType;

	// 수량 텍스트 설정
	if (Txt_Quantity)
	{
		if (ItemQuantity > 0)
		{
			FText QuantityText = UKismetTextLibrary::Conv_IntToText(ItemQuantity);
			Txt_Quantity->SetText(QuantityText);
			Txt_Quantity->SetRenderOpacity(1.0f);
		}
		else
		{
			// 수량이 0이면 텍스트를 투명하게 처리
			Txt_Quantity->SetText(FText::GetEmpty());
			Txt_Quantity->SetRenderOpacity(0.0f);
		}
	}

	if (Btn_Icon)
	{
		FButtonStyle ButtonStyle = Btn_Icon->GetStyle();
		FSlateBrush Brush;

		// 아이템 아이콘이 있으면 사용, 없으면 기본 슬롯 아이콘 사용
		UTexture2D* IconToUse = ItemIcon ? ItemIcon : DefaultSlotIcon;

		if (IconToUse)
		{
			Brush.SetResourceObject(IconToUse);
			Brush.ImageSize = FVector2D(64.0f, 64.0f);
			Brush.DrawAs = ESlateBrushDrawType::Image;
		}
		else
		{
			// 아이콘이 전혀 없으면 비워둠
			Brush.SetResourceObject(nullptr);
			Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
		}

		ButtonStyle.Normal = Brush;
		ButtonStyle.Hovered = Brush;
		ButtonStyle.Pressed = Brush;

		Btn_Icon->SetStyle(ButtonStyle);
	}
}

void UOZInvenEntry::ClearEntry()
{
	ItemIcon = nullptr;
	ItemQuantity = 0;
	// SlotIndex는 UI 위치를 나타내므로 유지
	ItemID = 0;
	ItemType = EOZItemType::None;

	// 수량 텍스트를 투명하게 처리
	if (Txt_Quantity)
	{
		Txt_Quantity->SetText(FText::GetEmpty());
		Txt_Quantity->SetRenderOpacity(0.0f);
	}

	if (Btn_Icon)
	{
		FButtonStyle ButtonStyle = Btn_Icon->GetStyle();
		FSlateBrush Brush;

		// 기본 슬롯 아이콘이 있으면 사용
		if (DefaultSlotIcon)
		{
			Brush.SetResourceObject(DefaultSlotIcon);
			Brush.ImageSize = FVector2D(64.0f, 64.0f);
			Brush.DrawAs = ESlateBrushDrawType::Image;
		}
		else
		{
			// 기본 아이콘도 없으면 비워둠
			Brush.SetResourceObject(nullptr);
			Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
		}

		ButtonStyle.Normal = Brush;
		ButtonStyle.Hovered = Brush;
		ButtonStyle.Pressed = Brush;

		Btn_Icon->SetStyle(ButtonStyle);
	}
}

void UOZInvenEntry::RestoreOpacity()
{
	if (Btn_Icon)
	{
		Btn_Icon->SetRenderOpacity(1.0f);
	}
	if (Txt_Quantity)
	{
		// 수량이 0이 아닐 때만 투명도 복원
		if (ItemQuantity > 0)
		{
			Txt_Quantity->SetRenderOpacity(1.0f);
		}
		else
		{
			Txt_Quantity->SetRenderOpacity(0.0f);
		}
	}
}

FReply UOZInvenEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && ItemID > 0 && ItemType != EOZItemType::None)
	{
		bIsPressed = true;
		Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Reply;
}

FReply UOZInvenEntry::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (bIsPressed && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = false;

		if (SlotIndex >= 0 && ItemID > 0 && ItemType != EOZItemType::None)
		{
			OnEntryClicked.Broadcast(SlotIndex, ItemID, ItemType, ItemIcon, ItemQuantity);
		}
	}

	return Reply;
}

void UOZInvenEntry::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	bIsPressed = false;

	UOZInventoryDragDropOperation* DragDropOp = NewObject<UOZInventoryDragDropOperation>();
	if (DragDropOp)
	{
		DragDropOp->DraggedSlotIndex = SlotIndex;
		DragDropOp->DraggedItemID = ItemID;
		DragDropOp->DraggedItemType = ItemType;
		DragDropOp->DraggedQuantity = ItemQuantity;
		DragDropOp->DraggedIcon = ItemIcon;
		DragDropOp->SourceWidget = this;

		UImage* DragVisual = NewObject<UImage>(this);
		if (DragVisual && ItemIcon)
		{
			DragVisual->SetBrushFromTexture(ItemIcon);
			// DragVisual->SetBrushSize(FVector2D(64.0f, 64.0f));
			DragVisual->SetOpacity(0.75f);

			DragDropOp->DefaultDragVisual = DragVisual;
			DragDropOp->Pivot = EDragPivot::MouseDown;
		}

		// 드래그 중 원본 아이템을 투명하게 표시
		if (Btn_Icon)
		{
			Btn_Icon->SetRenderOpacity(0.5f);
		}
		if (Txt_Quantity && ItemQuantity > 0)
		{
			Txt_Quantity->SetRenderOpacity(0.5f);
		}

		OutOperation = DragDropOp;
	}
}

bool UOZInvenEntry::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	UOZInventoryDragDropOperation* DragDropOp = Cast<UOZInventoryDragDropOperation>(InOperation);
	if (!DragDropOp)
		return false;

	// 같은 곳이면 무시
	if (DragDropOp->DraggedSlotIndex == SlotIndex)
	{
		if (DragDropOp->SourceWidget)
		{
			DragDropOp->SourceWidget->RestoreOpacity();
		}
		return false;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
		return false;

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(PC->GetCharacter());
	if (!OZPlayer)
		return false;

	AOZPlayerState* OZPlayerState = Cast<AOZPlayerState>(OZPlayer->GetPlayerState());
	if (!OZPlayerState)
		return false;

	UOZInventoryComponent* InventoryComp = OZPlayerState->InventoryComp;
	if (!InventoryComp)
		return false;

	InventoryComp->SwapSlots(DragDropOp->DraggedSlotIndex, SlotIndex);

	if (DragDropOp->SourceWidget)
	{
		DragDropOp->SourceWidget->RestoreOpacity();
	}

	return true;
}

void UOZInvenEntry::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	RestoreOpacity();
}

