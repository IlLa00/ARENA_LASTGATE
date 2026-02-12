#include "Object/OZPuppetAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UOZPuppetAttributeSet::UOZPuppetAttributeSet()
{
	InitMaxHealth(1000000.f);
	InitHealth(GetMaxHealth());
}

void UOZPuppetAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		const float Max = FMath::Max(GetMaxHealth(), 0.0f);
		NewValue = FMath::Clamp(NewValue, 0.0f, Max);
	}
}

void UOZPuppetAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (Damage <= 0.f)
			return;

		const float OldHealth = GetHealth();
		const float NewHealth = FMath::Clamp(OldHealth - Damage, 0.f, GetMaxHealth());
		SetHealth(NewHealth);

		const float Applied = OldHealth - NewHealth;

		if (Applied > 0.f)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC && ASC->GetOwnerRole() == ROLE_Authority)
			{
				FGameplayCueParameters Params;
				Params.RawMagnitude = Applied;
				Params.EffectContext = Data.EffectSpec.GetEffectContext();

				static const FGameplayTag DamageCueTag =
					FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.UI.DamageNumber"));

				ASC->ExecuteGameplayCue(DamageCueTag, Params);
			}
		}
	}
	else if (Attr == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		SetHealth(NewHealth);
	}
}

void UOZPuppetAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPuppetAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPuppetAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UOZPuppetAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{

}

void UOZPuppetAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{

}