// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OZPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "OZPlayer.h"
#include "OZPlayerState.h"
#include "Tags/OZGameplayTags.h"

UOZPlayerAttributeSet::UOZPlayerAttributeSet()
{

	MoveSpeed = 600.f;
	InitMaxHealth(1000.f);
	InitHealth(GetMaxHealth());

	InitMaxShield(100.f);
	InitShield(GetMaxShield());
	InitShieldRegenTime(5.f);
	InitShieldRegenRate(10.f);

	InitArmor(0.f);

	InitIncomingDamage(0.f);
	InitIncomingTrueDamage(0.f);
}

void UOZPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, CurrentStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, StaminaRegenDelay, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, ShieldRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, ShieldRegenTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, Armor, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, WalkSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, SprintSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, SprintStaminaCost, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, EvLDistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, EvDelay, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, EvStaminaCost, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, AimingDistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, AimingCameraDelay, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, AimingMoveSpeed, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, HitMoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, HitMoveSpeedTime, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, ViewingAngle, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, CameraDepth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOZPlayerAttributeSet, PeripharalVision, COND_None, REPNOTIFY_Always);

}

void UOZPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetCurrentStaminaAttribute())
	{
		const float Max = GetMaxStamina();
		NewValue = FMath::Clamp(NewValue, 0.0f, Max);
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		const float Max = FMath::Max(GetMaxHealth(), 0.0f);
		NewValue = FMath::Clamp(NewValue, 0.0f, Max);
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}

void UOZPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;

	if (Attr == GetCurrentStaminaAttribute())
	{
		float NewStamina = GetCurrentStamina();
		const float Max = GetMaxStamina();

		NewStamina = FMath::Clamp(NewStamina, 0.0f, Max);
		SetCurrentStamina(NewStamina);

		if (Data.Target.AbilityActorInfo.IsValid())
		{
			AActor* OwnerActor = Data.Target.AbilityActorInfo->OwnerActor.Get();
			if (OwnerActor && OwnerActor->HasAuthority())
			{
				OwnerActor->SetNetDormancy(DORM_Awake);
				OwnerActor->ForceNetUpdate();
			}
		}
	}

	else if (Attr == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

		UpdateHealthNotFullTag(Data);

		AActor* Avatar = Data.Target.AbilityActorInfo.IsValid()
			? Data.Target.AbilityActorInfo->AvatarActor.Get()
			: nullptr;

		if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
		{
			Player->OnCharacterHPChanged(GetHealth());

			if (GetHealth() <= 0.f)
			{
				Player->HandleOutOfHealth();
			}
		}
	}
	else if (Attr == GetIncomingDamageAttribute())
	{
		float Damage = GetIncomingDamage();
		float TotalApplied = 0.f;
		SetIncomingDamage(0.f);

		const float OldHealthBeforeDamage = GetHealth();
		if (OldHealthBeforeDamage <= 0.f)
			return;

		if (Damage <= 0.f)
			return;

		const float ArmorValue = GetArmor();
		const float ArmorReductionPct = FMath::Clamp(ArmorValue / 100.f, 0.f, 0.5f); // Max Armor = 50
		float DamageAfterArmor = Damage * (1.f - ArmorReductionPct);

		float ShieldValue = GetShield();
		float Absorb = FMath::Min(ShieldValue, DamageAfterArmor);
		if (Absorb > 0.f)
		{
			float NewShieldValue = FMath::Clamp(ShieldValue - Absorb, 0.f, GetMaxShield());
			SetShield(NewShieldValue);
			DamageAfterArmor -= Absorb;
			TotalApplied += Absorb;

			AActor* Avatar = Data.Target.AbilityActorInfo.IsValid()
				? Data.Target.AbilityActorInfo->AvatarActor.Get()
				: nullptr;

			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				if (Player->HasAuthority())
				{
					Player->Multicast_UpdateShieldEffect(NewShieldValue > 0.f);
				}
			}
		}

		if (DamageAfterArmor > 0.f)
		{
			const float NewHealth =
				FMath::Clamp(GetHealth() - DamageAfterArmor, 0.f, GetMaxHealth());
			SetHealth(NewHealth);
			TotalApplied += DamageAfterArmor;
		}

		if (TotalApplied > 0.f)
		{
			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
			if (ASC && ASC->GetOwnerRole() == ROLE_Authority)
			{
				FGameplayCueParameters Params;
				Params.RawMagnitude = TotalApplied;
				Params.EffectContext = Data.EffectSpec.GetEffectContext();

				static const FGameplayTag DamageCueTag =
					FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.UI.DamageNumber"));

				ASC->ExecuteGameplayCue(DamageCueTag, Params);

				static const FGameplayTag HitShakeCueTag =
					FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Camera.HitShake"));

				ASC->ExecuteGameplayCue(HitShakeCueTag, Params);
			}
		}

		UpdateHealthNotFullTag(Data);

		AActor* Avatar = Data.Target.AbilityActorInfo.IsValid()
			? Data.Target.AbilityActorInfo->AvatarActor.Get()
			: nullptr;

		if (Avatar)
		{
			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				if (AOZPlayerState* PS = Player->GetPlayerState<AOZPlayerState>())
				{
					PS->NotifyDamaged();

					float currSheildRatio = GetShield() / GetMaxShield();
					Player->OnCharacterSheildChanged(currSheildRatio);
				}

				Player->OnCharacterHPChanged(GetHealth());

				if (GetHealth() <= 0.f)
				{
					Player->HandleOutOfHealth();
				}
			}
		}
	}

	else if (Attr == GetIncomingTrueDamageAttribute())
	{
		float Damage = GetIncomingTrueDamage();
		SetIncomingTrueDamage(0.f);

		const float OldHealthBeforeDamage = GetHealth();
		if (OldHealthBeforeDamage <= 0.f)
			return;

		if (Damage <= 0.f)
			return;

		const float OldHealth = GetHealth();
		const float NewHealth = FMath::Clamp(GetHealth() - Damage, 0.f, GetMaxHealth());
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

		UpdateHealthNotFullTag(Data);

		AActor* Avatar = Data.Target.AbilityActorInfo.IsValid()
			? Data.Target.AbilityActorInfo->AvatarActor.Get()
			: nullptr;

		if (Avatar)
		{
			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				if (AOZPlayerState* PS = Player->GetPlayerState<AOZPlayerState>())
				{
					PS->NotifyDamaged();
				}

				Player->OnCharacterHPChanged(GetHealth());

				if (GetHealth() <= 0.f)
				{
					Player->HandleOutOfHealth();
				}
			}
		}
	}

	else if (Attr == GetMaxHealthAttribute())
	{
		float NewMaxHealth = GetMaxHealth();
		float CurrentMaxSheild = GetMaxShield();

		AActor* Avatar = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			Avatar = Data.Target.AbilityActorInfo->AvatarActor.Get();
		}

		if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
		{
			//ü�� �̵������� ȿ��
			//Player->OnCharacterMaxHPChanged(NewMaxHealth);

			Player->BroadCastUpdateCharacterSheildHPRatio(CurrentMaxSheild, NewMaxHealth);
		}

		UpdateHealthNotFullTag(Data);
	}

	else if (Attr == GetShieldAttribute())
	{
		float NewShield = GetShield();
		const float Max = FMath::Max(GetMaxShield(), 0.0f);

		NewShield = FMath::Clamp(NewShield, 0.0f, Max);
		SetShield(NewShield);

		AActor* Avatar = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			Avatar = Data.Target.AbilityActorInfo->AvatarActor.Get();

			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				float currSheildRatio = NewShield / GetMaxShield();

				Player->OnCharacterSheildChanged(currSheildRatio);

				// Shield Effect 업데이트 (서버에서 Multicast)
				if (Player->HasAuthority())
				{
					Player->Multicast_UpdateShieldEffect(NewShield > 0.f);
				}
			}
		}

	}

	else if (Attr == GetMaxShieldAttribute())
	{
		float NewMaxShield = GetMaxShield();

		AActor* Avatar = nullptr;
		if (Data.Target.AbilityActorInfo.IsValid())
		{
			Avatar = Data.Target.AbilityActorInfo->AvatarActor.Get();

			if (AOZPlayer* Player = Cast<AOZPlayer>(Avatar))
			{
				Player->BroadCastUpdateCharacterSheildHPRatio(GetMaxShield(), GetMaxHealth());
			}
		}

	}
}

void UOZPlayerAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const bool bIsServer = (ASC && ASC->GetOwnerRole() == ROLE_Authority);

	AOZPlayer* Player = nullptr;
	if (ASC && ASC->AbilityActorInfo.IsValid())
	{
		Player = Cast<AOZPlayer>(ASC->AbilityActorInfo->AvatarActor.Get());
	}

	if (Attribute == GetMaxHealthAttribute())
	{
		const float ClampedNewMax = FMath::Max(NewValue, 0.f);

		if (bIsServer)
		{
			if (ClampedNewMax > OldValue)
			{
				SetHealth(ClampedNewMax);
			}
			else
			{
				SetHealth(FMath::Clamp(GetHealth(), 0.f, ClampedNewMax));
			}
		}

		if (Player)
		{
			Player->BroadCastUpdateCharacterSheildHPRatio(GetMaxShield(), ClampedNewMax);
			Player->OnCharacterHPChanged(GetHealth());
		}

		return;
	}

	if (Attribute == GetMaxStaminaAttribute())
	{
		const float ClampedNewMax = FMath::Max(NewValue, 0.f);

		if (bIsServer)
		{
			if (ClampedNewMax > OldValue)
			{
				SetCurrentStamina(ClampedNewMax);
			}
			else
			{
				SetCurrentStamina(FMath::Clamp(GetCurrentStamina(), 0.f, ClampedNewMax));
			}
		}

		return;
	}

	if (Attribute == GetMaxShieldAttribute())
	{
		const float ClampedNewMax = FMath::Max(NewValue, 0.f);

		if (bIsServer)
		{
			if (ClampedNewMax > OldValue)
			{
				SetShield(ClampedNewMax);
			}
			else
			{
				SetShield(FMath::Clamp(GetShield(), 0.f, ClampedNewMax));
			}
		}

		if (Player)
		{
			Player->BroadCastUpdateCharacterSheildHPRatio(ClampedNewMax, GetMaxHealth());

			const float Ratio = (ClampedNewMax > 0.f) ? (GetShield() / ClampedNewMax) : 0.f;
			Player->OnCharacterSheildChanged(Ratio);
		}

		return;
	}
}

void UOZPlayerAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, MaxStamina, OldValue);
}

void UOZPlayerAttributeSet::OnRep_StaminaRegenDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, StaminaRegenDelay, OldValue);
}

void UOZPlayerAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, StaminaRegenRate, OldValue);
}

void UOZPlayerAttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, WalkSpeed, OldValue);
}

void UOZPlayerAttributeSet::OnRep_SprintSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, SprintSpeed, OldValue);
}

void UOZPlayerAttributeSet::OnRep_SprintStaminaCost(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, SprintStaminaCost, OldValue);
}

void UOZPlayerAttributeSet::OnRep_EvLDistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, EvLDistance, OldValue);
}

void UOZPlayerAttributeSet::OnRep_EvDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, EvDelay, OldValue);
}

void UOZPlayerAttributeSet::OnRep_EvStaminaCost(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, EvStaminaCost, OldValue);
}

void UOZPlayerAttributeSet::OnRep_AimingDistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, AimingDistance, OldValue);
}

void UOZPlayerAttributeSet::OnRep_AimingCameraDelay(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, AimingCameraDelay, OldValue);
}

void UOZPlayerAttributeSet::OnRep_AimingMoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, AimingMoveSpeed, OldValue);
}

void UOZPlayerAttributeSet::OnRep_ShieldRegenTime(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, ShieldRegenTime, OldValue);
}

void UOZPlayerAttributeSet::OnRep_ShieldRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, ShieldRegenRate, OldValue);
}

void UOZPlayerAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, Armor, OldValue);
}

void UOZPlayerAttributeSet::OnRep_HitMoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, HitMoveSpeed, OldValue);
}

void UOZPlayerAttributeSet::OnRep_HitMoveSpeedTime(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, HitMoveSpeedTime, OldValue);
}

void UOZPlayerAttributeSet::OnRep_ViewingAngle(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, ViewingAngle, OldValue);
}

void UOZPlayerAttributeSet::OnRep_CameraDepth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, CameraDepth, OldValue);
}

void UOZPlayerAttributeSet::OnRep_PeripharalVision(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, PeripharalVision, OldValue);
}

void UOZPlayerAttributeSet::OnRep_CurrentStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, CurrentStamina, OldValue);
}

void UOZPlayerAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UOZPlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, Health, OldValue);
}

void UOZPlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, MaxHealth, OldValue);
}

void UOZPlayerAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, Shield, OldValue);

	// 클라이언트에서 Shield Effect 업데이트
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (ASC && ASC->AbilityActorInfo.IsValid())
	{
		if (AOZPlayer* Player = Cast<AOZPlayer>(ASC->AbilityActorInfo->AvatarActor.Get()))
		{
			Player->UpdateShieldEffectLocal(GetShield() > 0.f);
		}
	}
}

void UOZPlayerAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOZPlayerAttributeSet, MaxShield, OldValue);
}

void UOZPlayerAttributeSet::UpdateHealthNotFullTag(
	const FGameplayEffectModCallbackData& Data)
{
	if (!Data.Target.AbilityActorInfo.IsValid())
		return;

	UAbilitySystemComponent* TargetASC =
		Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
	if (!TargetASC)
		return;

	AActor* Avatar = Data.Target.AbilityActorInfo->AvatarActor.Get();
	if (!Avatar || !Avatar->HasAuthority())
		return;

	const float HP = GetHealth();
	const float MaxHP = GetMaxHealth();

	const bool bHealthFull =
		(MaxHP > 0.f) &&
		(HP >= MaxHP - KINDA_SMALL_NUMBER);

	TargetASC->SetLooseGameplayTagCount(
		OZGameplayTags::Player_State_HealthNotFull,
		bHealthFull ? 0 : 1
	);
}

