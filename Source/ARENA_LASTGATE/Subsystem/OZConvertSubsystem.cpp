#include "Subsystem/OZConvertSubsystem.h"
#include "Data/OZConvertData.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"
#include "Character/OZPlayerState.h"

void UOZConvertSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DataTable 로드
	if (ConvertDataTableRef.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] Config was NULL, using hardcoded path"));
		ConvertDataTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_Convert.DT_Convert"));
	}

	ConvertDataTable = ConvertDataTableRef.LoadSynchronous();

	if (!ConvertDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Failed to load Convert DataTable!"));
		return;
	}

	// DataTable 캐싱
	TArray<FName> RowNames = ConvertDataTable->GetRowNames();
	ConvertCache.Reserve(RowNames.Num());

	for (const FName& RowName : RowNames)
	{
		FOZConvertData* Data = ConvertDataTable->FindRow<FOZConvertData>(RowName, TEXT(""));
		if (Data)
		{
			// 먼저 캐시에 복사
			int32 Index = ConvertCache.Add(*Data);
			FOZConvertData& CachedData = ConvertCache[Index];
			ConvertMap.Add(CachedData.ID, &CachedData);

			// 복사본에서 로드해야 Get()이 동작함
			// Convert_Asset (GameplayEffect) 로드 및 캐싱
			if (!CachedData.Convert_Asset.IsNull())
			{
				UClass* LoadedClass = CachedData.Convert_Asset.LoadSynchronous();
				if (!LoadedClass)
				{
					UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Failed to load GameplayEffect for Convert ID %d (%s) - Path: %s"),
						CachedData.ID, *CachedData.Name.ToString(), *CachedData.Convert_Asset.ToString());
				}
				else
				{
					CachedEffectClasses.Add(CachedData.ID, LoadedClass);
					UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Loaded and cached GameplayEffect for Convert ID %d: %s"),
						CachedData.ID, *LoadedClass->GetName());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] Convert ID %d (%s) has no GameplayEffect asset assigned!"),
					CachedData.ID, *CachedData.Name.ToString());
			}

			// Icon 로드 및 캐싱
			if (!CachedData.Icon.IsNull())
			{
				UTexture2D* LoadedIcon = CachedData.Icon.LoadSynchronous();
				if (!LoadedIcon)
				{
					UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Failed to load Icon for Convert ID %d (%s) - Path: %s"),
						CachedData.ID, *CachedData.Name.ToString(), *CachedData.Icon.ToString());
				}
				else
				{
					CachedIcons.Add(CachedData.ID, LoadedIcon);
					UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Loaded and cached Icon for Convert ID %d"), CachedData.ID);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] Convert ID %d (%s) has no Icon assigned!"),
					CachedData.ID, *CachedData.Name.ToString());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Loaded %d converts"), ConvertCache.Num());
}

