// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Battle/OZGrenadeProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Item/Battle/OZFireArea.h"
#include "Item/Battle/OZSmokeArea.h"
#include "Character/Components/OZVisionComponent.h"
#include "TimerManager.h"
#include "Data/OZBattleItemData.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraComponent.h" 
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AOZGrenadeProjectile::AOZGrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);

	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetGenerateOverlapEvents(false);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	ProjectileMove = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMove"));
	ProjectileMove->UpdatedComponent = CollisionSphere;
	ProjectileMove->bRotationFollowsVelocity = true;

	ProjectileMove->ProjectileGravityScale = 1.0f;
	ProjectileMove->bShouldBounce = false;

	ProjectileMove->InitialSpeed = 1500.f;
	ProjectileMove->MaxSpeed = 1500.f;

	TrailComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComp"));
	TrailComp->SetupAttachment(RootComponent);
	TrailComp->SetAutoActivate(false);

	bCanDamageInstigator = false;

	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AOZGrenadeProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ProjectileMove->OnProjectileStop.AddDynamic(this, &AOZGrenadeProjectile::OnProjectileStopped);

	}

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[Grenade] Spawned Mode=%d Radius=%.1f Ratio=%.2f Instigator=%s"),
			(int32)ImpactMode, ExplosionRadiusCm, DamageRatio, *GetNameSafe(InstigatorPawn.Get()));
	}

	if(TrailComp)
		TrailComp->Activate();
}

void AOZGrenadeProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(StartDelayTimerHandle);
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AOZGrenadeProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZGrenadeProjectile::InitInstigator(APawn* InInstigatorPawn)
{
	InstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);

	if (CollisionSphere && InstigatorPawn)
	{
		CollisionSphere->IgnoreActorWhenMoving(InstigatorPawn.Get(), true);
	}
}

void AOZGrenadeProjectile::InitFromBattleItemData(const FOZBattleItemData Data)
{
	ItemStartDelaySec = static_cast<float>(Data.Item_Start_Delay);

	// 반지름
	ExplosionRadiusCm = Data.Form_Extent;

	// 틱데미지
	DamageRatio = Data.Item_Damage;

	// 섬광탄
	FlashBangDuration = static_cast<float>(Data.Item_Duration);
}

void AOZGrenadeProjectile::Multicast_PlayFireImpactEffect_Implementation(FVector Location)
{
	if (!FireImpactEffect)
		return;
	
	const FRotator Rotation = FRotator::ZeroRotator;
	const FVector Scale = FVector(FireImpactEffectScale);

	UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		FireImpactEffect,
		Location,
		Rotation,
		Scale,
		true,   // bAutoDestroy
		true,   // bAutoActivate
		ENCPoolMethod::AutoRelease
	);

	if (Comp)
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, [Comp]()
			{
				if (IsValid(Comp))
				{
					Comp->Deactivate();
					Comp->DestroyComponent();
				}
			}, 2.0f, false); // 2초 → 이펙트 길이에 맞춰 조절
	}
}

void AOZGrenadeProjectile::InitVelocity(const FVector& InWorldVelocity)
{
	if (!ProjectileMove)
		return;

	ProjectileMove->Velocity = InWorldVelocity;

	ProjectileMove->UpdateComponentVelocity();
}

void AOZGrenadeProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
	if (TrailComp)
		TrailComp->Deactivate();

	if (!HasAuthority() || bImpacted)
		return;

	bImpacted = true;

	// 캐싱
	CachedImpactResult = ImpactResult;

	// 딜레이 시작(중복 방지)
	if (bStartDelayStarted)
		return;
	bStartDelayStarted = true;

	GetWorldTimerManager().ClearTimer(StartDelayTimerHandle);

	if (ItemStartDelaySec <= 0.f)
	{
		OnStartDelayFinished_Server();
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			StartDelayTimerHandle,
			this,
			&AOZGrenadeProjectile::OnStartDelayFinished_Server,
			ItemStartDelaySec,
			false
		);
	}
}

