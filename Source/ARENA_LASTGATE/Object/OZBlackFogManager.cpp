#include "Object/OZBlackFogManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameState/OZInGameGameState.h"
#include "Subsystem/OZFogSubsystem.h"
#include "Character/OZPlayer.h"

AOZBlackFogManager::AOZBlackFogManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void AOZBlackFogManager::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOZFogSubsystem* FogSubsystem = GI->GetSubsystem<UOZFogSubsystem>())
		{
			FogDataTable = FogSubsystem->GetFogDataTable();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[BlackFogManager] FogSubsystem not found!"));
		}
	}

}

void AOZBlackFogManager::StartFogForRound()
{
	if (!IsServer())
		return;

	if (!FogDataTable)
	{
		return;
	}

	ClearAllTimers();
	CachedOutsidePawns.Reset();

	State = EOZFogState::Idle;
	PhaseIndex = 1;

	PrevCenter = MapCenter;
	NextCenter = MapCenter;

	PrevRadiusCm = InitialMapRadiusCm;
	NextRadiusCm = InitialMapRadiusCm;

	StateStartTime = 0.f;
	StateEndTime = 0.f;

	DamageTickSeconds = 0.5f;
	DamagePerTick = 0;

	if (bDrawDebug)
	{
		GetWorldTimerManager().SetTimer(
			Timer_DebugDraw, this, &AOZBlackFogManager::DebugDraw, DebugDrawInterval, true);
	}

	EnterPhase(1);
	StartRepUpdate();
	PushFogSnapshotToGameState();
}

void AOZBlackFogManager::StopFogForRound()
{
	if (!IsServer())
		return;

	for (const TWeakObjectPtr<APawn>& WeakPawn : CachedOutsidePawns)
	{
		if (APawn* Pawn = WeakPawn.Get())
		{
			if (AOZPlayer* TargetPlayer = Cast<AOZPlayer>(Pawn))
			{
				TargetPlayer->Client_StopEffect();
			}
		}
	}

	ClearAllTimers();
	CachedOutsidePawns.Reset();

	State = EOZFogState::Idle;
	PhaseIndex = 1;

	PrevCenter = MapCenter;
	NextCenter = MapCenter;
	PrevRadiusCm = 0.f;
	NextRadiusCm = 0.f;

	StateStartTime = 0.f;
	StateEndTime = 0.f;

	DamageTickSeconds = 0.f;
	DamagePerTick = 0;

	StopRepUpdate();
	PushFogSnapshotToGameState();
}

void AOZBlackFogManager::EnterPhase(int32 NewPhaseIndex)
{
	PhaseIndex = FMath::Clamp(NewPhaseIndex, 1, MaxPhase);

	const FOZFogData* PhaseData = FindPhaseRow(PhaseIndex);
	if (!PhaseData)
	{
		State = EOZFogState::Idle;
		return;
	}

	if (PhaseIndex == 1)
	{
		PrevCenter = MapCenter;
		PrevRadiusCm = InitialMapRadiusCm;
	}

	NextRadiusCm = PhaseData->Fog_Target_Size;

	const float MaxShift = FMath::Max(PrevRadiusCm - NextRadiusCm, 0.f);

	const FVector Offset = PickRandomOffset(MaxShift);

	NextCenter = PrevCenter + Offset;
	EnterWaiting(*PhaseData);
}

void AOZBlackFogManager::EnterWaiting(const FOZFogData& PhaseData)
{
	State = EOZFogState::Waiting;

	const float Now = GetNow();
	StateStartTime = Now;
	StateEndTime = Now + PhaseData.Fog_Reduction;

	OnBlackFogStartCountingForShrink.Broadcast(StateEndTime);

	RefreshDamageTimer(PhaseData);

	UE_LOG(LogTemp, Log, TEXT("[Fog] Phase %d WAIT (%.1fs). PrevCenter=(%.0f,%.0f) PrevR=%.1f NextR=%.1f"),
		PhaseIndex, PhaseData.Fog_Reduction,
		PrevCenter.X, PrevCenter.Y,
		PrevRadiusCm, NextRadiusCm);

	GetWorldTimerManager().SetTimer(
		Timer_StateTransition, this, &AOZBlackFogManager::OnWaitingFinished,
		PhaseData.Fog_Reduction, false);
}

