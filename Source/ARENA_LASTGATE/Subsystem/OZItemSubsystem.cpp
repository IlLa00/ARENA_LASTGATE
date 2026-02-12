#include "Subsystem/OZItemSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"

void UOZItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (BattleItemDataTableRef.IsNull())
		BattleItemDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_BattleItem.DT_BattleItem"));

	BattleItemDataTable = BattleItemDataTableRef.LoadSynchronous();

	if (!BattleItemDataTable)
		UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] ❌ Failed to load BattleItemDataTable!"));

	if (BuffItemDataTableRef.IsNull())
		BuffItemDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_BuffItem.DT_BuffItem"));

	BuffItemDataTable = BuffItemDataTableRef.LoadSynchronous();

	if (!BuffItemDataTable)
		UE_LOG(LogTemp, Error, TEXT("[ItemSubsystem] ❌ Failed to load BuffItemDataTable!"));
}

FOZBattleItemData* UOZItemSubsystem::GetBattleItemData(int32 ItemID) const
{
	if (!BattleItemDataTable) 
		return nullptr;

	TArray<FOZBattleItemData*> AllRows;
	BattleItemDataTable->GetAllRows<FOZBattleItemData>(TEXT("GetBattleItemData"), AllRows);

	// ItemID 필드로 찾기
	for (FOZBattleItemData* Row : AllRows)
	{
		if (Row && Row->Item_ID == ItemID)
			return Row;
	}

	return nullptr;
}

int32 UOZItemSubsystem::GetBattleItemPrice(int32 ItemID)
{
	if (FOZBattleItemData* Data = GetBattleItemData(ItemID))
		return Data->Item_Cost;

	return 0;
}

int32 UOZItemSubsystem::GetBattleItemMaxStack(int32 ItemID)
{
	if (FOZBattleItemData* Data = GetBattleItemData(ItemID))
		return Data->Max_Stack;

	return 0;
}

FOZBuffItemData* UOZItemSubsystem::GetBuffItemData(int32 ItemID) const
{
	if (!BuffItemDataTable) 
		return nullptr;

	TArray<FOZBuffItemData*> AllRows;
	BuffItemDataTable->GetAllRows<FOZBuffItemData>(TEXT("GetBuffItemData"), AllRows);

	for (FOZBuffItemData* Row : AllRows)
	{
		if (Row && Row->Item_ID == ItemID)
			return Row;
	}

	return nullptr;
}

FOZBattleItemData UOZItemSubsystem::GetBattleItemDataFromID(int32 ItemID) const
{
	if (!BattleItemDataTable)
		return FOZBattleItemData();

	TArray<FOZBattleItemData*> AllRows;
	BattleItemDataTable->GetAllRows<FOZBattleItemData>(TEXT("GetBattleItemData"), AllRows);

	for (const FOZBattleItemData* RowPtr : AllRows)
	{
		if (RowPtr && RowPtr->Item_ID == ItemID)
		{
			return *RowPtr;
		}
	}

	return FOZBattleItemData();
}

FOZBuffItemData UOZItemSubsystem::GetBuffItemDataFromID(int32 ItemID) const
{
	if (!BuffItemDataTable)
		return FOZBuffItemData();

	TArray<FOZBuffItemData*> AllRows;
	BuffItemDataTable->GetAllRows<FOZBuffItemData>(TEXT("GetBuffItemData"), AllRows);

	for (const FOZBuffItemData* RowPtr : AllRows)
	{
		if (RowPtr && RowPtr->Item_ID == ItemID)
		{
			return *RowPtr;
		}
	}

	return FOZBuffItemData();
}


int32 UOZItemSubsystem::GetBuffItemMaxStack(int32 ItemID)
{
	if (FOZBuffItemData* Data = GetBuffItemData(ItemID))
		return Data->Max_Stack;

	return 0;
}

int32 UOZItemSubsystem::GetBuffItemPrice(int32 ItemID)
{
	if (FOZBuffItemData* Data = GetBuffItemData(ItemID))
		return Data->Item_Cost;

	return 0;
}