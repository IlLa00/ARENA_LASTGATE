#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZModuleData.h"
#include "GameplayEffect.h"
#include "OZModuleSubsystem.generated.h"

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZModuleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Module")
	FGameplayEffectSpecHandle GetModuleEffectSpec(int32 ModuleID, class UAbilitySystemComponent* SourceASC, int32 CurrentRound);

	UFUNCTION(BlueprintCallable, Category = "Module")
	FActiveGameplayEffectHandle ApplyModuleToSelf(int32 ModuleID, class UAbilitySystemComponent* TargetASC, int32 CurrentRound);

	// 사격장에서 사용 
	UFUNCTION(BlueprintCallable, Category = "Module")
	FActiveGameplayEffectHandle ApplyGameEffect(int32 ModuleID, class UAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintPure, Category = "Module")
	float CalculateFinalValue(const FOZModuleData& ModuleData, int32 CurrentRound) const;

	// ScaledMinValue, ScaledMaxValue, FinalValue를 모두 계산하여 ModuleData에 설정
	void CalculateScaledValues(FOZModuleData& ModuleData, int32 CurrentRound) const;

	UFUNCTION(BlueprintPure, Category = "Module")
	const TArray<FOZModuleData>& GetAllModules() const { return ModuleCache; }

	UFUNCTION(BlueprintCallable, Category = "Module")
	FOZModuleData GetRandomModule(const TArray<int32>& ExcludeIDs, bool& bSuccess) const;

	FOZModuleData* GetModuleData(int32 ModuleID) const;

	UFUNCTION(BlueprintCallable, Category = "Module")
	FOZModuleData GetModuleDataFromID(int32 ModuleID) const;

	UFUNCTION(BlueprintCallable, Category = "Module")
	void CalculateAndCacheAllModuleFinalValues(int32 CurrentRound);

	UFUNCTION(BlueprintCallable, Category = "Module")
	float GetCachedFinalValue(int32 ModuleID) const;


	void SetCachedFinalValue(int32 ModuleID, float NewFinalValue);
	void SetCachedMinValue(int32 ModuleID, float NewMinValue);
	void SetCachedMaxValue(int32 ModuleID, float NewMaxValue);
	
	void GetAllCachedFinalValues(TArray<int32>& OutModuleIDs, TArray<float>& OutFinalValues) const;
	void GetAllCachedMinValues(TArray<int32>& OutModuleIDs, TArray<float>& OutMinValues) const;
	void GetAllCachedMaxValues(TArray<int32>& OutModuleIDs, TArray<float>& OutMaxValues) const;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Module Data")
	TSoftObjectPtr<class UDataTable> ModuleDataTableRef;

	UPROPERTY()
	TObjectPtr<class UDataTable> ModuleDataTable;

	TArray<FOZModuleData> ModuleCache;
	TMap<int32, FOZModuleData*> ModuleMap;

	// 로드된 에셋 캐시 (GC 방지용)
	UPROPERTY()
	TMap<int32, TSubclassOf<UGameplayEffect>> CachedEffectClasses;

	UPROPERTY()
	TMap<int32, TObjectPtr<UTexture2D>> CachedIcons;

	FOZModuleData* FindModule(int32 ModuleID) const;
	FGameplayTag GetTagFromAttributeName(const int32 ID) const;
};
