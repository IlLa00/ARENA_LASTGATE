#include "UserInterface/OZInGameFloorUI.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "HUD/OZInGameHUD.h"
#include "GameState/OZInGameGameState.h"
#include "UserInterface/OZInvenEntry.h"
#include "UserInterface/OZBuffProgressEntry.h"
#include "Character/OZPlayerState.h"
#include "Character/Components/OZInventoryComponent.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Components/WidgetSwitcher.h"
#include "Widget/OZTutorialPopupTextWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Tags/OZGameplayTags.h"

void UOZInGameFloorUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Convert 슬롯 배열 초기화
	ConvertBGSlots = { img_BG_Convert_1, img_BG_Convert_2, img_BG_Convert_3,
					   img_BG_Convert_4, img_BG_Convert_5, img_BG_Convert_6 };
	ConvertIconSlots = { img_Icon_Convert_1, img_Icon_Convert_2, img_Icon_Convert_3,
						 img_Icon_Convert_4, img_Icon_Convert_5, img_Icon_Convert_6 };

	// ConvertSubsystem 캐싱
	if (UGameInstance* GI = GetGameInstance())
	{
		ConvertSubsystem = GI->GetSubsystem<UOZConvertSubsystem>();
	}

	// 블루프린트에서 설정한 빈 이미지 상태 유지 (Visibility 변경 안 함)
}

void UOZInGameFloorUI::NativeDestruct()
{
	UnbindAttributeDelegates();

	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UOZInGameFloorUI::OnInventoryUpdated);
	}

	if (PlayerState)
	{
		PlayerState->OnConvertAcquired.RemoveDynamic(this, &UOZInGameFloorUI::OnConvertAcquired);
	}

	// 활성 버프 엔트리 정리
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

	if (!bPlayerStateInitialized)
	{
		APlayerController* PC = GetOwningPlayer();
		if (PC)
		{
			PlayerState = PC->GetPlayerState<AOZPlayerState>();
			if (PlayerState)
			{
				InventoryComp = PlayerState->FindComponentByClass<UOZInventoryComponent>();
				if (InventoryComp)
				{
					InventoryComp->OnInventoryUpdated.AddDynamic(this, &UOZInGameFloorUI::OnInventoryUpdated);
					RefreshInventory();
				}

				ASC = PlayerState->GetAbilitySystemComponent();
				if (ASC)
				{
					BindAttributeDelegates();
					InitializeAttributeUI();
				}

				// Convert 획득 델리게이트 바인딩
				PlayerState->OnConvertAcquired.AddDynamic(this, &UOZInGameFloorUI::OnConvertAcquired);

				UpdateConvertUI();

				// 초기 Stealth UI 설정
				UpdateStealthVisibility(PlayerState->IsInBush());
				bPreviousInBush = PlayerState->IsInBush();

				bPlayerStateInitialized = true;
			}
		}
	}
	else if (PlayerState)
	{
		bool bCurrentInBush = PlayerState->IsInBush();
		if (bCurrentInBush != bPreviousInBush)
		{
			UpdateStealthVisibility(bCurrentInBush);
			bPreviousInBush = bCurrentInBush;
		}

		// 버프 타이머 업데이트
		UpdateBuffTimers();
	}
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
	if (!InventoryComp)
		return;

	UOZItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UOZItemSubsystem>();
	if (!ItemSubsystem)
		return;

	const TArray<FOZInventorySlot>& Slots = InventoryComp->GetAllSlots();

	TArray<UOZInvenEntry*> InvenEntries = { WBP_InvenEntry_1, WBP_InvenEntry_2, WBP_InvenEntry_3, WBP_InvenEntry_4 };

	for (int32 i = 0; i < InvenEntries.Num(); ++i)
	{
		UOZInvenEntry* Entry = InvenEntries[i];
		if (!Entry)
			continue;

		if (i < Slots.Num() && !Slots[i].IsEmpty())
		{
			const FOZInventorySlot& InvenSlot = Slots[i];

			FText ItemName;
			UTexture2D* Icon = nullptr;

			if (InvenSlot.ItemType == EOZItemType::Battle)
			{
				UDataTable* BattleItemDataTable = ItemSubsystem->GetBattleItemDataTable();
				if (BattleItemDataTable)
				{
					FString ContextString;
					TArray<FName> RowNames = BattleItemDataTable->GetRowNames();
					for (const FName& RowName : RowNames)
					{
						FOZBattleItemData* ItemData = BattleItemDataTable->FindRow<FOZBattleItemData>(RowName, ContextString);
						if (ItemData && ItemData->Item_ID == InvenSlot.ItemID)
						{
							ItemName = ItemData->Item_Name;
							Icon = ItemData->ItemIcon.LoadSynchronous();
							break;
						}
					}
				}
			}
			else if (InvenSlot.ItemType == EOZItemType::Buff)
			{
				UDataTable* BuffItemDataTable = ItemSubsystem->GetBuffItemDataTable();
				if (BuffItemDataTable)
				{
					FString ContextString;
					TArray<FName> RowNames = BuffItemDataTable->GetRowNames();
					for (const FName& RowName : RowNames)
					{
						FOZBuffItemData* ItemData = BuffItemDataTable->FindRow<FOZBuffItemData>(RowName, ContextString);
						if (ItemData && ItemData->Item_ID == InvenSlot.ItemID)
						{
							ItemName = ItemData->Item_Name;
							Icon = ItemData->ItemIcon.LoadSynchronous();
							break;
						}
					}
				}
			}

			Entry->SetupEntry(ItemName, Icon, InvenSlot.Quantity, i, InvenSlot.ItemID, InvenSlot.ItemType);
		}
		else
		{
			// 빈 슬롯이어도 슬롯 인덱스는 유지
			Entry->ClearEntry();
			Entry->SetupEntry(FText::GetEmpty(), nullptr, 0, i, 0, EOZItemType::None);
		}
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

	if (PlayerState == nullptr)
		return;

	if (PlayerState->GetPlayerController() == nullptr)
		return;

	if (PlayerState->GetPlayerController()->GetHUD() == nullptr)
		return;

	AOZInGameHUD* ozInGameHUD = Cast<AOZInGameHUD>(PlayerState->GetPlayerController()->GetHUD());

	bool bIsMinimapopen = true;
	if (ozInGameHUD != nullptr)
	{
		bIsMinimapopen = ozInGameHUD->bIsMinimapOpend;
	}


	TutorialPopupMsg->SetText(text_Title, text_Msg, bIsMinimapopen);
	TutorialPopupMsg->OnPlayTutorialText(lifeTime);
}

