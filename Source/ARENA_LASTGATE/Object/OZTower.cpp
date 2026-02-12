#include "Object/OZTower.h"
#include "Object/OZTowerVisionArea.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerController.h"
#include "GameState/OZInGameGameState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

AOZTower::AOZTower()
{
	PrimaryActorTick.bCanEverTick = true;

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	SetRootComponent(TowerMesh);

	TowerEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TowerEffectComp"));
	TowerEffectComp->SetupAttachment(RootComponent);
	TowerEffectComp->bAutoActivate = false;

	bReplicates = true;
	SetReplicateMovement(false);
}

void AOZTower::BeginPlay()
{
	Super::BeginPlay();

	UpdateTowerEffect();
}

void AOZTower::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZTower, TowerState);
	DOREPLIFETIME(AOZTower, OwningPlayer);
}

void AOZTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOZTower::OnInteract_Implementation(AActor* InteractActor)
{
	AOZPlayer* Player = Cast<AOZPlayer>(InteractActor);
	if (!Player)
		return;

	if (IsInsideBlackFog())
		return;

	// PlayerController를 통해 Server RPC 호출
	if (AOZPlayerController* PC = Cast<AOZPlayerController>(Player->GetController()))
	{
		PC->Server_ActivateTower(this);
	}
}

void AOZTower::ActivateTower(AOZPlayer* ActivatingPlayer)
{
	if (!HasAuthority())
		return;

	if (!ActivatingPlayer)
		return;

	AOZPlayer* OldPlayer = OwningPlayer;
	OwningPlayer = ActivatingPlayer;

	UpdateTowerVisionToControllerUI(ActivatingPlayer, OldPlayer);

	HandleOwnerChanged(OldPlayer);

	Multicast_StartEffect(ActivatingPlayer->GetBandanaColor());
	PlayActivationSoundToAll();
}

void AOZTower::DeactivateTower()
{
	if (!HasAuthority())
		return;

	AOZPlayer* OldPlayer = OwningPlayer;

	UpdateTowerVisionToControllerUI(nullptr, OldPlayer);

	OwningPlayer = nullptr;

	HandleOwnerChanged(OldPlayer);

	UE_LOG(LogTemp, Log, TEXT("[Tower] Deactivated"));
}

void AOZTower::ResetTower()
{
	if (!HasAuthority())
		return;

	// 모든 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(Timer_ActiveDuration);
	GetWorld()->GetTimerManager().ClearTimer(Timer_Transition);
	GetWorld()->GetTimerManager().ClearTimer(Timer_FogCheck);

	// 이전 소유자 UI 업데이트
	if (OwningPlayer)
	{
		UpdateTowerVisionToControllerUI(nullptr, OwningPlayer);
	}

	AOZPlayer* OldOwner = OwningPlayer;
	OwningPlayer = nullptr;
	TowerState = EOZTowerState::Inactive;
	ActivationStartTime = 0.f;

	DeactivateAllLinkedAreas();

	UpdateTowerEffect();

	UE_LOG(LogTemp, Log, TEXT("[Tower] Reset to initial state"));
}

void AOZTower::OnRep_TowerState()
{
	UpdateTowerEffect();
}

void AOZTower::OnRep_OwningPlayer(AOZPlayer* OldPlayer)
{
	HandleOwnerChanged(OldPlayer);
}

void AOZTower::HandleOwnerChanged(AOZPlayer* OldPlayer)
{
	// 서버에서만 타이머와 시야 로직 처리
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_ActiveDuration);
		GetWorld()->GetTimerManager().ClearTimer(Timer_Transition);
		GetWorld()->GetTimerManager().ClearTimer(Timer_FogCheck);

		if (OwningPlayer)
		{
			StartActivation();
		}
		else
		{
			StartDeactivation();
		}
	}

	// 이펙트는 모든 클라이언트에서 업데이트
	UpdateTowerEffect();
}

void AOZTower::StartActivation()
{
	TowerState = EOZTowerState::Activating;

	if (TransitionTimeSec > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			Timer_Transition,
			this,
			&AOZTower::FinishActivation,
			TransitionTimeSec,
			false
		);
	}
	else
	{
		FinishActivation();
	}
}

void AOZTower::FinishActivation()
{
	TowerState = EOZTowerState::Active;
	ActivationStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	ActivateAllLinkedAreas();

	if (ActiveDurationSec > 0.f)
	{
		// �۵� ����
		GetWorld()->GetTimerManager().SetTimer(
			Timer_ActiveDuration,
			this,
			&AOZTower::DeactivateTower,
			ActiveDurationSec,
			false
		);
	}

	GetWorld()->GetTimerManager().SetTimer(
		Timer_FogCheck,
		this,
		&AOZTower::CheckBlackFog,
		1.f,
		true
	);
}

