#include "Character/Weapon/OZWeaponAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Character/OZPlayer.h"
#include "GameplayEffectExtension.h"

UOZWeaponAttributeSet::UOZWeaponAttributeSet()
{
	// ** Gun **
	// DPS
	InitBaseDamage(100.f);
	InitBaseAtkDelay(0.7f);
	
	// OverHeat
	InitMaxHeat(100.f);
	InitCurrentHeat(0.f);
	InitCooling(15.f);
	InitCoolingDelay(0.5f);
	InitCoolingBuffer(0.5f);
	InitHeatCoefficient(20.f);
	InitOverheatPenalty(2.f);

	// Spray
	InitBaseSpreadAngle(4.f);

	// ** Projectile **
	// Base Attribute
	InitBaseRange(3000.f);
	InitBaseProjectileSpeed(500.f);  
	InitBaseProjectilesPerShot(1.f);
	InitBaseProjectileSize(2.f);

	// Special Attribute
	InitBaseKnockbackPower(500.f);
	InitMaxRicochetCount(0.f);
	InitExplosionRadius(150.f);
	InitExplosionDamageMult(0.5f);
	InitExplosionFalloffRate(1.f);

	// Damage
	InitDamageMulti(0.f);
	InitDamageAmpMulti(0.f);
	InitOutgoingDamage(0.f);

	// Noise
	InitBaseNoise(100.f);
	InitNoiseMaxRange(1500.f);
	InitNoiseFalloffRate(1.f);
}

void UOZWeaponAttributeSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseDamage, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseAtkDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseAtkDelay, OldValue);
}

void UOZWeaponAttributeSet::OnRep_MaxHeat(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, MaxHeat, OldValue);
}

void UOZWeaponAttributeSet::OnRep_CurrentHeat(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, CurrentHeat, OldValue);
}

void UOZWeaponAttributeSet::OnRep_Cooling(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, Cooling, OldValue);
}

void UOZWeaponAttributeSet::OnRep_CoolingDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, CoolingDelay, OldValue);
}

void UOZWeaponAttributeSet::OnRep_CoolingBuffer(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, CoolingBuffer, OldValue);
}

void UOZWeaponAttributeSet::OnRep_HeatCoefficient(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, HeatCoefficient, OldValue);
}

void UOZWeaponAttributeSet::OnRep_OverheatPenalty(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, OverheatPenalty, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseRange(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseRange, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseProjectileSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseProjectileSpeed, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseProjectilesPerShot(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseProjectilesPerShot, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseSpreadAngle(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseSpreadAngle, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseKnockbackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseKnockbackPower, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseProjectileSize(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseProjectileSize, OldValue);
}

void UOZWeaponAttributeSet::OnRep_MaxRicochetCount(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, MaxRicochetCount, OldValue);
}

void UOZWeaponAttributeSet::OnRep_ExplosionRadius(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, ExplosionRadius, OldValue);
}

void UOZWeaponAttributeSet::OnRep_ExplosionDamageMult(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, ExplosionDamageMult, OldValue);
}

void UOZWeaponAttributeSet::OnRep_ExplosionFalloffRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, ExplosionFalloffRate, OldValue);
}

void UOZWeaponAttributeSet::OnRep_DamageMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, DamageMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_DamageAmpMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, DamageAmpMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_AtkMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, AtkMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_AtkAmpMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, AtkAmpMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_SpreadAngleMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, AtkAmpMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_SpreadAngleAmpMulti(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, AtkAmpMulti, OldValue);
}

void UOZWeaponAttributeSet::OnRep_BaseNoise(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, BaseNoise, OldValue);
}

void UOZWeaponAttributeSet::OnRep_NoiseMaxRange(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, NoiseMaxRange, OldValue);
}

void UOZWeaponAttributeSet::OnRep_NoiseFalloffRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZWeaponAttributeSet, NoiseFalloffRate, OldValue);
}


void UOZWeaponAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseAtkDelay, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, MaxHeat, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, CurrentHeat, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, Cooling, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, CoolingDelay, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, CoolingBuffer, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, HeatCoefficient, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, OverheatPenalty, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseSpreadAngle, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, SpreadAngleMulti, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, SpreadAngleAmpMulti, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseRange, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseProjectileSpeed, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseProjectilesPerShot, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseKnockbackPower, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseProjectileSize, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, MaxRicochetCount, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, ExplosionRadius, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, ExplosionDamageMult, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, ExplosionFalloffRate, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, DamageMulti, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, DamageAmpMulti, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, AtkMulti, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, AtkAmpMulti, COND_OwnerOnly, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, BaseNoise, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, NoiseMaxRange, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZWeaponAttributeSet, NoiseFalloffRate, COND_OwnerOnly, REPNOTIFY_Always);
}

void UOZWeaponAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetMaxHeatAttribute())
	{
		float NewMax = GetMaxHeat();
		NewMax = FMath::Max(NewMax, 0.f);
		SetMaxHeat(NewMax);

		float Current = GetCurrentHeat();
		if (Current > NewMax)
		{
			SetCurrentHeat(NewMax);
		}
	}

	if (Attr == GetCurrentHeatAttribute())
	{
		float Heat = GetCurrentHeat();
		const float Max = FMath::Max(GetMaxHeat(), 0.f);

		Heat = FMath::Clamp(Heat, 0.f, Max);
		SetCurrentHeat(Heat);

		if (AActor* Avatar = Data.Target.AbilityActorInfo->AvatarActor.Get())
		{
			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				/*if (!Player->IsLocallyControlled())
					return;*/

				float maxHeat = GetMaxHeat();
				float heatRatio = Heat / maxHeat;

				Player->OnWeaponHeatChanged(heatRatio);
			}
		}
	}
}
