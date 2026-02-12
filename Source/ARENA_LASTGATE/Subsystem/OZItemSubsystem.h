#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZBattleItemData.h"
#include "Data/OZBuffItemData.h"
#include "OZItemSubsystem.generated.h"

UCLASS(Config=Engine)
class ARENA_LASTGATE_API UOZItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	class UDataTable* GetBattleItemDataTable() const { return BattleItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	class UDataTable* GetBuffItemDataTable() const { return BuffItemDataTable; }

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	bool IsDataLoaded() const { return BattleItemDataTable != nullptr && BuffItemDataTable != nullptr; }

	FOZBattleItemData* GetBattleItemData(int32 ItemID) const;
	FOZBuffItemData* GetBuffItemData(int32 ItemID) const;

	UFUNCTION(BlueprintCallable)
	FOZBattleItemData GetBattleItemDataFromID(int32 ItemID) const;

	UFUNCTION(BlueprintCallable)
	FOZBuffItemData GetBuffItemDataFromID(int32 ItemID) const;

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	int32 GetBattleItemPrice(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	int32 GetBuffItemPrice(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	int32 GetBattleItemMaxStack(int32 ItemID);

	UFUNCTION(BlueprintCallable, Category = "Item Data")
	int32 GetBuffItemMaxStack(int32 ItemID);
	
protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Item Data")
	TSoftObjectPtr<UDataTable> BattleItemDataTableRef;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Item Data")
	TSoftObjectPtr<UDataTable> BuffItemDataTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> BattleItemDataTable;

	UPROPERTY()
	TObjectPtr<UDataTable> BuffItemDataTable;
};