FActiveGameplayEffectHandle UOZConvertSubsystem::ApplyConvertToSelf(
	int32 ConvertID,
	UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] TargetASC is null!"));
		return FActiveGameplayEffectHandle();
	}

	// GameplayEffect는 서버에서만 적용되어야 함
	if (!TargetASC->IsOwnerActorAuthoritative())
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] ApplyConvertToSelf must be called on server! Owner: %s"),
			TargetASC->GetOwnerActor() ? *TargetASC->GetOwnerActor()->GetName() : TEXT("None"));
		return FActiveGameplayEffectHandle();
	}

	FOZConvertData* ConvertData = FindConvert(ConvertID);
	if (!ConvertData)
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Convert ID %d not found!"), ConvertID);
		return FActiveGameplayEffectHandle();
	}

	TSubclassOf<UGameplayEffect> EffectClass = ConvertData->Convert_Asset.Get();

	if (!EffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] Convert ID %d has no GameplayEffect assigned!"), ConvertID);
		return FActiveGameplayEffectHandle();
	}

	UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Applying Convert ID %d: %s"), ConvertID, *ConvertData->Name.ToString());

	// Context 생성
	FGameplayEffectContextHandle Context = TargetASC->MakeEffectContext();
	Context.AddSourceObject(this);

	// Spec 생성
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);

	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Failed to create GameplayEffect Spec!"));
		return FActiveGameplayEffectHandle();
	}

	// SetByCallerMagnitude로 모든 속성 설정 (0이 아닌 것만)

	// Damage 관련
	if (ConvertData->DamageAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseDamage"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->DamageAdd));
	}

	if (ConvertData->DamageMulti != 0.0f)
	{
		// DataTable의 값은 이미 Delta (예: 0.1 = 10% 증가)
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.DamageMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->DamageMulti);
	}

	if (ConvertData->DamageAmpMulti != 0.0f)
	{
		// DataTable의 값은 이미 Delta (예: 0.2 = 20% 증가)
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.DamageAmpMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->DamageAmpMulti);
	}

	// Attack Delay
	if (ConvertData->AtkDelayMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.AtkDelayMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->AtkDelayMulti);
	}

	if (ConvertData->AtkDelayAmpMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.AtkDelayAmp"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->AtkDelayAmpMulti);
	}

	// Heat System
	if (ConvertData->MaxHeatAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.MaxHeat"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->MaxHeatAdd));
	}

	if (ConvertData->CoolingAdd != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.Cooling"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->CoolingAdd);
	}

	if (ConvertData->HeatCoefficientMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.HeatCoefficient"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, 1 + (ConvertData->HeatCoefficientMulti));
	}

	// Spread Angle
	if (ConvertData->SpreadAngleMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.SpreadAngleMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->SpreadAngleMulti);
	}

	if (ConvertData->AmpSpreadAngleMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.SpreadAngleAmp"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->AmpSpreadAngleMulti);
	}

	// Noise (속성 추가 필요)
	if (ConvertData->NoiseMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.Noise"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->NoiseMulti);
	}

	if (ConvertData->AmpNoiseMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.NoiseAmp"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->AmpNoiseMulti);
	}

	if (ConvertData->NoiseMaxRangeMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.NoiseMaxRange"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->NoiseMaxRangeMulti);
	}

	// Range
	if (ConvertData->RangeAdd != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseRange"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->RangeAdd);
	}

	if (ConvertData->RangeMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseRange"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, 1 + (ConvertData->RangeMulti));

	}

	// Projectile Speed
	if (ConvertData->ProjectileSpeedAdd != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseProjectileSpeed"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->ProjectileSpeedAdd);
	}

	if (ConvertData->ProjectileSpeedMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseProjectileSpeed"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, 1 + (ConvertData->ProjectileSpeedMulti));
	}

	// Projectile Size
	if (ConvertData->ProjectileSizeAdd != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseProjectileSize"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->ProjectileSizeAdd);
	}

	// Projectiles Per Shot
	if (ConvertData->ProjectilesPershotAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseProjectilesPerShot"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->ProjectilesPershotAdd));
	}

	// Knockback
	if (ConvertData->KnockbackPowerAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.BaseKnockbackPower"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->KnockbackPowerAdd));
	}

	if (ConvertData->BaseKnockbackPowerMulti != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.KnockbackPowerMulti"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, 1 + (ConvertData->BaseKnockbackPowerMulti));
	}

	// Ricochet
	if (ConvertData->RicochetConutAdd != 0)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.MaxRicochetCount"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, static_cast<float>(ConvertData->RicochetConutAdd));

		FGameplayTag RicochetTag = FGameplayTag::RequestGameplayTag(FName("Weapon.State.CanRicochet"));
		TargetASC->AddLooseGameplayTag(RicochetTag);
	}

	// Explosion
	if (ConvertData->ExplosionRadiusAdd != 0.0f)
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Convert.ExplosionRadius"));
		SpecHandle.Data->SetSetByCallerMagnitude(Tag, ConvertData->ExplosionRadiusAdd);
	}

	// 적용
	FActiveGameplayEffectHandle Handle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (Handle.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Successfully applied Convert ID %d"), ConvertID);

		AActor* OwnerActor = TargetASC->GetOwnerActor();
		if (OwnerActor)
		{
			if (AOZPlayerState* PS = Cast<AOZPlayerState>(OwnerActor))
			{
				// 유니크/레전더리 등급인 경우 플레이어 이력에 추가
				if (ConvertData->Grade == EConvertGrade::Unique || ConvertData->Grade == EConvertGrade::Legendary)
				{
					PS->AddToConvertHistory(ConvertID, ConvertData->Grade);
				}

				// UI 업데이트를 위해 모든 Convert 획득 알림
				PS->NotifyConvertAcquired(ConvertID, ConvertData->Grade);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[OZConvertSubsystem] Failed to apply Convert ID %d - Handle is invalid!"), ConvertID);
	}

	return Handle;
}

FOZConvertData* UOZConvertSubsystem::FindConvert(int32 ConvertID) const
{
	FOZConvertData* const* Found = ConvertMap.Find(ConvertID);
	if (Found)
	{
		return *Found;
	}
	return nullptr;
}

TArray<FOZConvertData> UOZConvertSubsystem::GetConvertsByGrade(EConvertGrade Grade) const
{
	TArray<FOZConvertData> Result;

	for (const FOZConvertData& Convert : ConvertCache)
	{
		if (Convert.Grade == Grade)
		{
			Result.Add(Convert);
		}
	}

	return Result;
}

FOZConvertData UOZConvertSubsystem::GetRandomConvertByGrade(EConvertGrade Grade, const TArray<int32>& ExcludeIDs, bool& bSuccess) const
{
	TArray<FOZConvertData> ConvertsOfGrade = GetConvertsByGrade(Grade);

	// 제외할 ID 목록을 제거
	TArray<FOZConvertData> AvailableConverts;
	for (const FOZConvertData& Convert : ConvertsOfGrade)
	{
		if (!ExcludeIDs.Contains(Convert.ID))
		{
			AvailableConverts.Add(Convert);
		}
	}

	if (AvailableConverts.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] No available converts found for grade %d (all excluded)"), static_cast<int32>(Grade));
		bSuccess = false;
		return FOZConvertData();
	}

	int32 RandomIndex = FMath::RandRange(0, AvailableConverts.Num() - 1);
	bSuccess = true;
	return AvailableConverts[RandomIndex];
}

