#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "OZWeaponAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ARENA_LASTGATE_API UOZWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UOZWeaponAttributeSet();

public:
	// ** Gun ** 
	// DPS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|DPS", ReplicatedUsing = OnRep_BaseDamage)
	FGameplayAttributeData BaseDamage;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseDamage)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|DPS", ReplicatedUsing = OnRep_BaseAtkDelay)
	FGameplayAttributeData BaseAtkDelay;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseAtkDelay)

	// OverHeat
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_MaxHeat)
	FGameplayAttributeData MaxHeat;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, MaxHeat)

	// ���� �� ������ (0 ~ MaxHeat)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_CurrentHeat)
	FGameplayAttributeData CurrentHeat;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, CurrentHeat)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_Cooling)
	FGameplayAttributeData Cooling;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, Cooling)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_CoolingDelay)
	FGameplayAttributeData CoolingDelay;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, CoolingDelay)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_CoolingBuffer)
	FGameplayAttributeData CoolingBuffer;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, CoolingBuffer)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_HeatCoefficient)
	FGameplayAttributeData HeatCoefficient;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, HeatCoefficient)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|OverHeat", ReplicatedUsing = OnRep_OverheatPenalty)
	FGameplayAttributeData OverheatPenalty;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, OverheatPenalty)

	// Spray
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Spray", ReplicatedUsing = OnRep_BaseSpreadAngle)
	FGameplayAttributeData BaseSpreadAngle;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseSpreadAngle)
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Spray", ReplicatedUsing = OnRep_SpreadAngleMulti)
	FGameplayAttributeData SpreadAngleMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, SpreadAngleMulti)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Spray", ReplicatedUsing = OnRep_SpreadAngleAmpMulti)
	FGameplayAttributeData SpreadAngleAmpMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, SpreadAngleAmpMulti)

	// ** Projectile ** 
	// Base Attributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Base", ReplicatedUsing = OnRep_BaseRange)
	FGameplayAttributeData BaseRange;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseRange)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Base", ReplicatedUsing = OnRep_BaseProjectileSpeed)
	FGameplayAttributeData BaseProjectileSpeed;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseProjectileSpeed)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Base", ReplicatedUsing = OnRep_BaseProjectilesPerShot)
	FGameplayAttributeData BaseProjectilesPerShot;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseProjectilesPerShot)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Base", ReplicatedUsing = OnRep_BaseProjectileSize)
	FGameplayAttributeData BaseProjectileSize;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseProjectileSize)

	// Special Attributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Special", ReplicatedUsing = OnRep_BaseKnockbackPower)
	FGameplayAttributeData BaseKnockbackPower;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseKnockbackPower)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Special", ReplicatedUsing = OnRep_MaxRicochetCount)
	FGameplayAttributeData MaxRicochetCount;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, MaxRicochetCount)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Special", ReplicatedUsing = OnRep_ExplosionRadius)
	FGameplayAttributeData ExplosionRadius;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, ExplosionRadius)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Special", ReplicatedUsing = OnRep_ExplosionDamageMult)
	FGameplayAttributeData ExplosionDamageMult;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, ExplosionDamageMult)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile|Special", ReplicatedUsing = OnRep_ExplosionFalloffRate)
	FGameplayAttributeData ExplosionFalloffRate;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, ExplosionFalloffRate)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Damage", ReplicatedUsing = OnRep_DamageMulti)
	FGameplayAttributeData DamageMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, DamageMulti)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Damage", ReplicatedUsing = OnRep_DamageAmpMulti)
	FGameplayAttributeData DamageAmpMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, DamageAmpMulti)

	UPROPERTY(BlueprintReadOnly, Category = "Gun|Damage")
	FGameplayAttributeData OutgoingDamage;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, OutgoingDamage)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|AtkDelay", ReplicatedUsing = OnRep_AtkMulti)
	FGameplayAttributeData AtkMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, AtkMulti)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|AtkDelay", ReplicatedUsing = OnRep_AtkAmpMulti)
	FGameplayAttributeData AtkAmpMulti;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, AtkAmpMulti)

	// Noise
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Noise", ReplicatedUsing = OnRep_BaseNoise)
	FGameplayAttributeData BaseNoise;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, BaseNoise)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Noise", ReplicatedUsing = OnRep_NoiseMaxRange)
	FGameplayAttributeData NoiseMaxRange;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, NoiseMaxRange)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Noise", ReplicatedUsing = OnRep_NoiseFalloffRate)
	FGameplayAttributeData NoiseFalloffRate;
	ATTRIBUTE_ACCESSORS(UOZWeaponAttributeSet, NoiseFalloffRate)


	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseAtkDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHeat(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentHeat(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Cooling(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CoolingDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CoolingBuffer(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HeatCoefficient(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_OverheatPenalty(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseRange(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseProjectileSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseProjectilesPerShot(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseSpreadAngle(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseKnockbackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseProjectileSize(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxRicochetCount(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ExplosionRadius(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ExplosionDamageMult(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ExplosionFalloffRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageMulti(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_DamageAmpMulti(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AtkMulti(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AtkAmpMulti(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SpreadAngleMulti(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SpreadAngleAmpMulti(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_BaseNoise(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_NoiseMaxRange(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_NoiseFalloffRate(const FGameplayAttributeData& OldValue);


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

};
