#include "OZPlayerState.h"
#include "OZPlayerController.h"
#include "HUD/OZLobbyHUD.h"
#include "GameState/OZLobbyGameState.h"
#include "Data/OZAbilityInputID.h"
#include "HUD/OZHud.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "Character/Components/OZInventoryComponent.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Character/Weapon/OZWeaponAttributeSet.h"
#include "Character/OZPlayer.h"
#include "HUD/OZInGameHUD.h"
#include "Tags/OZGameplayTags.h"
#include "Subsystem/OZCharacterSubsystem.h"
#include "Subsystem/OZWeaponSubsystem.h"
#include "Subsystem/OZConvertSubsystem.h"


AOZPlayerState::AOZPlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));

	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	InventoryComp = CreateDefaultSubobject<UOZInventoryComponent>(TEXT("InventoryComp"));
	InventoryComp->SetIsReplicated(true);

	PlayerAttributes = CreateDefaultSubobject<UOZPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UOZWeaponAttributeSet>(TEXT("WeaponAttributeSet"));

	CurrentBushID = INDEX_NONE;
}

void AOZPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		SetAttributes();
		UE_LOG(LogTemp, Warning, TEXT(
			"[OZPlayerState::PostInitializeComponents] SetAttributes called"));
	}
}

void AOZPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !ASC)
		return;

	if (StaminaGE)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(StaminaGE, 1.f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	if (ShieldRegenGE)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle =
			ASC->MakeOutgoingSpec(ShieldRegenGE, 1.f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	GiveDefaultAbilities();

	if (CoolingAbilityClass)
	{
		ASC->TryActivateAbilityByClass(CoolingAbilityClass);
	}
}

void AOZPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZPlayerState, UserName);
	DOREPLIFETIME(AOZPlayerState, UserIdentificationCode);
	DOREPLIFETIME(AOZPlayerState, Round_SurvivalRanking);
	DOREPLIFETIME(AOZPlayerState, Round_KillCount);
	DOREPLIFETIME(AOZPlayerState, RoundCombatBonus);
	DOREPLIFETIME(AOZPlayerState, OwningCores);
	DOREPLIFETIME(AOZPlayerState, OwningScraps);
	DOREPLIFETIME(AOZPlayerState, userLobbyReadyState);
	DOREPLIFETIME(AOZPlayerState, PendingSlotReward);

	DOREPLIFETIME(AOZPlayerState, Total_KillCount);
	DOREPLIFETIME(AOZPlayerState, Round_DamageAmount);
	DOREPLIFETIME(AOZPlayerState, Round_DamagedAmount);
	DOREPLIFETIME(AOZPlayerState, Total_DamageAmount);
	DOREPLIFETIME(AOZPlayerState, Total_DamagedAmount);
	DOREPLIFETIME(AOZPlayerState, bInBush);
	DOREPLIFETIME(AOZPlayerState, CurrentBushID);
	DOREPLIFETIME(AOZPlayerState, AcquiredConvertIDs);
	DOREPLIFETIME(AOZPlayerState, AcquiredUniqueConvertHistory);
}

UAbilitySystemComponent* AOZPlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}

void AOZPlayerState::GiveDefaultAbilities()
{

	if (!ASC || !HasAuthority())
		return;


	if (SprintAbilityClass)
	{
		FGameplayAbilitySpec SprintSpec(SprintAbilityClass, 1, (int32)EOZAbilityInputID::Sprint, this);
		ASC->GiveAbility(SprintSpec);
	}
	if (AimingtAbilityClass)
	{
		FGameplayAbilitySpec AimingSpec(AimingtAbilityClass, 1, (int32)EOZAbilityInputID::Aiming, this);
		ASC->GiveAbility(AimingSpec);
	}
	if (FireAbilityClass)
	{
		FGameplayAbilitySpec FireSpec(FireAbilityClass, 1, (int32)EOZAbilityInputID::Fire, this);
		ASC->GiveAbility(FireSpec);
	}
	if (StaminaAbilityClass)
	{
		FGameplayAbilitySpec StaminaSpec(StaminaAbilityClass, 1, INDEX_NONE, this);
		ASC->GiveAbility(StaminaSpec);
	}
	if (OverheatHandleAbilityClass)
	{
		FGameplayAbilitySpec OverheatSpec(OverheatHandleAbilityClass, 1, INDEX_NONE, this);
		ASC->GiveAbility(OverheatSpec);
	}
	if (CoolingAbilityClass)
	{
		FGameplayAbilitySpec CoolingSpec(CoolingAbilityClass, 1, INDEX_NONE, this);
		ASC->GiveAbility(CoolingSpec);
	}
	if (DashAbilityClass)
	{
		FGameplayAbilitySpec DashSpec(DashAbilityClass, 1, (int32)EOZAbilityInputID::Dash, this);
		ASC->GiveAbility(DashSpec);
	}
}

