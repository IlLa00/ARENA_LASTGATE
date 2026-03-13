#pragma once

#include "CoreMinimal.h"
#include "ViewModel/OZViewModelBase.h"
#include "ViewModel/OZViewModelTypes.h"
#include "GameplayEffectTypes.h"
#include "ActiveGameplayEffectHandle.h"
#include "OZFloorViewModel.generated.h"

class UAbilitySystemComponent;
class UOZInventoryComponent;
class UOZConvertSubsystem;
class UOZItemSubsystem;
struct FOnAttributeChangeData;
struct FActiveGameplayEffect;

DECLARE_MULTICAST_DELEGATE_OneParam(FOZOnInventoryDataChanged, const TArray<FOZInventorySlotDisplayData>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOZOnConvertDataChanged, const TArray<FOZConvertSlotDisplayData>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOZOnNewBuffDetected, const FOZBuffDisplayData&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOZOnStealthChanged, bool);

UCLASS()
class ARENA_LASTGATE_API UOZFloorViewModel : public UOZViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	FOZOnInventoryDataChanged OnInventoryDataChanged;
	FOZOnConvertDataChanged OnConvertDataChanged;
	FOZOnNewBuffDetected OnNewBuffDetected;
	FOZOnStealthChanged OnStealthChanged;

	void RequestInventoryRefresh();
	void UpdateBuffTimers();
	void PollStealthState();

protected:
	virtual void OnPlayerStateReady(AOZPlayerState* InPlayerState) override;

private:
	UPROPERTY()
	TObjectPtr<UOZInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<UOZConvertSubsystem> ConvertSubsystem;

	UPROPERTY()
	TObjectPtr<UOZItemSubsystem> ItemSubsystem;

	bool bAttributeDelegatesBound = false;
	bool bPreviousInBush = false;

	TSet<FGameplayTag> TrackedBuffTags;

	void BindAttributeDelegates();
	void UnbindAttributeDelegates();
	void InitializeAttributeUI();

	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void OnMaxShieldChanged(const FOnAttributeChangeData& Data);
	void OnArmorChanged(const FOnAttributeChangeData& Data);
	void OnEvLDistanceChanged(const FOnAttributeChangeData& Data);
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void HandleInventoryUpdated();

	UFUNCTION()
	void HandleConvertAcquired(int32 ConvertID, EConvertGrade Grade);

	TArray<FOZInventorySlotDisplayData> TransformInventorySlots() const;
	TArray<FOZConvertSlotDisplayData> TransformConvertSlots() const;

	int32 GetBuffItemIDFromTag(const FGameplayTag& BuffTag) const;
};
