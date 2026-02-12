// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "OZPlayerAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

struct FGameplayEffectModCallbackData;

USTRUCT(BlueprintType)
struct FStatusRatioData
{
	GENERATED_BODY()

	FStatusRatioData()
	{
		CurrentAmount = -1.0f;
		MaxAmount = -1.0f;
	}

	FStatusRatioData(float curr, float max)
	{
		CurrentAmount = curr;
		MaxAmount = max;
	}

	UPROPERTY(BlueprintReadWrite)
	float CurrentAmount;

	UPROPERTY(BlueprintReadWrite)
	float MaxAmount;
};

UCLASS()
class ARENA_LASTGATE_API UOZPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
    UOZPlayerAttributeSet();

	void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Health 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, Health)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Health") // 내부 계산 용이므로 Replication 불필요
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Health") // 내부 계산 용이므로 Replication 불필요
	FGameplayAttributeData IncomingTrueDamage;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, IncomingTrueDamage)

	// Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_CurrentStamina)
	FGameplayAttributeData CurrentStamina;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, CurrentStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_StaminaRegenDelay)
	FGameplayAttributeData StaminaRegenDelay;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, StaminaRegenDelay);

	UPROPERTY(BlueprintReadOnly, Category = "Stamina", ReplicatedUsing = OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, StaminaRegenRate);

	// Speed
	UPROPERTY(BlueprintReadOnly, Category = "Move", ReplicatedUsing = OnRep_WalkSpeed)
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, WalkSpeed);

	UPROPERTY(BlueprintReadOnly, Category = "Move", ReplicatedUsing = OnRep_SprintSpeed)
	FGameplayAttributeData SprintSpeed;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, SprintSpeed);

	UPROPERTY(BlueprintReadOnly, Category = "Move", ReplicatedUsing = OnRep_SprintStaminaCost)
	FGameplayAttributeData SprintStaminaCost;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, SprintStaminaCost);

	// 수정 예정
	UPROPERTY(BlueprintReadOnly, Category = "Move", ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Evade", ReplicatedUsing = OnRep_EvLDistance)
	FGameplayAttributeData EvLDistance;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, EvLDistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Evade", ReplicatedUsing = OnRep_EvDelay)
	FGameplayAttributeData EvDelay;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, EvDelay);

	UPROPERTY(BlueprintReadOnly, Category = "Evade", ReplicatedUsing = OnRep_EvStaminaCost)
	FGameplayAttributeData EvStaminaCost;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, EvStaminaCost);
	
	UPROPERTY(BlueprintReadOnly, Category = "Aiming", ReplicatedUsing = OnRep_AimingDistance)
	FGameplayAttributeData AimingDistance;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, AimingDistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Aiming", ReplicatedUsing = OnRep_AimingCameraDelay)
	FGameplayAttributeData AimingCameraDelay;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, AimingCameraDelay);
	
	UPROPERTY(BlueprintReadOnly, Category = "Aiming", ReplicatedUsing = OnRep_AimingMoveSpeed)
	FGameplayAttributeData AimingMoveSpeed;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, AimingMoveSpeed);
	
	// Shield & Armor
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, Shield)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, MaxShield)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_ShieldRegenTime)
	FGameplayAttributeData ShieldRegenTime;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, ShieldRegenTime)

	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_ShieldRegenRate)
	FGameplayAttributeData ShieldRegenRate;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, ShieldRegenRate)
		
	UPROPERTY(BlueprintReadOnly, Category = "Shield", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, Armor)

	UPROPERTY(BlueprintReadOnly, Category = "State", ReplicatedUsing = OnRep_HitMoveSpeed)
	FGameplayAttributeData HitMoveSpeed;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, HitMoveSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "State", ReplicatedUsing = OnRep_HitMoveSpeedTime)
	FGameplayAttributeData HitMoveSpeedTime;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, HitMoveSpeedTime)

	UPROPERTY(BlueprintReadOnly, Category = "Camera", ReplicatedUsing = OnRep_ViewingAngle)
	FGameplayAttributeData ViewingAngle;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, ViewingAngle)

	UPROPERTY(BlueprintReadOnly, Category = "Camera", ReplicatedUsing = OnRep_CameraDepth)
	FGameplayAttributeData CameraDepth;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, CameraDepth)

	UPROPERTY(BlueprintReadOnly, Category = "Camera", ReplicatedUsing = OnRep_PeripharalVision)
	FGameplayAttributeData PeripharalVision;
	ATTRIBUTE_ACCESSORS(UOZPlayerAttributeSet, PeripharalVision)
		
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CurrentStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaminaRegenDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SprintSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SprintStaminaCost(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_EvLDistance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_EvDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_EvStaminaCost(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AimingDistance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AimingCameraDelay(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AimingMoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ShieldRegenTime(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ShieldRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HitMoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HitMoveSpeedTime(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ViewingAngle(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CameraDepth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_PeripharalVision(const FGameplayAttributeData& OldValue);

private:
	void UpdateHealthNotFullTag(const FGameplayEffectModCallbackData& Data);
};