void AOZPlayerState::OnRep_UserName()
{
	Server_ReportStateUpdate();
}

void AOZPlayerState::OnRep_UserIdentificationCode()
{

}

float AOZPlayerState::GetHeatRatio() const
{
	if (!WeaponAttributeSet)
	{
		return 0.f;
	}

	const float Current = WeaponAttributeSet->GetCurrentHeat();
	const float Max = WeaponAttributeSet->GetMaxHeat();

	if (Max <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}

	return FMath::Clamp(Current / Max, 0.f, 1.f);
}

FStatusRatioData AOZPlayerState::GetHeatlInfo() const
{
	if (!IsValid(PlayerAttributes))
		return FStatusRatioData();

	const float CurrentHP = PlayerAttributes->GetHealth();
	const float MaxHP = PlayerAttributes->GetMaxHealth();

	return FStatusRatioData(CurrentHP, MaxHP);
}

FStatusRatioData AOZPlayerState::GetSteminaInfo() const
{
	if (!IsValid(PlayerAttributes))
		return FStatusRatioData();

	const float CurrentStamina = PlayerAttributes->GetCurrentStamina();
	const float MaxStamina = PlayerAttributes->GetMaxStamina();

	return FStatusRatioData(CurrentStamina, MaxStamina);
}

FStatusRatioData AOZPlayerState::GetSheildInfo() const
{
	if (!IsValid(PlayerAttributes))
		return FStatusRatioData();

	const float CurrentSheild = PlayerAttributes->GetShield();
	const float MaxSheild = PlayerAttributes->GetMaxShield();

	return FStatusRatioData(CurrentSheild, MaxSheild);
}

void AOZPlayerState::HandleRecentlyDamagedExpired()
{
	if (!HasAuthority() || !ASC) return;

	ASC->SetLooseGameplayTagCount(OZGameplayTags::Player_State_RecentlyDamaged, 0);

	UE_LOG(LogTemp, Log, TEXT("[RecentlyDamagedExpired] RecentlyDamaged=%d"),
		ASC->HasMatchingGameplayTag(OZGameplayTags::Player_State_RecentlyDamaged) ? 1 : 0);
}

void AOZPlayerState::HandleHitMoveSpeedExpired()
{
	if (!HasAuthority() || !ASC) return;

	if (ActiveHitMoveSpeedHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ActiveHitMoveSpeedHandle);
		ActiveHitMoveSpeedHandle.Invalidate();
	}
}

void AOZPlayerState::NotifyDamaged()
{
	if (!HasAuthority() || !ASC) return;

	ASC->SetLooseGameplayTagCount(OZGameplayTags::Player_State_RecentlyDamaged, 1);

	UWorld* World = GetWorld();
	if (!World) return;

	const float DelaySeconds = GetShieldRegenDelaySeconds();

	World->GetTimerManager().ClearTimer(RecentlyDamagedTimerHandle);
	World->GetTimerManager().SetTimer(
		RecentlyDamagedTimerHandle,
		this,
		&AOZPlayerState::HandleRecentlyDamagedExpired,
		DelaySeconds,
		false
	);

	if (HitMoveSpeedGE && PlayerAttributes)
	{
		UAbilitySystemComponent* TargetASC = GetAbilitySystemComponent();
		if (TargetASC)
		{
			if (ActiveHitMoveSpeedHandle.IsValid())
			{
				TargetASC->RemoveActiveGameplayEffect(ActiveHitMoveSpeedHandle);
			}

			World->GetTimerManager().ClearTimer(HitMoveSpeedTimerHandle);

			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
				HitMoveSpeedGE, 1.f, ContextHandle);

			if (SpecHandle.IsValid())
			{
				ActiveHitMoveSpeedHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				const float HitMoveSpeedDuration = PlayerAttributes->GetHitMoveSpeedTime();
				World->GetTimerManager().SetTimer(
					HitMoveSpeedTimerHandle,
					this,
					&AOZPlayerState::HandleHitMoveSpeedExpired,
					HitMoveSpeedDuration,
					false
				);
			}
		}
	}
}