void UOZInGameFloorUI::BindAttributeDelegates()
{
	if (!ASC || bAttributeDelegatesBound)
		return;

	// MaxHealth
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnMaxHealthChanged);

	// MaxStamina
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxStaminaAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnMaxStaminaChanged);

	// MaxShield
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxShieldAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnMaxShieldChanged);

	// Armor
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetArmorAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnArmorChanged);

	// EvLDistance
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetEvLDistanceAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnEvLDistanceChanged);

	// MoveSpeed
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &UOZInGameFloorUI::OnMoveSpeedChanged);

	bAttributeDelegatesBound = true;
}

void UOZInGameFloorUI::UnbindAttributeDelegates()
{
	if (!ASC || !bAttributeDelegatesBound)
		return;

	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxHealthAttribute())
		.RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxStaminaAttribute())
		.RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxShieldAttribute())
		.RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetArmorAttribute())
		.RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetEvLDistanceAttribute())
		.RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMoveSpeedAttribute())
		.RemoveAll(this);

	bAttributeDelegatesBound = false;
}

void UOZInGameFloorUI::InitializeAttributeUI()
{
	if (!ASC)
		return;

	// 최대값을 가져와서 초기 UI 설정
	float MaxHP = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxHealthAttribute());
	UpdateHealthUI(MaxHP);

	float MaxStamina = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxStaminaAttribute());
	UpdateStaminaUI(MaxStamina);

	float MaxShield = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxShieldAttribute());
	UpdateShieldUI(MaxShield);

	float Armor = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetArmorAttribute());
	UpdateArmorUI(Armor);

	float EvLDistance = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetEvLDistanceAttribute());
	UpdateEvLDistanceUI(EvLDistance);

	float MoveSpeed = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMoveSpeedAttribute());
	UpdateSpeedUI(MoveSpeed);
}