void AOZBlackFogManager::OnWaitingFinished()
{
	if (!IsServer() || State != EOZFogState::Waiting)
		return;

	const FOZFogData* PhaseData = FindPhaseRow(PhaseIndex);
	if (!PhaseData)
		return;

	EnterShrinking(*PhaseData);
}

void AOZBlackFogManager::EnterShrinking(const FOZFogData& PhaseData)
{
	OnBlackFogStartShrink.Broadcast(PhaseIndex, PhaseData.Fog_Move_Time);

	State = EOZFogState::Shrinking;

	const float Now = GetNow();
	StateStartTime = Now;
	StateEndTime = Now + PhaseData.Fog_Move_Time;

	RefreshDamageTimer(PhaseData);

	UE_LOG(LogTemp, Log, TEXT("[Fog] Phase %d SHRINK (%.1fs). Damage=%d / %.2fs"),
		PhaseIndex, PhaseData.Fog_Move_Time, DamagePerTick, DamageTickSeconds);

	GetWorldTimerManager().SetTimer(
		Timer_StateTransition, this, &AOZBlackFogManager::OnShrinkingFinished,
		PhaseData.Fog_Move_Time, false);
}

void AOZBlackFogManager::OnShrinkingFinished()
{
	if (!IsServer() || State != EOZFogState::Shrinking)
		return;

	PrevCenter = NextCenter;
	PrevRadiusCm = NextRadiusCm;

	CachedOutsidePawns.Reset();

	if (PhaseIndex >= MaxPhase && PrevRadiusCm <= KINDA_SMALL_NUMBER)
	{
		const FOZFogData* PhaseData = FindPhaseRow(PhaseIndex);
		if (PhaseData)
		{
			EnterFinalSuddenDeath(*PhaseData);
		}
		return;
	}

	const int32 NextPhase = FMath::Min(PhaseIndex + 1, MaxPhase);
	EnterPhase(NextPhase);
}

void AOZBlackFogManager::EnterFinalSuddenDeath(const FOZFogData& PhaseData)
{
	State = EOZFogState::Final;

	PrevRadiusCm = 0.f;
	NextRadiusCm = 0.f;

	RefreshDamageTimer(PhaseData);

	UE_LOG(LogTemp, Log, TEXT("[Fog] FINAL SuddenDeath. Damage=%d / %.2fs (all outside)"),
		DamagePerTick, DamageTickSeconds);
}

void AOZBlackFogManager::TickFogDamage()
{
	if (!IsServer() || State == EOZFogState::Idle)
		return;

	const float Now = GetNow();
	const float CurrentRadius = GetCurrentRadiusCm(Now);
	const FVector CurrentCenter = GetCurrentCenter(Now);

	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		bool bOutside = true;

		if (!IsFinalSuddenDeath())
		{
			const FVector P = Pawn->GetActorLocation();
			const float Dist2D = FVector::Dist2D(P, FVector(CurrentCenter.X, CurrentCenter.Y, P.Z));
			bOutside = (Dist2D > CurrentRadius);
		}

		bool bWasOutside = false;
		for (const TWeakObjectPtr<APawn>& WeakPawn : CachedOutsidePawns)
		{
			if (WeakPawn.Get() == Pawn)
			{
				bWasOutside = true;
				break;
			}
		}

		if (bOutside)
		{
			if (!bWasOutside)
			{
				if (AOZPlayer* TargetPlayer = Cast<AOZPlayer>(Pawn))
				{
					TargetPlayer->Client_StartEffect(EPlayerEffectType::Fog);
				}
				CachedOutsidePawns.Add(Pawn);
			}
			ApplyFogDamageToPawn(Pawn, DamagePerTick);
		}
		else
		{
			if (bWasOutside)
			{
				if (AOZPlayer* TargetPlayer = Cast<AOZPlayer>(Pawn))
				{
					TargetPlayer->Client_StopEffect();
				}
				for (auto SetIt = CachedOutsidePawns.CreateIterator(); SetIt; ++SetIt)
				{
					if (SetIt->Get() == Pawn)
					{
						SetIt.RemoveCurrent();
						break;
					}
				}
			}
		}
	}
}

