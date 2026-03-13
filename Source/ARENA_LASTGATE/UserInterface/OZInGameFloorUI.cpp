#include "UserInterface/OZInGameFloorUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "HUD/OZInGameHUD.h"
#include "UserInterface/OZInvenEntry.h"
#include "UserInterface/OZBuffProgressEntry.h"
#include "Widget/OZTutorialPopupTextWidget.h"
#include "ViewModel/OZFloorViewModel.h"
#include "ViewModel/OZViewModelTypes.h"
#include "Character/OZPlayerState.h"

void UOZInGameFloorUI::NativeConstruct()
{
	Super::NativeConstruct();

	ConvertBGSlots = { img_BG_Convert_1, img_BG_Convert_2, img_BG_Convert_3,
					   img_BG_Convert_4, img_BG_Convert_5, img_BG_Convert_6 };
	ConvertIconSlots = { img_Icon_Convert_1, img_Icon_Convert_2, img_Icon_Convert_3,
						 img_Icon_Convert_4, img_Icon_Convert_5, img_Icon_Convert_6 };

	InitializeViewModel();
}

void UOZInGameFloorUI::NativeDestruct()
{
	if (ViewModel)
	{
		ViewModel->OnFloatPropertyChanged.RemoveDynamic(this, &UOZInGameFloorUI::OnFloatPropertyUpdated);
		ViewModel->OnInventoryDataChanged.RemoveAll(this);
		ViewModel->OnConvertDataChanged.RemoveAll(this);
		ViewModel->OnNewBuffDetected.RemoveAll(this);
		ViewModel->OnStealthChanged.RemoveAll(this);
		ViewModel->Deinitialize();
		ViewModel = nullptr;
	}

	for (UOZBuffProgressEntry* Entry : ActiveBuffEntries)
	{
		if (Entry)
		{
			Entry->RemoveFromParent();
		}
	}
	ActiveBuffEntries.Empty();

	Super::NativeDestruct();
}

void UOZInGameFloorUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ViewModel && ViewModel->IsInitialized())
	{
		ViewModel->PollStealthState();
		ViewModel->UpdateBuffTimers();
	}
}

void UOZInGameFloorUI::InitializeViewModel()
{
	ViewModel = NewObject<UOZFloorViewModel>(this);

	ViewModel->OnFloatPropertyChanged.AddDynamic(this, &UOZInGameFloorUI::OnFloatPropertyUpdated);
	ViewModel->OnInventoryDataChanged.AddUObject(this, &UOZInGameFloorUI::OnInventoryDisplayUpdated);
	ViewModel->OnConvertDataChanged.AddUObject(this, &UOZInGameFloorUI::OnConvertDisplayUpdated);
	ViewModel->OnNewBuffDetected.AddUObject(this, &UOZInGameFloorUI::OnNewBuffDisplayed);
	ViewModel->OnStealthChanged.AddUObject(this, &UOZInGameFloorUI::OnStealthStateUpdated);

	ViewModel->Initialize(GetOwningPlayer());
}

void UOZInGameFloorUI::SetCurrentRound(uint8 currRound)
{
	if (Text_Upper == nullptr)
		return;

	Text_Upper->SetText(FText::AsNumber(currRound));
}

void UOZInGameFloorUI::SetCurrentGameState(EGameStateType currGameState, int numRound)
{
	if (Text_Upper == nullptr)
		return;

	FText currentState;

	switch (currGameState)
	{
	case EGameStateType::STATE_PREP:
		currentState = FText::FromString(TEXT("정비"));
		break;
	case EGameStateType::STATE_COMBAT:
		currentState = FText::FromString(FString::Printf(TEXT("%d라운드"), numRound));
		break;
	case EGameStateType::STATE_RESULT:
		currentState = FText::FromString(TEXT("전투결과"));
		break;
	case EGameStateType::STATE_GAMEEND:
		currentState = FText::FromString(TEXT("=="));
		break;
	case EGameStateType::None:
	case EGameStateType::MAX:
	default:
		currentState = FText::FromString(TEXT("NULL"));
		break;
	}

	Text_Upper->SetText(currentState);
}

void UOZInGameFloorUI::SetCurrentRemainTime(int32 remainTime)
{
	if (Text_Timer == nullptr)
		return;

	const FString TimeString = FString::Printf(TEXT("%02d"), remainTime);
	Text_Timer->SetText(FText::FromString(TimeString));
}