void UOZInGameFloorUI::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHealthUI(Data.NewValue);
}

void UOZInGameFloorUI::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	UpdateStaminaUI(Data.NewValue);
}

void UOZInGameFloorUI::OnMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	UpdateShieldUI(Data.NewValue);
}

void UOZInGameFloorUI::OnArmorChanged(const FOnAttributeChangeData& Data)
{
	UpdateArmorUI(Data.NewValue);
}

void UOZInGameFloorUI::OnEvLDistanceChanged(const FOnAttributeChangeData& Data)
{
	UpdateEvLDistanceUI(Data.NewValue);
}

void UOZInGameFloorUI::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	UpdateSpeedUI(Data.NewValue);
}

void UOZInGameFloorUI::UpdateHealthUI(float CurrentHP)
{
	if (Text_CurrentHP)
	{
		FString HPText = FString::Printf(TEXT("%.0f"), CurrentHP);
		Text_CurrentHP->SetText(FText::FromString(HPText));
	}
}

void UOZInGameFloorUI::UpdateStaminaUI(float CurrentStamina)
{
	if (Text_CurrentStamina)
	{
		FString StaminaText = FString::Printf(TEXT("%.0f"), CurrentStamina);
		Text_CurrentStamina->SetText(FText::FromString(StaminaText));
	}
}

void UOZInGameFloorUI::UpdateShieldUI(float CurrentShield)
{
	if (Text_CurrentShield)
	{
		FString ShieldText = FString::Printf(TEXT("%.0f"), CurrentShield);
		Text_CurrentShield->SetText(FText::FromString(ShieldText));
	}
}

void UOZInGameFloorUI::UpdateArmorUI(float Armor)
{
	if (Text_CurrentArmor)
	{
		FString ArmorText = FString::Printf(TEXT("%.0f"), Armor);
		Text_CurrentArmor->SetText(FText::FromString(ArmorText));
	}
}

void UOZInGameFloorUI::UpdateEvLDistanceUI(float EvLDistance)
{
	if (Text_CurrentEvldistance)
	{
		FString EvLDistText = FString::Printf(TEXT("%.0f"), EvLDistance);
		Text_CurrentEvldistance->SetText(FText::FromString(EvLDistText));
	}
}

void UOZInGameFloorUI::UpdateSpeedUI(float MoveSpeed)
{
	if (Text_CurrentSpeed)
	{
		FString SpeedText = FString::Printf(TEXT("%.0f"), MoveSpeed);
		Text_CurrentSpeed->SetText(FText::FromString(SpeedText));
	}
}

void UOZInGameFloorUI::UpdateStealthVisibility(bool bInBush)
{
	OnStealthStateChanged(bInBush);
}

void UOZInGameFloorUI::OnConvertAcquired(int32 ConvertID, EConvertGrade Grade)
{
	UpdateConvertUI();
}

void UOZInGameFloorUI::UpdateConvertUI()
{
	if (!PlayerState || !ConvertSubsystem)
		return;

	const TArray<int32>& AcquiredConvertIDs = PlayerState->GetAcquiredConvertIDs();

	// 획득한 Convert만 슬롯에 설정 (빈 슬롯은 블루프린트의 빈 이미지 유지)
	const int32 MaxSlots = FMath::Min(AcquiredConvertIDs.Num(), ConvertBGSlots.Num());

	for (int32 i = 0; i < MaxSlots; ++i)
	{
		SetConvertSlot(i, AcquiredConvertIDs[i]);
	}
}

