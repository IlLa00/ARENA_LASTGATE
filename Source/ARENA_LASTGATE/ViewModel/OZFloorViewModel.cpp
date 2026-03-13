#include "ViewModel/OZFloorViewModel.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Character/Components/OZInventoryComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"
#include "Tags/OZGameplayTags.h"

void UOZFloorViewModel::OnPlayerStateReady(AOZPlayerState* InPlayerState)
{
	if (!InPlayerState)
		return;

	if (UGameInstance* GI = InPlayerState->GetGameInstance())
	{
		ConvertSubsystem = GI->GetSubsystem<UOZConvertSubsystem>();
		ItemSubsystem = GI->GetSubsystem<UOZItemSubsystem>();
	}

	InventoryComp = InPlayerState->FindComponentByClass<UOZInventoryComponent>();
	if (InventoryComp)
	{
		InventoryComp->OnInventoryUpdated.AddDynamic(this, &UOZFloorViewModel::HandleInventoryUpdated);
		OnInventoryDataChanged.Broadcast(TransformInventorySlots());
	}

	ASC = InPlayerState->GetAbilitySystemComponent();
	if (ASC)
	{
		BindAttributeDelegates();
		InitializeAttributeUI();
	}

	InPlayerState->OnConvertAcquired.AddDynamic(this, &UOZFloorViewModel::HandleConvertAcquired);
	OnConvertDataChanged.Broadcast(TransformConvertSlots());

	bPreviousInBush = InPlayerState->IsInBush();
	OnStealthChanged.Broadcast(bPreviousInBush);
}

void UOZFloorViewModel::Deinitialize()
{
	UnbindAttributeDelegates();

	if (InventoryComp)
		InventoryComp->OnInventoryUpdated.RemoveDynamic(this, &UOZFloorViewModel::HandleInventoryUpdated);

	if (AOZPlayerState* PS = GetLocalPlayerState())
		PS->OnConvertAcquired.RemoveDynamic(this, &UOZFloorViewModel::HandleConvertAcquired);

	TrackedBuffTags.Empty();

	Super::Deinitialize();
}

void UOZFloorViewModel::BindAttributeDelegates()
{
	if (!ASC || bAttributeDelegatesBound)
		return;

	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnMaxHealthChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxStaminaAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnMaxStaminaChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxShieldAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnMaxShieldChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetArmorAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnArmorChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetEvLDistanceAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnEvLDistanceChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &UOZFloorViewModel::OnMoveSpeedChanged);

	bAttributeDelegatesBound = true;
}

void UOZFloorViewModel::UnbindAttributeDelegates()
{
	if (!ASC || !bAttributeDelegatesBound)
		return;

	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxStaminaAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMaxShieldAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetArmorAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetEvLDistanceAttribute()).RemoveAll(this);
	ASC->GetGameplayAttributeValueChangeDelegate(UOZPlayerAttributeSet::GetMoveSpeedAttribute()).RemoveAll(this);

	bAttributeDelegatesBound = false;
}

void UOZFloorViewModel::InitializeAttributeUI()
{
	if (!ASC)
		return;

	NotifyFloatChanged(FName("MaxHealth"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxHealthAttribute()));
	NotifyFloatChanged(FName("MaxStamina"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxStaminaAttribute()));
	NotifyFloatChanged(FName("MaxShield"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMaxShieldAttribute()));
	NotifyFloatChanged(FName("Armor"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetArmorAttribute()));
	NotifyFloatChanged(FName("EvLDistance"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetEvLDistanceAttribute()));
	NotifyFloatChanged(FName("MoveSpeed"), ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetMoveSpeedAttribute()));
}


void UOZFloorViewModel::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("MaxHealth"), Data.NewValue);
}

void UOZFloorViewModel::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("MaxStamina"), Data.NewValue);
}

void UOZFloorViewModel::OnMaxShieldChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("MaxShield"), Data.NewValue);
}

void UOZFloorViewModel::OnArmorChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("Armor"), Data.NewValue);
}

void UOZFloorViewModel::OnEvLDistanceChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("EvLDistance"), Data.NewValue);
}

void UOZFloorViewModel::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	NotifyFloatChanged(FName("MoveSpeed"), Data.NewValue);
}

void UOZFloorViewModel::HandleInventoryUpdated()
{
	OnInventoryDataChanged.Broadcast(TransformInventorySlots());
}

void UOZFloorViewModel::RequestInventoryRefresh()
{
	if (InventoryComp)
	{
		OnInventoryDataChanged.Broadcast(TransformInventorySlots());
	}
}