void AOZTower::StartDeactivation()
{
	GetWorld()->GetTimerManager().ClearTimer(Timer_FogCheck);

	TowerState = EOZTowerState::Deactivating;

	if (TransitionTimeSec > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			Timer_Transition,
			this,
			&AOZTower::FinishDeactivation,
			TransitionTimeSec,
			false
		);
	}
	else
	{
		FinishDeactivation();
	}
}

void AOZTower::FinishDeactivation()
{
	TowerState = EOZTowerState::Inactive;
	ActivationStartTime = 0.f;

	DeactivateAllLinkedAreas();
}

void AOZTower::CheckBlackFog()
{
	if (!HasAuthority())
		return;

	if (TowerState != EOZTowerState::Active)
		return;

	if (IsInsideBlackFog())
	{
		GetWorld()->GetTimerManager().ClearTimer(Timer_ActiveDuration);
		DeactivateTower();
	}
}

bool AOZTower::IsInsideBlackFog() const
{
	if (!GetWorld())
		return false;

	AOZInGameGameState* GS = GetWorld()->GetGameState<AOZInGameGameState>();
	if (!GS)
		return false;

	if (GS->RepFogSafeRadiusCm <= 0.f)
		return false;

	const FVector TowerLocation = GetActorLocation();
	const FVector FogCenter = GS->RepFogCenter;
	const float SafeRadius = GS->RepFogSafeRadiusCm;

	const float Dist2D = FVector::Dist2D(TowerLocation, FogCenter);
	return Dist2D > SafeRadius;
}

float AOZTower::GetRemainingTime() const
{
	if (TowerState != EOZTowerState::Active || ActivationStartTime <= 0.f)
		return 0.f;

	const float Elapsed = GetWorld() ? GetWorld()->GetTimeSeconds() - ActivationStartTime : 0.f;

	return FMath::Max(0.f, ActiveDurationSec - Elapsed);
}

void AOZTower::ActivateAllLinkedAreas()
{
	for (AOZTowerVisionArea* Area : LinkedVisionAreas)
	{
		if (Area && OwningPlayer)
		{
			Area->Activate(OwningPlayer);
		}
	}

}

void AOZTower::DeactivateAllLinkedAreas()
{
	for (AOZTowerVisionArea* Area : LinkedVisionAreas)
	{
		if (Area)
		{
			Area->Deactivate();
		}
	}
}

void AOZTower::UpdateTowerVisionToControllerUI(AOZPlayer* ActivatingPlayer, AOZPlayer* OldPlayer)
{
	if (ActivatingPlayer == OldPlayer)
		return;

	if (ActivatingPlayer != nullptr)
	{
		if (AOZPlayerController* activatiingPlayerPC = Cast<AOZPlayerController>(ActivatingPlayer->GetController()))
		{
			activatiingPlayerPC->OnTowerActivationChanged(true);
		}
	}
	

	if (OldPlayer != nullptr)
	{
		if (AOZPlayerController* deActivatingPlayerPC = Cast<AOZPlayerController>(OldPlayer->GetController()))
		{
			deActivatingPlayerPC->OnTowerActivationChanged(false);
		}
	}
	
}

void AOZTower::UpdateTowerEffect()
{
	if (!TowerEffectComp)
		return;

	FLinearColor EffectColor = GetEffectColorForLocalPlayer();
	TowerEffectComp->SetVariableLinearColor("BodyColor", EffectColor);

	if (TowerState == EOZTowerState::Inactive)
	{
		if (TowerEffectComp->IsActive())
		{
			TowerEffectComp->Deactivate();
		}
	}
	else
	{
		if (!TowerEffectComp->IsActive())
		{
			TowerEffectComp->Activate(true);
		}
	}
}

FLinearColor AOZTower::GetEffectColorForLocalPlayer() const
{
	if (!OwningPlayer)
	{
		return InactiveEffectColor;
	}

	if (OwningPlayer->IsLocallyControlled())
	{
		return SelfActivatedEffectColor;
	}

	return OtherActivatedEffectColor;
}

void AOZTower::Multicast_StartEffect_Implementation(FLinearColor BandanaColor)
{
	StartEffect(BandanaColor);
}

void AOZTower::PlayActivationSoundToAll()
{
	if (!HasAuthority())
		return;

	Multicast_PlayActivationSound();
}

void AOZTower::Multicast_PlayActivationSound_Implementation()
{
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}
}