void UOZInGameFloorUI::SetCombatTimer(float remainTime)
{
	if (Text_Timer == nullptr)
		return;

	const FString TimeString = FString::Printf(TEXT("%05.2f"), remainTime);
	Text_Timer->SetText(FText::FromString(TimeString));
}

void UOZInGameFloorUI::SetIsOnCutScene(bool isOnCutScene)
{
	float targetRenderOpacity = isOnCutScene ? 0.0f : 1.0f;

	HorizontalBox_TimeDisplaySlot->SetRenderOpacity(targetRenderOpacity);
	Overlay_UserInfoSlot->SetRenderOpacity(targetRenderOpacity);
	Overlay_InventorySlot->SetRenderOpacity(targetRenderOpacity);
	HorizontalBox_BuffSlotUpper->SetRenderOpacity(targetRenderOpacity);
	HorizontalBox_BuffSlotLower->SetRenderOpacity(targetRenderOpacity);
}

void UOZInGameFloorUI::OnInventoryUpdated()
{
	RefreshInventory();
}

void UOZInGameFloorUI::RefreshInventory()
{
	if (ViewModel)
	{
		ViewModel->RequestInventoryRefresh();
	}
}

void UOZInGameFloorUI::SetActiveInvenEntries(bool state)
{
	TArray<UOZInvenEntry*> InvenEntries = {
		WBP_InvenEntry_1,
		WBP_InvenEntry_2,
		WBP_InvenEntry_3,
		WBP_InvenEntry_4
	};

	for (UOZInvenEntry* Entry : InvenEntries)
	{
		if (Entry)
		{
			Entry->SetIsEnabled(state);
		}
	}
}

void UOZInGameFloorUI::ChangeFloorUI(bool bIsWin)
{
	ChangeSwitcher(bIsWin);
}

void UOZInGameFloorUI::ChangeSwitcher(bool bisAlive)
{
	if (!Switcher)
		return;

	Switcher->SetActiveWidgetIndex(bisAlive ? 0 : 1);
}

void UOZInGameFloorUI::PlayPopup(FString text_Title, FString text_Msg, float lifeTime)
{
	TutorialPopupMsg->SetRenderOpacity(1.0f);

	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
		return;

	AHUD* HUD = PC->GetHUD();
	if (HUD == nullptr)
		return;

	AOZInGameHUD* ozInGameHUD = Cast<AOZInGameHUD>(HUD);

	bool bIsMinimapopen = true;
	if (ozInGameHUD != nullptr)
	{
		bIsMinimapopen = ozInGameHUD->bIsMinimapOpend;
	}

	TutorialPopupMsg->SetText(text_Title, text_Msg, bIsMinimapopen);
	TutorialPopupMsg->OnPlayTutorialText(lifeTime);
}

void UOZInGameFloorUI::OnFloatPropertyUpdated(FName PropertyName, float NewValue)
{
	UpdateAttributeText(PropertyName, NewValue);
}

void UOZInGameFloorUI::OnInventoryDisplayUpdated(const TArray<FOZInventorySlotDisplayData>& SlotData)
{
	TArray<UOZInvenEntry*> InvenEntries = { WBP_InvenEntry_1, WBP_InvenEntry_2, WBP_InvenEntry_3, WBP_InvenEntry_4 };

	for (int32 i = 0; i < InvenEntries.Num(); ++i)
	{
		UOZInvenEntry* Entry = InvenEntries[i];
		if (!Entry)
			continue;

		if (i < SlotData.Num() && !SlotData[i].IsEmpty())
		{
			const FOZInventorySlotDisplayData& Data = SlotData[i];
			Entry->SetupEntry(Data.ItemName, Data.Icon, Data.Quantity, Data.SlotIndex, Data.ItemID, Data.ItemType);
		}
		else
		{
			Entry->ClearEntry();
			Entry->SetupEntry(FText::GetEmpty(), nullptr, 0, i, 0, EOZItemType::None);
		}
	}
}

void UOZInGameFloorUI::OnConvertDisplayUpdated(const TArray<FOZConvertSlotDisplayData>& SlotData)
{
	for (const FOZConvertSlotDisplayData& Data : SlotData)
	{
		if (Data.SlotIndex >= 0 && Data.SlotIndex < ConvertBGSlots.Num())
		{
			SetConvertSlotVisual(Data.SlotIndex, Data.Grade, Data.IconTexture);
		}
	}
}