TArray<FOZInventorySlotDisplayData> UOZFloorViewModel::TransformInventorySlots() const
{
	TArray<FOZInventorySlotDisplayData> Result;

	if (!InventoryComp || !ItemSubsystem)
		return Result;

	const TArray<FOZInventorySlot>& Slots = InventoryComp->GetAllSlots();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		FOZInventorySlotDisplayData DisplayData;
		DisplayData.SlotIndex = i;

		if (!Slots[i].IsEmpty())
		{
			const FOZInventorySlot& InvenSlot = Slots[i];
			DisplayData.ItemID = InvenSlot.ItemID;
			DisplayData.ItemType = InvenSlot.ItemType;
			DisplayData.Quantity = InvenSlot.Quantity;

			if (InvenSlot.ItemType == EOZItemType::Battle)
			{
				FOZBattleItemData* ItemData = ItemSubsystem->GetBattleItemData(InvenSlot.ItemID);
				if (ItemData)
				{
					DisplayData.ItemName = ItemData->Item_Name;
					DisplayData.Icon = ItemData->ItemIcon.LoadSynchronous();
				}
			}
			else if (InvenSlot.ItemType == EOZItemType::Buff)
			{
				FOZBuffItemData* ItemData = ItemSubsystem->GetBuffItemData(InvenSlot.ItemID);
				if (ItemData)
				{
					DisplayData.ItemName = ItemData->Item_Name;
					DisplayData.Icon = ItemData->ItemIcon.LoadSynchronous();
				}
			}
		}

		Result.Add(DisplayData);
	}

	return Result;
}

void UOZFloorViewModel::HandleConvertAcquired(int32 ConvertID, EConvertGrade Grade)
{
	OnConvertDataChanged.Broadcast(TransformConvertSlots());
}

TArray<FOZConvertSlotDisplayData> UOZFloorViewModel::TransformConvertSlots() const
{
	TArray<FOZConvertSlotDisplayData> Result;

	AOZPlayerState* PS = GetLocalPlayerState();
	if (!PS || !ConvertSubsystem)
		return Result;

	const TArray<int32>& AcquiredConvertIDs = PS->GetAcquiredConvertIDs();

	for (int32 i = 0; i < AcquiredConvertIDs.Num(); ++i)
	{
		FOZConvertSlotDisplayData SlotData;
		SlotData.SlotIndex = i;

		FOZConvertData* ConvertData = ConvertSubsystem->FindConvert(AcquiredConvertIDs[i]);
		if (ConvertData)
		{
			SlotData.Grade = ConvertData->Grade;
			SlotData.IconTexture = ConvertSubsystem->GetCachedIcon(AcquiredConvertIDs[i]);
		}

		Result.Add(SlotData);
	}

	return Result;
}

void UOZFloorViewModel::PollStealthState()
{
	AOZPlayerState* PS = GetLocalPlayerState();
	if (!PS)
		return;

	bool bCurrentInBush = PS->IsInBush();
	if (bCurrentInBush != bPreviousInBush)
	{
		bPreviousInBush = bCurrentInBush;
		OnStealthChanged.Broadcast(bCurrentInBush);
	}
}

void UOZFloorViewModel::UpdateBuffTimers()
{
	if (!ASC || !InventoryComp)
		return;

	const FActiveGameplayEffectsContainer& ActiveEffects = ASC->GetActiveGameplayEffects();
	for (const FActiveGameplayEffect& ActiveEffect : &ActiveEffects)
	{
		if (!ActiveEffect.Spec.Def)
			continue;

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

		float Duration = ActiveEffect.Spec.GetDuration();
		if (Duration <= 0.f)
			continue;

		if (TrackedBuffTags.Contains(BuffTag))
			continue;

		int32 ItemID = GetBuffItemIDFromTag(BuffTag);
		if (ItemID == 0)
			continue;

		FOZBuffDisplayData BuffData;
		BuffData.ItemID = ItemID;
		BuffData.BuffTag = BuffTag;
		BuffData.Duration = Duration;
		BuffData.BuffIcon = InventoryComp->GetBuffItemIconByID(ItemID).LoadSynchronous();
		BuffData.BuffName = InventoryComp->GetBuffItemNameByID(ItemID);

		TrackedBuffTags.Add(BuffTag);
		OnNewBuffDetected.Broadcast(BuffData);
	}
}


int32 UOZFloorViewModel::GetBuffItemIDFromTag(const FGameplayTag& BuffTag) const
{
	FString TagString = BuffTag.ToString();
	FString BuffName;
	TagString.Split(TEXT("."), nullptr, &BuffName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

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