void AOZPlayerState::Server_ReportStateUpdate_Implementation()
{
	if (AOZLobbyGameState* gameState = Cast<AOZLobbyGameState>(GetWorld()->GetGameState()))
	{
		TArray<FString> UserNames;

		TArray<APlayerState*> PlayerStates = gameState->PlayerArray;

		for (APlayerState* PS : PlayerStates)
		{
			if (AOZPlayerState* OzplayerState = Cast<AOZPlayerState>(PS))
			{
				UserNames.Add(OzplayerState->UserName);
			}
		}

		Multicast_BroadcastUIUpdate(UserNames);

	}
}

void AOZPlayerState::Multicast_BroadcastUIUpdate_Implementation(const TArray<FString>& UserNames)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC == nullptr)
		return;

	TWeakObjectPtr<AOZPlayerState> WeakThis = this;

	AOZPlayerController* OzPC = Cast<AOZPlayerController>(PC);
	if (OzPC->bUIReady == false)
	{
		FTimerHandle RetryForUIConstructHandle;
		GetWorld()->GetTimerManager().SetTimer(
			RetryForUIConstructHandle,
			FTimerDelegate::CreateLambda([WeakThis, UserNames]()
				{
					if (!WeakThis.IsValid())
						return;

					WeakThis->Multicast_BroadcastUIUpdate(UserNames);

				}),
			0.1f,
			false
		);

		return;

	}

	OzPC->GetHUD<AOZHud>()->SetUserNames(UserNames);
	OzPC->GetHUD<AOZHud>()->UpdateUI();

}

void AOZPlayerState::UpdateClientMiniMapUI(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor)
{
	if (AOZPlayerController* OZController = Cast<AOZPlayerController>(GetOwner()))
	{
		AOZInGameHUD* OzInGameHUD = OZController->GetHUD<AOZInGameHUD>();

		if (OzInGameHUD == nullptr)
			return;

		OzInGameHUD->UpdateMinimap(AddedActor, RemovedActor);
	}

}

void AOZPlayerState::Server_CallUserLobbyReadyState_Implementation(ELobbyUserState userState)
{
	userLobbyReadyState = userState;
}

void AOZPlayerState::OnRep_LoobyReadyState()
{

}

void AOZPlayerState::Server_AddScraps_Implementation(int32 Amount)
{
	if (Amount == 0)return;

	OwningScraps = FMath::Max(0, OwningScraps + Amount);
}
void AOZPlayerState::Server_AddCores_Implementation(uint8 Amount)
{
	if (Amount == 0) return;

	const int32 NewValue = (int32)OwningCores + (int32)Amount;
	OwningCores = (uint8)FMath::Clamp(NewValue, 0, 255);
}
bool AOZPlayerState::HasPendingSlotReward() const
{
	return PendingSlotReward.bWin &&
		(PendingSlotReward.RewardItemIDs.Num() > 0);
}
void AOZPlayerState::ClearPendingSlotReward()
{
	PendingSlotReward = FOZSlotRewardResult();
}
void AOZPlayerState::OnRep_InBush()
{
	APawn* Pawn = GetPawn();
	AOZPlayer* Player = Cast<AOZPlayer>(Pawn);
	if (!Player)
		return;

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC || !PC->IsLocalController())
		return;

	Player->SetHideUI(bInBush);
}

void AOZPlayerState::SetInBush_Server(bool bNewInBush)
{
	if (!HasAuthority()) return;

	if (bInBush == bNewInBush) return;
	bInBush = bNewInBush;

	ForceNetUpdate();

	if (APawn* Pawn = GetPawn())
	{
		if (AOZPlayer* Player = Cast<AOZPlayer>(Pawn))
		{
			if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
			{
				if (PC->IsLocalController())
				{
					Player->SetHideUI(bInBush);
				}
			}
		}
	}
}

float AOZPlayerState::GetShieldRegenDelaySeconds() const
{
	if (!ASC) return 5.f;

	const float Delay = ASC->GetNumericAttribute(UOZPlayerAttributeSet::GetShieldRegenTimeAttribute());

	return (Delay > 0.f) ? Delay : 5.f;
}