void AOZBlackFogManager::RefreshDamageTimer(const FOZFogData& PhaseData)
{
	DamageTickSeconds = PhaseData.Fog_Damage_Delay;
	DamagePerTick = PhaseData.Fog_Damage;

	GetWorldTimerManager().ClearTimer(Timer_DamageTick);

	if (DamageTickSeconds > 0.f && DamagePerTick > 0)
	{
		GetWorldTimerManager().SetTimer(
			Timer_DamageTick,
			this,
			&AOZBlackFogManager::TickFogDamage,
			DamageTickSeconds,
			true
		);
	}
}

void AOZBlackFogManager::ApplyFogDamageToPawn(APawn* Pawn, int32 DamageAmount)
{
	if (!IsServer() || !Pawn || DamageAmount <= 0 || !DamageEffectClass)
		return;

	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Pawn);
	if (!TargetASI)
		return;

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
		return;

	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle =
		TargetASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);

	if (!SpecHandle.IsValid())
		return;

	static const FGameplayTag DamageTag =
		FGameplayTag::RequestGameplayTag(FName("Player.Data.TrueDamage"));

	SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, static_cast<float>(DamageAmount));

	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (AOZPlayer* TargetPlayer = Cast<AOZPlayer>(Pawn))
	{
		TargetPlayer->Client_PlayFogDamageSound();
	}
}

float AOZBlackFogManager::GetNow() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

