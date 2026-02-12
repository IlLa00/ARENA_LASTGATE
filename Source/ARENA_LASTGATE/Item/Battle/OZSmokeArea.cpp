#include "Item/Battle/OZSmokeArea.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/OZPlayer.h"
#include "Data/OZBattleItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"

AOZSmokeArea::AOZSmokeArea()
{
	PrimaryActorTick.bCanEverTick = false;

	SmokeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SmokeSphere"));
	SetRootComponent(SmokeSphere);

	SmokeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SmokeSphere->SetGenerateOverlapEvents(true);

	SmokeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	SmokeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bReplicates = true;
	SetReplicateMovement(false);
}

void AOZSmokeArea::BeginPlay()
{
	Super::BeginPlay();

	SmokeSphere->SetSphereRadius(FMath::Max(0.f, RadiusCm));

	if (SmokeParticle)
	{
		const FVector Scale = FVector(SmokeParticleScale);

		UGameplayStatics::SpawnEmitterAttached(
			SmokeParticle,
			SmokeSphere,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			true,
			EPSCPoolMethod::None
		);
	}

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), RadiusCm, 16, FColor::White, false, DurationSec);
	}

	if (SmokeLoopSound)
	{
		SmokeAudioComp = UGameplayStatics::SpawnSoundAttached(
			SmokeLoopSound,
			SmokeSphere,
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false,
			0.0f,  
			1.0f,
			0.0f
		);

		if (SmokeAudioComp)
		{
			UpdateSoundVolume();

			GetWorldTimerManager().SetTimer(
				VolumeUpdateTimerHandle,
				this,
				&AOZSmokeArea::UpdateSoundVolume,
				0.1f,
				true
			);
		}
	}

	if (HasAuthority())
	{
		if (DurationSec > 0.f)
		{
			SetLifeSpan(DurationSec);
		}
	}
}

void AOZSmokeArea::InitInstigator(APawn* InInstigatorPawn)
{
	InstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);
}

void AOZSmokeArea::InitFromBattleItemData(const FOZBattleItemData Data)
{
	RadiusCm = Data.Form_Extent;
	DurationSec = static_cast<float>(Data.Item_Duration);
}

bool AOZSmokeArea::IsPlayerInside(AOZPlayer* Player) const
{
	if (!Player || !SmokeSphere)
		return false;

	TArray<AActor*> OverlappingActors;
	SmokeSphere->GetOverlappingActors(OverlappingActors);

	return OverlappingActors.Contains(Player);
}

void AOZSmokeArea::UpdateSoundVolume()
{
	if (!SmokeAudioComp)
		return;

	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!LocalPawn)
	{
		SmokeAudioComp->SetVolumeMultiplier(0.f);
		return;
	}

	float Volume = CalculateVolumeByDistance(GetActorLocation(), LocalPawn->GetActorLocation());
	SmokeAudioComp->SetVolumeMultiplier(Volume);
}

float AOZSmokeArea::CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const
{
	const float Distance = FVector::Dist(SoundLocation, ListenerLocation);

	if (Distance <= 300.f)
		return 1.0f;

	if (Distance <= 700.f)
		return 0.8f;

	if (Distance <= 1200.f)
		return 0.5f;

	return 0.f;
}