void AOZGrenadeProjectile::HandleImpact_Server(const FHitResult& ImpactResult)
{
	if (!HasAuthority())
		return;

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[Grenade] Impact at %s, HitActor=%s"),
			*ImpactResult.ImpactPoint.ToString(), *GetNameSafe(ImpactResult.GetActor()));
	}

	const FVector SoundLocation = ImpactResult.ImpactPoint;

	switch (ImpactMode)
	{
	case EOZGrenadeImpactMode::ExplodeDamage:
		Multicast_PlayExplosionSound(GrenadeSound, SoundLocation, false);
		ExplodeDamage_Server(ImpactResult);
		break;

	case EOZGrenadeImpactMode::SpawnFireArea:
		Multicast_PlayExplosionSound(FireGrenadeSound, SoundLocation, false);
		SpawnFireArea_Server(ImpactResult);
		break;

	case EOZGrenadeImpactMode::FlashBang:
		Multicast_PlayExplosionSound(FlashBangSound, SoundLocation, false);
		FlashBang_Server(ImpactResult);
		break;

	case EOZGrenadeImpactMode::SpawnSmokeArea:
		Multicast_PlayExplosionSound(SmokeShellSound, SoundLocation, true);  // 본인만 들림
		SpawnSmokeArea_Server(ImpactResult);
		break;

	default:
		break;
	}

	// Multicast RPC가 클라이언트에 도달할 시간 확보 후 파괴 - 이게 없으면 클라이언트에서 사운드 재생 RPC 미작동
	SetLifeSpan(0.1f);
}

void AOZGrenadeProjectile::ExplodeDamage_Server(const FHitResult& ImpactResult)
{
	Multicast_PlayExplosionParticle(ImpactResult.ImpactPoint);

	UAbilitySystemComponent* SourceASC = ResolveSourceASC();
	if (!SourceASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Grenade] No SourceASC. Instigator=%s"), *GetNameSafe(InstigatorPawn.Get()));
		return;
	}

	const FVector Center = ImpactResult.ImpactPoint;
	const float Radius = ExplosionRadiusCm;

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), Center, Radius, 16, FColor::Red, false, 1.5f);
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OZGrenadeExplode), false, this);
	if (!bCanDamageInstigator && InstigatorPawn)
	{
		QueryParams.AddIgnoredActor(InstigatorPawn.Get());
	}

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[Grenade] OverlapMulti bHit=%d Num=%d"), bHit, Overlaps.Num());
	}

	TSet<AActor*> DamagedActors;

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* TargetActor = O.GetActor();
		if (!TargetActor)
			continue;

		if (!bCanDamageInstigator && InstigatorPawn && TargetActor == InstigatorPawn)
			continue;

		if (DamagedActors.Contains(TargetActor))
			continue;
		DamagedActors.Add(TargetActor);

		UAbilitySystemComponent* TargetASC = ResolveTargetASC(TargetActor);
		if (!TargetASC)
			continue;

		ApplyPercentDamageToTarget(SourceASC, TargetActor, TargetASC);
	}
}

void AOZGrenadeProjectile::SpawnFireArea_Server(const FHitResult& ImpactResult)
{
	if (!FireAreaClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FireGrenade] FireAreaClass is null"));
		return;
	}

	Multicast_PlayFireImpactEffect(ImpactResult.ImpactPoint);

	FVector SpawnLoc = ImpactResult.ImpactPoint;

	if (bSnapFireAreaToGround)
	{
		FVector Ground;
		if (FindGroundPointFromImpact(ImpactResult, Ground))
		{
			SpawnLoc = Ground;
		}
	}

	FRotator SpawnRot = FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = InstigatorPawn ? InstigatorPawn.Get() : GetInstigator();

	AOZFireArea* FireArea = GetWorld()->SpawnActor<AOZFireArea>(FireAreaClass, SpawnLoc, SpawnRot, Params);
	if (!FireArea)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FireGrenade] Failed to spawn FireArea"));
		return;
	}

	FireArea->InitInstigator(Params.Instigator);

	UOZItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UOZItemSubsystem>();
	if (ItemSubsystem)
		FireArea->InitFromBattleItemData(*ItemSubsystem->GetBattleItemData(30102));

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), SpawnLoc, 80.f, 12, FColor::Orange, false, 2.f);
		UE_LOG(LogTemp, Log, TEXT("[FireGrenade] Spawn FireArea at %s (snap=%d)"), *SpawnLoc.ToString(), bSnapFireAreaToGround);
	}
}