float AOZBlackFogManager::GetCurrentRadiusCm(float Now) const
{
	if (State != EOZFogState::Shrinking)
		return PrevRadiusCm;

	const float Duration = FMath::Max(StateEndTime - StateStartTime, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp((Now - StateStartTime) / Duration, 0.f, 1.f);
	return FMath::Lerp(PrevRadiusCm, NextRadiusCm, Alpha);
}

FVector AOZBlackFogManager::GetCurrentCenter(float Now) const
{
	if (State != EOZFogState::Shrinking)
		return PrevCenter;

	const float Duration = FMath::Max(StateEndTime - StateStartTime, KINDA_SMALL_NUMBER);
	const float Alpha = FMath::Clamp((Now - StateStartTime) / Duration, 0.f, 1.f);

	return FMath::Lerp(PrevCenter, NextCenter, Alpha);
}

bool AOZBlackFogManager::IsFinalSuddenDeath() const
{
	return State == EOZFogState::Final;
}

FVector AOZBlackFogManager::PickRandomOffset(float MaxShiftCm)
{
	const float Radius = FMath::Max(MaxShiftCm, 0.f);

	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float U = FMath::FRand();
	const float R = FMath::Sqrt(U) * Radius;

	return FVector(R * FMath::Cos(Angle), R * FMath::Sin(Angle), 0.f);
}

void AOZBlackFogManager::DebugDraw()
{
	if (!bDrawDebug)
		return;

	if (State == EOZFogState::Idle)
		return;

	const float Now = GetNow();
	const FVector CurrCenter = GetCurrentCenter(Now);
	const float   CurrRadius = GetCurrentRadiusCm(Now);

	if (PrevRadiusCm > 1.f)
	{
		DrawDebugCircle(GetWorld(), FVector(PrevCenter.X, PrevCenter.Y, 50.f),
			PrevRadiusCm, 64, FColor::Red, false, DebugDrawInterval, 0, 5.f,
			FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	if (NextRadiusCm > 1.f)
	{
		DrawDebugCircle(GetWorld(), FVector(NextCenter.X, NextCenter.Y, 50.f),
			NextRadiusCm, 64, FColor::Green, false, DebugDrawInterval, 0, 5.f,
			FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	if (CurrRadius > 1.f && (State == EOZFogState::Waiting || State == EOZFogState::Shrinking))
	{
		DrawDebugCircle(GetWorld(), FVector(CurrCenter.X, CurrCenter.Y, 50.f),
			CurrRadius, 64, FColor::Yellow, false, DebugDrawInterval, 0, 8.f,
			FVector(1, 0, 0), FVector(0, 1, 0), false);
	}

	UE_LOG(LogTemp, VeryVerbose, TEXT("[Fog] State=%d Phase=%d CurrR=%.1f PrevR=%.1f NextR=%.1f"),
		(int32)State, PhaseIndex, CurrRadius, PrevRadiusCm, NextRadiusCm);
}

FName AOZBlackFogManager::MakeRowName(int32 Phase) const
{
	return FName(*FString::Printf(TEXT("%s%d"), *RowPrefix.ToString(), Phase));
}

const FOZFogData* AOZBlackFogManager::FindPhaseRow(int32 Phase) const
{
	if (!FogDataTable)
		return nullptr;

	const FName RowName = MakeRowName(Phase);
	return FogDataTable->FindRow<FOZFogData>(RowName, TEXT("Fog"));
}

void AOZBlackFogManager::ClearAllTimers()
{
	if (!GetWorld())
		return;

	GetWorldTimerManager().ClearTimer(Timer_StateTransition);
	GetWorldTimerManager().ClearTimer(Timer_DamageTick);
	GetWorldTimerManager().ClearTimer(Timer_DebugDraw);
}

bool AOZBlackFogManager::IsServer() const
{
	return HasAuthority();
}

void AOZBlackFogManager::StartRepUpdate()
{
	if (!IsServer())
		return;

	GetWorldTimerManager().ClearTimer(Timer_RepUpdate);

	GetWorldTimerManager().SetTimer(
		Timer_RepUpdate,
		this,
		&AOZBlackFogManager::TickRepUpdate,
		RepUpdateInterval,
		true
	);
}

void AOZBlackFogManager::StopRepUpdate()
{
	if (!GetWorld())
		return;

	GetWorldTimerManager().ClearTimer(Timer_RepUpdate);
}

void AOZBlackFogManager::TickRepUpdate()
{
	if (!IsServer())
		return;

	if (State == EOZFogState::Idle)
	{
		PushFogSnapshotToGameState();
		return;
	}

	PushFogSnapshotToGameState();
}

void AOZBlackFogManager::PushFogSnapshotToGameState()
{
	if (!IsServer() || !GetWorld())
		return;

	AOZInGameGameState* GS = GetWorld()->GetGameState<AOZInGameGameState>();
	if (!GS)
		return;

	const float Now = GetNow();

	const FVector CurrCenter = (State == EOZFogState::Idle) ? MapCenter : GetCurrentCenter(Now);
	const float   CurrRadius = (State == EOZFogState::Idle) ? 0.f : GetCurrentRadiusCm(Now);

	GS->RepFogState = State;
	GS->RepFogPhaseIndex = PhaseIndex;
	GS->RepFogCenter = CurrCenter;
	GS->RepFogSafeRadiusCm = CurrRadius;
	GS->RepFogInitialMapRadiusCm = InitialMapRadiusCm;

	if (State == EOZFogState::Idle)
	{
		GS->RepFogPrevCenter = MapCenter;
		GS->RepFogPrevRadiusCm = 0.f;
		GS->RepFogNextCenter = MapCenter;
		GS->RepFogNextRadiusCm = 0.f;
	}
	else
	{
		GS->RepFogPrevCenter = PrevCenter;
		GS->RepFogPrevRadiusCm = PrevRadiusCm;
		GS->RepFogNextCenter = NextCenter;
		GS->RepFogNextRadiusCm = NextRadiusCm;
	}

	GS->ForceNetUpdate();

	GS->UpdateFogVisual_Local();
}
