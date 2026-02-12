#include "Subsystem/OZWeaponSubsystem.h"
#include "Engine/DataTable.h"

void UOZWeaponSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (WeaponDataTableRef.IsNull())
		WeaponDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Weapon.DT_Weapon"));

	WeaponDataTable = WeaponDataTableRef.LoadSynchronous();

	if (!WeaponDataTable)
		UE_LOG(LogTemp, Error, TEXT("[WeaponSubsystem] ? Failed to load CharacterDataTable!"));
}

FOZWeaponData* UOZWeaponSubsystem::GetWeaponData()
{
	TArray<FName> AllRow = WeaponDataTable->GetRowNames();

	FOZWeaponData* Data = WeaponDataTable->FindRow<FOZWeaponData>(AllRow[0], TEXT(""));

	return Data;
}

