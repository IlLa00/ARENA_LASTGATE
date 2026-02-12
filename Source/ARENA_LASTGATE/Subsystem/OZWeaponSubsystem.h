#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZWeaponData.h"
#include "OZWeaponSubsystem.generated.h"

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZWeaponSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon Data")
	class UDataTable* GetWeaponDataTable() const { return WeaponDataTable; }

	FOZWeaponData* GetWeaponData();

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Weapon Data")
	TSoftObjectPtr<UDataTable> WeaponDataTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> WeaponDataTable;
	
};
