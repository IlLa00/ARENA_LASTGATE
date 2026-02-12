#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZConvertData.h"
#include "GameplayEffect.h"
#include "OZConvertSubsystem.generated.h"

UCLASS(Config=Engine)
class ARENA_LASTGATE_API UOZConvertSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// DataTable의 Convert_Asset 사용 (모듈과 동일한 방식)
	UFUNCTION(BlueprintCallable, Category = "Convert")
	FActiveGameplayEffectHandle ApplyConvertToSelf(int32 ConvertID, class UAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintPure, Category = "Convert")
	const TArray<FOZConvertData>& GetAllConverts() const { return ConvertCache; }

	UFUNCTION(BlueprintCallable, Category = "Convert")
	TArray<FOZConvertData> GetConvertsByGrade(EConvertGrade Grade) const;

	UFUNCTION(BlueprintCallable, Category = "Convert")
	FOZConvertData GetRandomConvertByGrade(EConvertGrade Grade, const TArray<int32>& ExcludeIDs, bool& bSuccess) const;

	UFUNCTION(BlueprintCallable, Category = "Convert")
	FOZConvertData RerollConvert(int32 CurrentConvertID, const TArray<int32>& ExcludeConvertIDs, bool& bSuccess) const;

	FOZConvertData* FindConvert(int32 ConvertID) const;

	// 캐싱된 아이콘 가져오기
	UFUNCTION(BlueprintPure, Category = "Convert")
	UTexture2D* GetCachedIcon(int32 ConvertID) const;

protected:
	UPROPERTY(Config, EditDefaultsOnly, Category = "Convert Data")
	TSoftObjectPtr<class UDataTable> ConvertDataTableRef;

	UPROPERTY()
	TObjectPtr<class UDataTable> ConvertDataTable;

	TArray<FOZConvertData> ConvertCache;
	TMap<int32, FOZConvertData*> ConvertMap;

	// 로드된 에셋 캐시 (GC 방지용)
	UPROPERTY()
	TMap<int32, TSubclassOf<UGameplayEffect>> CachedEffectClasses;

	UPROPERTY()
	TMap<int32, TObjectPtr<UTexture2D>> CachedIcons;
};