void AOZPlayerState::SetAttributes()
{
	// 이미 초기화되었으면 리턴 (리스폰 시 중복 호출 방지)
	if (bAttributesInitialized)
		return;

	if (!PlayerAttributes || !WeaponAttributeSet)
		return;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return;

	if (UOZCharacterSubsystem* CharacterSys = GameInstance->GetSubsystem<UOZCharacterSubsystem>())
	{
		FOZCharacterData* CharData = CharacterSys->GetCharacterData();
		if (CharData)
		{
			PlayerAttributes->InitMaxHealth(CharData->HP);
			PlayerAttributes->InitHealth(PlayerAttributes->GetMaxHealth());
			PlayerAttributes->InitMaxStamina(CharData->Stamina);
			PlayerAttributes->InitCurrentStamina(PlayerAttributes->GetMaxStamina());
			PlayerAttributes->InitStaminaRegenDelay(CharData->Stamina_Regen_Delay);
			PlayerAttributes->InitStaminaRegenRate(CharData->Stamina_Regen_Rate);
			PlayerAttributes->InitWalkSpeed(CharData->Walk_Speed);
			PlayerAttributes->InitSprintSpeed(CharData->Sprint_Speed);
			PlayerAttributes->InitSprintStaminaCost(CharData->Sprint_Stamina_Cost);
			PlayerAttributes->InitMoveSpeed(CharData->Walk_Speed);
			PlayerAttributes->InitEvLDistance(CharData->EvLDistance);
			PlayerAttributes->InitEvDelay(CharData->Ev_Delay);
			PlayerAttributes->InitEvStaminaCost(CharData->Ev_Stamina_Cost);
			PlayerAttributes->InitAimingDistance(CharData->Aiming_Distance);
			PlayerAttributes->InitAimingCameraDelay(CharData->Aiming_Camera_Delay);
			PlayerAttributes->InitAimingMoveSpeed(CharData->Aiming_Move_Speed);
			PlayerAttributes->InitMaxShield(CharData->Shield);
			PlayerAttributes->InitShield(PlayerAttributes->GetMaxShield());
			PlayerAttributes->InitShieldRegenTime(CharData->Shield_Regen_Time);
			PlayerAttributes->InitShieldRegenRate(CharData->Shield_Regen_Rate);
			PlayerAttributes->InitArmor(CharData->Armor);
			PlayerAttributes->InitHitMoveSpeed(CharData->Hit_Move_Speed);
			PlayerAttributes->InitHitMoveSpeedTime(CharData->Hit_Move_Speed_Time);
			PlayerAttributes->InitViewingAngle(CharData->Viewing_Angle);
			PlayerAttributes->InitCameraDepth(CharData->Camera_depth);
			PlayerAttributes->InitPeripharalVision(CharData->Peripharal_vision);
		}
	}

	if (UOZWeaponSubsystem* WeaponSys = GameInstance->GetSubsystem<UOZWeaponSubsystem>())
	{
		FOZWeaponData* WeaponData = WeaponSys->GetWeaponData();
		if (WeaponData)

		{
			WeaponAttributeSet->InitBaseDamage(WeaponData->BaseDamage);
			WeaponAttributeSet->InitBaseAtkDelay(WeaponData->BaseAtkDelay);
			WeaponAttributeSet->InitMaxHeat(WeaponData->BaseMaxHeat);
			WeaponAttributeSet->InitCooling(WeaponData->Cooling);
			WeaponAttributeSet->InitCoolingDelay(WeaponData->CoolingDelay);
			WeaponAttributeSet->InitCoolingBuffer(WeaponData->CoolingBuffer);
			WeaponAttributeSet->InitHeatCoefficient(WeaponData->HeatCoefficient);
			WeaponAttributeSet->InitOverheatPenalty(WeaponData->OverheatPenalty);
			WeaponAttributeSet->InitBaseSpreadAngle(WeaponData->BaseSpread_Angle);
			WeaponAttributeSet->InitBaseNoise(WeaponData->BaseNoise);
			WeaponAttributeSet->InitNoiseMaxRange(WeaponData->NoiseMaxRange);
			WeaponAttributeSet->InitNoiseFalloffRate(WeaponData->NoiseFalloff_Rate);
			WeaponAttributeSet->InitBaseRange(WeaponData->BaseRange);
			WeaponAttributeSet->InitBaseProjectileSpeed(WeaponData->BaseProjectileSpeed);
			WeaponAttributeSet->InitBaseProjectileSize(WeaponData->BaseProjectileSize);
			WeaponAttributeSet->InitBaseProjectilesPerShot(WeaponData->BaseProjectilesPerShot);
			WeaponAttributeSet->InitBaseKnockbackPower(WeaponData->BaseKnockbackPower);
			WeaponAttributeSet->InitMaxRicochetCount(WeaponData->RicochetConut);
			WeaponAttributeSet->InitExplosionRadius(WeaponData->ExplosionRadius);
			WeaponAttributeSet->InitExplosionDamageMult(WeaponData->ExplosionDamage_mult);
			WeaponAttributeSet->InitExplosionFalloffRate(WeaponData->ExplosionFalloff_Rate);
		}
	}

	bAttributesInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("[OZPlayerState::SetAttributes] Attributes initialized"));
}

