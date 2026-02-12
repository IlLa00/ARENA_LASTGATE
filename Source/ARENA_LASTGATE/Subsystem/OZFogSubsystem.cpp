#include "Subsystem/OZFogSubsystem.h"

void UOZFogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (FogDataTableRef.IsNull())
		FogDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Fog.DT_Fog"));

	FogDataTable = FogDataTableRef.LoadSynchronous();

	if (!FogDataTable)
		UE_LOG(LogTemp, Error, TEXT("[FogSubsystem] ? Failed to load FogDataTable!"));
}

FOZFogData* UOZFogSubsystem::GetFogPhaseData(int Phase)
{
	if (!FogDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[FogSubsystem] FogDataTable is null!"));
		return nullptr;
	}

	TArray<FName> RowNames = FogDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FOZFogData* Data = FogDataTable->FindRow<FOZFogData>(RowName, TEXT(""));
		if (Data && Data->Fog_Phase == Phase)
		{
			return Data;
		}
	}

	return nullptr;
}
