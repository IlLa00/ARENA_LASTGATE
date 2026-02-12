#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "Character/Components/OZInventoryComponent.h"
#include "OZShopItemEntry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnShopItemClicked, int32, ItemID, EOZItemType, ItemType, FText, ItemName, FText, ItemExplain, int32, Price, UTexture2D*, Icon);

UCLASS()
class ARENA_LASTGATE_API UOZShopItemEntry : public UOZUI
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Shop")
	FOnShopItemClicked OnEntryClicked;

	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetupEntry(int32 InItemID, EOZItemType InItemType, const FText& InName, const FText& InExplanation, UTexture2D* InIcon, int32 InPrice);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnItemButtonClicked();

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* ItemButton;

	UPROPERTY()
	int32 ItemID = 0;

	UPROPERTY()
	EOZItemType ItemType = EOZItemType::None;

	UPROPERTY()
	FText ItemName;

	UPROPERTY()
	FText ItemExplain;

	UPROPERTY()
	int32 Price = 0;

	UPROPERTY()
	UTexture2D* ItemIcon = nullptr;
};
