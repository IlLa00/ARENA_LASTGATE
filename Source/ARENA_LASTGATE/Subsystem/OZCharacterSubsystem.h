#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZCharacterData.h"
#include "OZCharacterSubsystem.generated.h"

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZCharacterSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Character Data")
	class UDataTable* GetCharacterDataTable() const { return ChracterDataTable; }

	FOZCharacterData* GetCharacterData();

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Character Data")
	TSoftObjectPtr<UDataTable> ChracterDataTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> ChracterDataTable;
};