bool AOZGrenadeProjectile::FindGroundPointFromImpact(const FHitResult& ImpactResult, FVector& OutGroundPoint) const
{
	const FVector Start = ImpactResult.ImpactPoint + FVector(0, 0, GroundTraceUpOffsetCm);
	const FVector End = Start - FVector(0, 0, GroundTraceDownLengthCm);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(OZFireGroundSnap), false, this);
	if (InstigatorPawn)
	{
		Params.AddIgnoredActor(InstigatorPawn.Get());
	}

	FHitResult GroundHit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		GroundHit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	if (bDebugDraw)
	{
		DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f);
	}

	if (!bHit)
		return false;

	OutGroundPoint = GroundHit.ImpactPoint;
	return true;
}

void AOZGrenadeProjectile::Multicast_PlayExplosionSound_Implementation(USoundBase* Sound, FVector SoundLocation, bool bInstigatorOnly)
{
	if (!Sound)
		return;

	// 로컬 플레이어 (리스너)
	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!LocalPawn)
		return;

	// 연막은 본인(Instigator)만 들림
	if (bInstigatorOnly)
	{
		if (InstigatorPawn && LocalPawn == InstigatorPawn)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, SoundLocation, 1.0f);
		}
		return;
	}

	// 거리 기반 볼륨 계산 (로컬 플레이어 기준)
	float Volume = CalculateVolumeByDistance(SoundLocation, LocalPawn->GetActorLocation());
	if (Volume <= 0.f)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, Sound, SoundLocation, Volume);
}

float AOZGrenadeProjectile::CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const
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

UAbilitySystemComponent* AOZGrenadeProjectile::ResolveSourceASC() const
{
	APawn* Pawn = InstigatorPawn ? InstigatorPawn.Get() : GetInstigator();
	if (!Pawn)
		return nullptr;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		return ASI->GetAbilitySystemComponent();
	}

	if (APlayerState* PS = Pawn->GetPlayerState())
	{
		if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(PS))
		{
			return PS_ASI->GetAbilitySystemComponent();
		}
	}
	return nullptr;
}

UAbilitySystemComponent* AOZGrenadeProjectile::ResolveTargetASC(AActor* TargetActor)
{
	if (!TargetActor)
		return nullptr;

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(TargetActor))
	{
		return ASI->GetAbilitySystemComponent();
	}

	if (APawn* Pawn = Cast<APawn>(TargetActor))
	{
		if (APlayerState* PS = Pawn->GetPlayerState())
		{
			if (IAbilitySystemInterface* PS_ASI = Cast<IAbilitySystemInterface>(PS))
			{
				return PS_ASI->GetAbilitySystemComponent();
			}
		}
	}
	return nullptr;
}

bool AOZGrenadeProjectile::ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth)
{
	OutMaxHealth = 0.f;
	if (!TargetASC)
		return false;

	if (const UOZPlayerAttributeSet* PlayerAS = TargetASC->GetSet<UOZPlayerAttributeSet>())
	{
		OutMaxHealth = PlayerAS->GetMaxHealth();
		return OutMaxHealth > 0.f;
	}
	return false;
}

void AOZGrenadeProjectile::ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC)
{
	if (!SourceASC || !TargetASC)
		return;

	float TargetMaxHealth = 0.f;
	if (!ResolveTargetMaxHealth(TargetASC, TargetMaxHealth))
		return;

	const float FinalDamage = TargetMaxHealth * DamageRatio;
	if (FinalDamage <= 0.f)
		return;

	AOZPlayer* InstigatorPlayer = Cast<AOZPlayer>(InstigatorPawn ? InstigatorPawn.Get() : GetInstigator());
	if (!InstigatorPlayer)
		return;

	InstigatorPlayer->ApplyDamageToASC(SourceASC, TargetASC, FinalDamage, nullptr, this);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[Grenade] %s -> %s : %.1f (MaxHP %.1f * %.2f)"),
			*GetNameSafe(this), *GetNameSafe(TargetActor), FinalDamage, TargetMaxHealth, DamageRatio);
	}
}

