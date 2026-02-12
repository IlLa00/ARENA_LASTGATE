#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZFogData.h"
#include "OZFogSubsystem.generated.h"

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZFogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Fog Data")
	class UDataTable* GetFogDataTable() const { return FogDataTable; }

	FOZFogData* GetFogPhaseData(int Phase);

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Fog Data")
	TSoftObjectPtr<UDataTable> FogDataTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> FogDataTable;
};