void UOZInGameFloorUI::SetConvertSlot(int32 SlotIndex, int32 ConvertID)
{
	if (!ConvertSubsystem)
		return;

	if (SlotIndex < 0 || SlotIndex >= ConvertBGSlots.Num())
		return;

	FOZConvertData* ConvertData = ConvertSubsystem->FindConvert(ConvertID);
	if (!ConvertData)
		return;

	UImage* BGImage = ConvertBGSlots[SlotIndex];
	UImage* IconImage = ConvertIconSlots[SlotIndex];

	// BG 설정 (등급에 따라)
	if (BGImage)
	{
		UTexture2D* BGTexture = GetBGTextureForGrade(ConvertData->Grade);
		if (BGTexture)
		{
			BGImage->SetBrushFromTexture(BGTexture);
			BGImage->SetDesiredSizeOverride(FVector2D(69.0f, 75.0f));
		}
	}

	// Icon 설정 (캐싱된 아이콘 사용)
	if (IconImage)
	{
		UTexture2D* IconTexture = ConvertSubsystem->GetCachedIcon(ConvertID);
		if (IconTexture)
		{
			IconImage->SetBrushFromTexture(IconTexture);
			IconImage->SetVisibility(ESlateVisibility::Visible);
			IconImage->SetDesiredSizeOverride(FVector2D(48.0f, 48.0f));
		}
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
		return ConvertBG_Rare; // 기본값
	}
}

void UOZInGameFloorUI::UpdateBuffTimers()
{
	if (!ASC || !BuffProgressEntryClass || !VBox_BuffContainer || !InventoryComp)
		return;

	const FActiveGameplayEffectsContainer& ActiveEffects = ASC->GetActiveGameplayEffects();
	for (const FActiveGameplayEffect& ActiveEffect : &ActiveEffects)
	{
		if (!ActiveEffect.Spec.Def)
			continue;

		// SetByCaller 태그에서 Item.Buff 자식 태그 찾기
		FGameplayTag BuffTag;
		for (const auto& Pair : ActiveEffect.Spec.SetByCallerTagMagnitudes)
		{
			if (Pair.Key.MatchesTag(OZGameplayTags::Item_Buff))
			{
				BuffTag = Pair.Key;
				break;
			}
		}

		if (!BuffTag.IsValid())
			continue;

		// Duration 확인
		float Duration = ActiveEffect.Spec.GetDuration();
		if (Duration <= 0.f)
			continue;

		// 이미 UI가 있는지 확인
		bool bAlreadyTracked = false;
		for (const UOZBuffProgressEntry* Entry : ActiveBuffEntries)
		{
			if (Entry && Entry->GetBuffTag() == BuffTag)
			{
				bAlreadyTracked = true;
				break;
			}
		}

		// 새로운 버프면 UI 생성
		if (!bAlreadyTracked)
		{
			int32 ItemID = GetBuffItemIDFromTag(BuffTag);
			if (ItemID == 0)
				continue;

			TSoftObjectPtr<UTexture2D> BuffIconSoft = InventoryComp->GetBuffItemIconByID(ItemID);
			UTexture2D* BuffIcon = BuffIconSoft.LoadSynchronous();
			FText BuffName = InventoryComp->GetBuffItemNameByID(ItemID);

			UOZBuffProgressEntry* NewEntry = CreateWidget<UOZBuffProgressEntry>(
				GetOwningPlayer(), BuffProgressEntryClass);

			if (NewEntry)
			{
				NewEntry->InitBuff(BuffIcon, Duration, BuffTag, BuffName);
				NewEntry->OnBuffExpired.BindUObject(this, &UOZInGameFloorUI::OnBuffEntryExpired);
				VBox_BuffContainer->InsertChildAt(0, NewEntry);
				ActiveBuffEntries.Add(NewEntry);
			}
		}
	}
}

void UOZInGameFloorUI::OnBuffEntryExpired(UOZBuffProgressEntry* ExpiredEntry)
{
	ActiveBuffEntries.Remove(ExpiredEntry);
}

int32 UOZInGameFloorUI::GetBuffItemIDFromTag(const FGameplayTag& BuffTag) const
{
	FString TagString = BuffTag.ToString();
	FString BuffName;
	TagString.Split(TEXT("."), nullptr, &BuffName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	// 영어 태그 이름 → Item_ID 매핑
	static TMap<FString, int32> BuffNameToID = {
		{TEXT("Aspirin"), 30000},
		{TEXT("EnergyDrink"), 30001},
		{TEXT("Adrenaline"), 30002},
		{TEXT("ProteinShake"), 30003},
		{TEXT("PlasmaBattery"), 30004},
	};

	if (const int32* FoundID = BuffNameToID.Find(BuffName))
	{
		return *FoundID;
	}

	return 0;
}
