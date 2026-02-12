#include "Subsystem/OZModuleSubsystem.h"
#include "Data/OZModuleData.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"

void UOZModuleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ModuleDataTableRef.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Config was NULL, using hardcoded path"));
		ModuleDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Module.DT_Module"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] Config loaded from DefaultEngine.ini"));
	}

	ModuleDataTable = ModuleDataTableRef.LoadSynchronous();

	if (!ModuleDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] Failed to load DataTable! Check if DT_Module exists"));
		return;
	}

	TArray<FName> RowNames = ModuleDataTable->GetRowNames();

	ModuleCache.Reserve(RowNames.Num());

	for (const FName& RowName : RowNames)
	{
		FOZModuleData* Data = ModuleDataTable->FindRow<FOZModuleData>(RowName, TEXT(""));
		if (Data)
		{
			// 먼저 캐시에 복사
			int32 Index = ModuleCache.Add(*Data);
			FOZModuleData& CachedData = ModuleCache[Index];
			ModuleMap.Add(CachedData.Module_ID, &CachedData);

			// 복사본에서 로드해야 Get()이 동작함
			// Module_Asset (GameplayEffect) 로드 및 캐싱
			if (!CachedData.Module_Asset.IsNull())
			{
				UClass* LoadedClass = CachedData.Module_Asset.LoadSynchronous();
				if (!LoadedClass)
				{
					UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] Failed to load GameplayEffect for Module ID %d (%s) - Path: %s"),
						CachedData.Module_ID, *CachedData.name.ToString(), *CachedData.Module_Asset.ToString());
				}
				else
				{
					CachedEffectClasses.Add(CachedData.Module_ID, LoadedClass);
					UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] Loaded and cached GameplayEffect for Module ID %d: %s"),
						CachedData.Module_ID, *LoadedClass->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Module ID %d (%s) has no GameplayEffect asset assigned!"),
					CachedData.Module_ID, *CachedData.name.ToString());
			}

			// Icon 로드 및 캐싱
			if (!CachedData.Icon.IsNull())
			{
				UTexture2D* LoadedIcon = CachedData.Icon.LoadSynchronous();
				if (!LoadedIcon)
				{
					UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] Failed to load Icon for Module ID %d (%s) - Path: %s"),
						CachedData.Module_ID, *CachedData.name.ToString(), *CachedData.Icon.ToString());
				}
				else
				{
					CachedIcons.Add(CachedData.Module_ID, LoadedIcon);
					UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] Loaded and cached Icon for Module ID %d"), CachedData.Module_ID);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Module ID %d (%s) has no Icon assigned!"),
					CachedData.Module_ID, *CachedData.name.ToString());
			}
		}
	}
}

FGameplayEffectSpecHandle UOZModuleSubsystem::GetModuleEffectSpec(int32 ModuleID, UAbilitySystemComponent* SourceASC, int32 CurrentRound)
{
	if (!SourceASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] SourceASC is null!"));
		return FGameplayEffectSpecHandle();
	}

	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (!ModuleData)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] Module ID %d not found in cache!"), ModuleID);
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] Available Module IDs in cache:"));
		for (const auto& Pair : ModuleMap)
		{
			UE_LOG(LogTemp, Error, TEXT("  - Module ID: %d"), Pair.Key);
		}
		return FGameplayEffectSpecHandle();
	}

	// TSoftClassPtr에서 실제 클래스 가져오기
	TSubclassOf<UGameplayEffect> EffectClass = ModuleData->Module_Asset.Get();

	if (!EffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Module ID %d has no GameplayEffect assigned or failed to load!"), ModuleID);
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
		EffectClass, 1.0f, Context
	);

	if (!SpecHandle.IsValid())
		return FGameplayEffectSpecHandle();

	// FinalValue 계산
	float FinalValue = CalculateFinalValue(*ModuleData, CurrentRound);

	FGameplayTag DataTag = GetTagFromAttributeName(ModuleData->Module_ID);

	if (DataTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DataTag, FinalValue);
		UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] Module %s FinalValue: %.1f (Round: %d)"),
			*ModuleData->name.ToString(), FinalValue, CurrentRound);
	}

	return SpecHandle;
}