void UOZInGameFloorUI::OnNewBuffDisplayed(const FOZBuffDisplayData& BuffData)
{
	if (!BuffProgressEntryClass || !VBox_BuffContainer)
		return;

	UOZBuffProgressEntry* NewEntry = CreateWidget<UOZBuffProgressEntry>(
		GetOwningPlayer(), BuffProgressEntryClass);

	if (NewEntry)
	{
		NewEntry->InitBuff(BuffData.BuffIcon, BuffData.Duration, BuffData.BuffTag, BuffData.BuffName);
		NewEntry->OnBuffExpired.BindUObject(this, &UOZInGameFloorUI::OnBuffEntryExpired);
		VBox_BuffContainer->InsertChildAt(0, NewEntry);
		ActiveBuffEntries.Add(NewEntry);
	}
}

void UOZInGameFloorUI::OnStealthStateUpdated(bool bInBush)
{
	OnStealthStateChanged(bInBush);
}

void UOZInGameFloorUI::UpdateAttributeText(FName PropertyName, float Value)
{
	FString FormattedText = FString::Printf(TEXT("%.0f"), Value);
	FText DisplayText = FText::FromString(FormattedText);

	static const FName NAME_MaxHealth("MaxHealth");
	static const FName NAME_MaxStamina("MaxStamina");
	static const FName NAME_MaxShield("MaxShield");
	static const FName NAME_Armor("Armor");
	static const FName NAME_EvLDistance("EvLDistance");
	static const FName NAME_MoveSpeed("MoveSpeed");

	if (PropertyName == NAME_MaxHealth && Text_CurrentHP)
	{
		Text_CurrentHP->SetText(DisplayText);
	}
	else if (PropertyName == NAME_MaxStamina && Text_CurrentStamina)
	{
		Text_CurrentStamina->SetText(DisplayText);
	}
	else if (PropertyName == NAME_MaxShield && Text_CurrentShield)
	{
		Text_CurrentShield->SetText(DisplayText);
	}
	else if (PropertyName == NAME_Armor && Text_CurrentArmor)
	{
		Text_CurrentArmor->SetText(DisplayText);
	}
	else if (PropertyName == NAME_EvLDistance && Text_CurrentEvldistance)
	{
		Text_CurrentEvldistance->SetText(DisplayText);
	}
	else if (PropertyName == NAME_MoveSpeed && Text_CurrentSpeed)
	{
		Text_CurrentSpeed->SetText(DisplayText);
	}
}

void UOZInGameFloorUI::SetConvertSlotVisual(int32 SlotIndex, EConvertGrade Grade, UTexture2D* IconTexture)
{
	if (SlotIndex < 0 || SlotIndex >= ConvertBGSlots.Num())
		return;

	UImage* BGImage = ConvertBGSlots[SlotIndex];
	UImage* IconImage = ConvertIconSlots[SlotIndex];

	if (BGImage)
	{
		UTexture2D* BGTexture = GetBGTextureForGrade(Grade);
		if (BGTexture)
		{
			BGImage->SetBrushFromTexture(BGTexture);
			BGImage->SetDesiredSizeOverride(FVector2D(69.0f, 75.0f));
		}
	}

	if (IconImage && IconTexture)
	{
		IconImage->SetBrushFromTexture(IconTexture);
		IconImage->SetVisibility(ESlateVisibility::Visible);
		IconImage->SetDesiredSizeOverride(FVector2D(48.0f, 48.0f));
	}
}

UTexture2D* UOZInGameFloorUI::GetBGTextureForGrade(EConvertGrade Grade) const
{
	switch (Grade)
	{
	case EConvertGrade::Rare:
		return ConvertBG_Rare;
	case EConvertGrade::Unique:
		return ConvertBG_Unique;
	case EConvertGrade::Legendary:
		return ConvertBG_Legendary;
	default:
		return ConvertBG_Rare;
	}
}

void UOZInGameFloorUI::OnBuffEntryExpired(UOZBuffProgressEntry* ExpiredEntry)
{
	ActiveBuffEntries.Remove(ExpiredEntry);
}
