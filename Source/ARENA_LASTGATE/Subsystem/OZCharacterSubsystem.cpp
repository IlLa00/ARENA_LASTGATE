#include "Subsystem/OZCharacterSubsystem.h"
#include "Engine/DataTable.h"

void UOZCharacterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ChracterDataTableRef.IsNull())
		ChracterDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Character.DT_Character"));

	ChracterDataTable = ChracterDataTableRef.LoadSynchronous();

	if (!ChracterDataTable)
		UE_LOG(LogTemp, Error, TEXT("[ChracterSubsystem] ? Failed to load CharacterDataTable!"));
}

FOZCharacterData* UOZCharacterSubsystem::GetCharacterData()
{
	TArray<FName> AllRow = ChracterDataTable->GetRowNames();

	FOZCharacterData* Data = ChracterDataTable->FindRow<FOZCharacterData>(AllRow[0], TEXT(""));

	return Data;
}