void AOZPlayerState::SetRespawnAttributes()
{
	if (!PlayerAttributes || !WeaponAttributeSet)
		return;

	PlayerAttributes->SetHealth(PlayerAttributes->GetMaxHealth());
	PlayerAttributes->SetCurrentStamina(PlayerAttributes->GetMaxStamina());

	PlayerAttributes->SetShield(PlayerAttributes->GetMaxShield());

	PlayerAttributes->SetMoveSpeed(PlayerAttributes->GetWalkSpeed());

	WeaponAttributeSet->SetMaxHeat(WeaponAttributeSet->GetMaxHeat());
	WeaponAttributeSet->SetCooling(WeaponAttributeSet->GetCooling());
	WeaponAttributeSet->SetHeatCoefficient(WeaponAttributeSet->GetHeatCoefficient());

	GetWorld()->GetTimerManager().ClearTimer(RecentlyDamagedTimerHandle);
	ASC->SetLooseGameplayTagCount(OZGameplayTags::Player_State_RecentlyDamaged, 0);

	const bool bFull =
		PlayerAttributes->GetHealth() >= PlayerAttributes->GetMaxHealth() - KINDA_SMALL_NUMBER;

	ASC->SetLooseGameplayTagCount(
		OZGameplayTags::Player_State_HealthNotFull,
		bFull ? 0 : 1
	);
}

#include "Kismet/GameplayStatics.h"
void AOZPlayerState::OnRep_CurrentBushID()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (APawn* Pawn = GetPawn())
	{
		if (AOZPlayer* Player = Cast<AOZPlayer>(Pawn))
		{
			if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
			{
				if (PC->IsLocalController())
				{
					Player->UpdateBushVision();
				}
			}
		}
	}
}

void AOZPlayerState::SetCurrentBushID(int32 NewBushID)
{
	if (!HasAuthority()) return;

	CurrentBushID = NewBushID;
	OnRep_CurrentBushID();
}

void AOZPlayerState::AddToConvertHistory(int32 ConvertID, EConvertGrade Grade)
{
	if (!HasAuthority())
		return;

	if (Grade == EConvertGrade::Unique || Grade == EConvertGrade::Legendary)
	{
		AcquiredUniqueConvertHistory.AddUnique(ConvertID);
	}
}

void AOZPlayerState::NotifyConvertAcquired(int32 ConvertID, EConvertGrade Grade)
{
	if (!HasAuthority())
		return;

	// 획득한 Convert ID 추가
	AcquiredConvertIDs.Add(ConvertID);

	// 델리게이트 브로드캐스트 (서버에서)
	OnConvertAcquired.Broadcast(ConvertID, Grade);

	UE_LOG(LogTemp, Log, TEXT("[OZPlayerState] Convert acquired: ID=%d, Total=%d"), ConvertID, AcquiredConvertIDs.Num());
}

void AOZPlayerState::OnRep_AcquiredConvertIDs()
{
	// 클라이언트에서 Convert 목록이 변경되었을 때 델리게이트 브로드캐스트
	// 마지막으로 추가된 Convert의 Grade를 가져오기 위해 ConvertSubsystem 사용
	if (AcquiredConvertIDs.Num() > 0)
	{
		int32 LastConvertID = AcquiredConvertIDs.Last();

		if (UGameInstance* GI = GetGameInstance())
		{
			if (UOZConvertSubsystem* ConvertSubsystem = GI->GetSubsystem<UOZConvertSubsystem>())
			{
				if (FOZConvertData* ConvertData = ConvertSubsystem->FindConvert(LastConvertID))
				{
					OnConvertAcquired.Broadcast(LastConvertID, ConvertData->Grade);
				}
			}
		}
	}
}