FActiveGameplayEffectHandle UOZModuleSubsystem::ApplyModuleToSelf(int32 ModuleID, UAbilitySystemComponent* TargetASC, int32 CurrentRound)
{
	if (!TargetASC)
		return FActiveGameplayEffectHandle();

	if (!TargetASC->IsOwnerActorAuthoritative())
		return FActiveGameplayEffectHandle();

	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (!ModuleData)
		return FActiveGameplayEffectHandle();

	TSubclassOf<UGameplayEffect> EffectClass = ModuleData->Module_Asset.Get();
	if (!EffectClass)
		return FActiveGameplayEffectHandle();

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
	if (!SpecHandle.IsValid())
		return FActiveGameplayEffectHandle();

	// 캐시된 FinalValue 사용
	float FinalValue = GetCachedFinalValue(ModuleID);

	// FinalValue가 0인 경우 경고
	if (FMath::IsNearlyZero(FinalValue))
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] ApplyModuleToSelf - Module %s FinalValue is 0! Was CalculateAndCacheAllModuleFinalValues called and synced?"),
			*ModuleData->name.ToString());
	}

	FGameplayTag DataTag = GetTagFromAttributeName(ModuleData->Module_ID);
	if (DataTag.IsValid())
		SpecHandle.Data->SetSetByCallerMagnitude(DataTag, FinalValue);
	else
		return FActiveGameplayEffectHandle();

	FActiveGameplayEffectHandle Handle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (Handle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] ApplyModuleToSelf - Successfully applied Module %s with FinalValue: %.1f"),
			*ModuleData->name.ToString(), FinalValue);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] ApplyModuleToSelf - Failed to apply Module %s!"), *ModuleData->name.ToString());

	return Handle;
}

FActiveGameplayEffectHandle UOZModuleSubsystem::ApplyGameEffect(int32 ModuleID, UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] ApplyGameEffect - TargetASC is null!"));
		return FActiveGameplayEffectHandle();
	}

	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (!ModuleData)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] ApplyGameEffect - Module ID %d not found!"), ModuleID);
		return FActiveGameplayEffectHandle();
	}

	TSubclassOf<UGameplayEffect> EffectClass = ModuleData->Module_Asset.Get();
	if (!EffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] ApplyGameEffect - Module %s has no GameplayEffect!"), *ModuleData->name.ToString());
		return FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[OZModuleSubsystem] ApplyGameEffect - Failed to create SpecHandle!"));
		return FActiveGameplayEffectHandle();
	}

	float MinValue = ModuleData->ValueMin;

	FGameplayTag DataTag = GetTagFromAttributeName(ModuleData->Module_ID);
	if (DataTag.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DataTag, MinValue);
		UE_LOG(LogTemp, Log, TEXT("[OZModuleSubsystem] ApplyGameEffect (ShootingRange) - Module %s MinValue: %.1f"),
			*ModuleData->name.ToString(), MinValue);
	}

	return TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

float UOZModuleSubsystem::CalculateFinalValue(const FOZModuleData& ModuleData, int32 CurrentRound) const
{
	int32 X = CurrentRound / 2;

	float FinalScaleMulti = 1.0f + (X * ModuleData.ScaleMulti);

	float RawMin = FinalScaleMulti * ModuleData.ValueMin;
	float RawMax = FinalScaleMulti * ModuleData.ValueMax;

	float ValueStep = ModuleData.ValueStep;
	if (ValueStep <= 0.f)
	{
		ValueStep = 1.f;
	}

	float CurrentValueMin = FMath::RoundToFloat(RawMin / ValueStep) * ValueStep;
	float CurrentValueMax = FMath::RoundToFloat(RawMax / ValueStep) * ValueStep;

	int32 StepCount = FMath::RoundToInt((CurrentValueMax - CurrentValueMin) / ValueStep);
	int32 RandomStepIndex = FMath::RandRange(0, StepCount);
	float FinalValue = CurrentValueMin + (RandomStepIndex * ValueStep);

	return FinalValue;
}

void UOZModuleSubsystem::CalculateScaledValues(FOZModuleData& ModuleData, int32 CurrentRound) const
{
	int32 X = CurrentRound / 2;

	float FinalScaleMulti = 1.0f + (X * ModuleData.ScaleMulti);

	float RawMin = FinalScaleMulti * ModuleData.ValueMin;
	float RawMax = FinalScaleMulti * ModuleData.ValueMax;

	float ValueStep = ModuleData.ValueStep;
	if (ValueStep <= 0.f)
	{
		ValueStep = 1.f;
	}

	float CurrentValueMin = FMath::RoundToFloat(RawMin / ValueStep) * ValueStep;
	float CurrentValueMax = FMath::RoundToFloat(RawMax / ValueStep) * ValueStep;

	ModuleData.ScaledMinValue = CurrentValueMin;
	ModuleData.ScaledMaxValue = CurrentValueMax;

	// FinalValue 계산
	int32 StepCount = FMath::RoundToInt((CurrentValueMax - CurrentValueMin) / ValueStep);
	int32 RandomStepIndex = FMath::RandRange(0, StepCount);

	ModuleData.FinalValue = CurrentValueMin + (RandomStepIndex * ValueStep);
}

FOZModuleData* UOZModuleSubsystem::FindModule(int32 ModuleID) const
{
	FOZModuleData* const* Found = ModuleMap.Find(ModuleID);
	if (Found)
	{
		return *Found;
	}
	return nullptr;
}

FOZModuleData* UOZModuleSubsystem::GetModuleData(int32 ModuleID) const
{
	return FindModule(ModuleID);
}