FOZConvertData UOZConvertSubsystem::RerollConvert(int32 CurrentConvertID, const TArray<int32>& ExcludeConvertIDs, bool& bSuccess) const
{
	FOZConvertData* CurrentConvert = FindConvert(CurrentConvertID);
	if (!CurrentConvert)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OZConvertSubsystem] RerollConvert: Current convert ID %d not found"), CurrentConvertID);
		bSuccess = false;
		return FOZConvertData();
	}

	TArray<int32> FullExcludeList = ExcludeConvertIDs;
	FullExcludeList.Add(CurrentConvertID);

	FOZConvertData NewConvert = GetRandomConvertByGrade(CurrentConvert->Grade, FullExcludeList, bSuccess);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[OZConvertSubsystem] Rerolled: %s (ID: %d, Grade: %d) -> %s (ID: %d)"),
			*CurrentConvert->Name.ToString(), CurrentConvertID, static_cast<int32>(CurrentConvert->Grade),
			*NewConvert.Name.ToString(), NewConvert.ID);
	}

	return NewConvert;
}

UTexture2D* UOZConvertSubsystem::GetCachedIcon(int32 ConvertID) const
{
	const TObjectPtr<UTexture2D>* Found = CachedIcons.Find(ConvertID);
	if (Found && *Found)
	{
		return *Found;
	}
	return nullptr;
}