void AOZGrenadeProjectile::FlashBang_Server(const FHitResult& ImpactResult)
{
	Multicast_PlayFlashBangEffect(ImpactResult.ImpactPoint);

	const FVector Center = ImpactResult.ImpactPoint;
	const float Radius = ExplosionRadiusCm;

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), Center, Radius, 16, FColor::Yellow, false, FlashBangDuration);
		UE_LOG(LogTemp, Log, TEXT("[FlashBang] Exploded at %s, Radius=%.1f, Duration=%.1f"),
			*Center.ToString(), Radius, FlashBangDuration);
	}

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OZFlashBang), false, this);
	if (!bCanDamageInstigator && InstigatorPawn)
	{
		QueryParams.AddIgnoredActor(InstigatorPawn.Get());
	}

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[FlashBang] OverlapMulti bHit=%d Num=%d"), bHit, Overlaps.Num());
	}

	TSet<AActor*> AffectedActors;
	TArray<FVector> HitEffectLocations;
	int32 TotalPlayers = 0;
	int32 AffectedPlayers = 0;

	for (const FOverlapResult& O : Overlaps)
	{
		AActor* TargetActor = O.GetActor();
		if (!TargetActor)
			continue;

		AOZPlayer* TargetPlayer = Cast<AOZPlayer>(TargetActor);
		if (!TargetPlayer)
			continue;

		TotalPlayers++;

		if (!bCanDamageInstigator && InstigatorPawn && TargetActor == InstigatorPawn)
			continue;

		if (AffectedActors.Contains(TargetActor))
			continue;

		AffectedActors.Add(TargetActor);

		if (!TargetPlayer->VisionComp)
			continue;

		TargetPlayer->Multicast_DisableVision(FlashBangDuration);

		if (FlashBangHitEffect)
		{
			TargetPlayer->Client_PlayFlashBangHitEffect(FlashBangHitEffect);
		}

		AffectedPlayers++;
	}

	UE_LOG(LogTemp, Log, TEXT("[FlashBang] Summary: %d players in range, %d affected"), TotalPlayers, AffectedPlayers);
}

void AOZGrenadeProjectile::SpawnSmokeArea_Server(const FHitResult& ImpactResult)
{
	if (!SmokeAreaClass)
		return;

	FVector SpawnLoc = ImpactResult.ImpactPoint;

	if (bSnapSmokeAreaToGround)
	{
		FVector Ground;
		if (FindGroundPointFromImpact(ImpactResult, Ground))
		{
			SpawnLoc = Ground;
		}
	}

	FRotator SpawnRot = FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = InstigatorPawn ? InstigatorPawn.Get() : GetInstigator();

	AOZSmokeArea* SmokeArea = GetWorld()->SpawnActor<AOZSmokeArea>(SmokeAreaClass, SpawnLoc, SpawnRot, Params);
	if (!SmokeArea)
		return;

	SmokeArea->InitInstigator(Params.Instigator);

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), SpawnLoc, 80.f, 12, FColor::White, false, 2.f);
	}
}

void AOZGrenadeProjectile::Multicast_PlayExplosionParticle_Implementation(FVector Location)
{
	if (!ExplosionParticle)
		return;

	const FRotator Rotation = FRotator::ZeroRotator;
	const FVector Scale = FVector(ExplosionParticleScale);

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		ExplosionParticle,
		Location,
		Rotation,
		Scale,
		true,
		EPSCPoolMethod::None
	);
}

void AOZGrenadeProjectile::Multicast_PlayFlashBangEffect_Implementation(FVector Location)
{
	if (!FlashBangEffect)
		return;

	const FRotator Rotation = FRotator::ZeroRotator;
	const FVector Scale = FVector(FlashBangEffectScale);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		FlashBangEffect,
		Location,
		Rotation,
		Scale,
		true,
		true,
		ENCPoolMethod::AutoRelease
	);
}

void AOZGrenadeProjectile::Multicast_PlayFlashBangHitEffect_Implementation(const TArray<FVector>& HitLocations)
{
	if (!FlashBangHitEffect)
		return;

	const FRotator Rotation = FRotator::ZeroRotator;
	const FVector Scale = FVector(1.f);

	for (const FVector& Location : HitLocations)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FlashBangHitEffect,
			Location,
			Rotation,
			Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

}

void AOZGrenadeProjectile::OnStartDelayFinished_Server()
{
	if (!HasAuthority())
		return;

	HandleImpact_Server(CachedImpactResult);
}