FOZModuleData UOZModuleSubsystem::GetModuleDataFromID(int32 ModuleID) const
{
	FOZModuleData* Data = FindModule(ModuleID);
	if (Data)
	{
		return *Data;
	}
	return FOZModuleData();
}

void UOZModuleSubsystem::CalculateAndCacheAllModuleFinalValues(int32 CurrentRound)
{
	for (FOZModuleData& ModuleData : ModuleCache)
	{
		CalculateScaledValues(ModuleData, CurrentRound);
	}
}

float UOZModuleSubsystem::GetCachedFinalValue(int32 ModuleID) const
{
	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (ModuleData)
		return ModuleData->FinalValue;

	return 0.0f;
}

void UOZModuleSubsystem::SetCachedFinalValue(int32 ModuleID, float NewFinalValue)
{
	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (ModuleData)
	{
		ModuleData->FinalValue = NewFinalValue;
	}
}

void UOZModuleSubsystem::SetCachedMinValue(int32 ModuleID, float NewMinValue)
{
	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (ModuleData)
	{
		ModuleData->ScaledMinValue = NewMinValue;
	}
}

void UOZModuleSubsystem::SetCachedMaxValue(int32 ModuleID, float NewMaxValue)
{
	FOZModuleData* ModuleData = FindModule(ModuleID);
	if (ModuleData)
	{
		ModuleData->ScaledMaxValue = NewMaxValue;
	}
}

void UOZModuleSubsystem::GetAllCachedFinalValues(TArray<int32>& OutModuleIDs, TArray<float>& OutFinalValues) const
{
	OutModuleIDs.Empty();
	OutFinalValues.Empty();
	for (const FOZModuleData& Data : ModuleCache)
	{
		OutModuleIDs.Add(Data.Module_ID);
		OutFinalValues.Add(Data.FinalValue);
	}
}

void UOZModuleSubsystem::GetAllCachedMinValues(TArray<int32>& OutModuleIDs, TArray<float>& OutMinValues) const
{
	OutModuleIDs.Empty();
	OutMinValues.Empty();
	for (const FOZModuleData& Data : ModuleCache)
	{
		OutModuleIDs.Add(Data.Module_ID);
		OutMinValues.Add(Data.ScaledMinValue);
	}
}

void UOZModuleSubsystem::GetAllCachedMaxValues(TArray<int32>& OutModuleIDs, TArray<float>& OutMaxValues) const
{
	OutModuleIDs.Empty();
	OutMaxValues.Empty();
	for (const FOZModuleData& Data : ModuleCache)
	{
		OutModuleIDs.Add(Data.Module_ID);
		OutMaxValues.Add(Data.ScaledMaxValue);
	}
}

FOZModuleData UOZModuleSubsystem::GetRandomModule(const TArray<int32>& ExcludeIDs, bool& bSuccess) const
{
	TArray<FOZModuleData> AvailableModules;

	// 제외할 ID 목록을 제거
	for (const FOZModuleData& Module : ModuleCache)
	{
		if (!ExcludeIDs.Contains(Module.Module_ID))
		{
			AvailableModules.Add(Module);
		}
	}

	if (AvailableModules.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] No available modules found (all excluded)"));
		bSuccess = false;
		return FOZModuleData();
	}

	// 총 가중치 계산
	float TotalWeight = 0.f;
	for (const FOZModuleData& Module : AvailableModules)
	{
		TotalWeight += Module.weight;
	}

	// 가중치 기반 랜덤 선택
	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	float AccumulatedWeight = 0.f;

	for (const FOZModuleData& Module : AvailableModules)
	{
		AccumulatedWeight += Module.weight;
		if (RandomValue <= AccumulatedWeight)
		{
			bSuccess = true;
			return Module;
		}
	}

	// fallback (도달하지 않아야 함)
	bSuccess = true;
	return AvailableModules.Last();
}

FGameplayTag UOZModuleSubsystem::GetTagFromAttributeName(const int32 ID) const
{
	FString TagName;

	switch (ID)
	{
	case 40101:
		TagName = TEXT("Module.HP");
		break;
	case 40102:
		TagName = TEXT("Module.Stamina");
		break;
	case 40103:
		TagName = TEXT("Module.WalkSpeed");
		break;
	case 40104:
		TagName = TEXT("Module.SprintSpeed");
		break;
	case 40105:
		TagName = TEXT("Module.EVLDistance");
		break;
	case 40106:
		TagName = TEXT("Module.Shield");
		break;
	case 40107:
		TagName = TEXT("Module.Armor");
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Unknown Module ID: %d"), ID);
		return FGameplayTag();
	}

	FGameplayTag ResultTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);

	if (!ResultTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZModuleSubsystem] Failed to get tag for ID %d (%s)"), ID, *TagName);
	}

	return ResultTag;
}
