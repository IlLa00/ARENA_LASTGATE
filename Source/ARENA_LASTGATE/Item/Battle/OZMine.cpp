#include "Item/Battle/OZMine.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"
#include "Data/OZBattleItemData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Character/OZPlayerController.h"

// Sets default values
AOZMine::AOZMine()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1) 비행/착지 충돌용 (Root)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);

	CollisionSphere->InitSphereRadius(10.f); // 비행 충돌용 크기(원하면 데이터로)
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetGenerateOverlapEvents(false);

	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); // 비행 중 플레이어에 안 걸리게(추천)

	// 2) 설치 후 "밟기" 감지용
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);

	TriggerSphere->SetSphereRadius(40.f);
	TriggerSphere->SetGenerateOverlapEvents(true);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // <-- 처음엔 꺼둠
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 3) 폭발 범위 계산용(기존 그대로)
	ExplosionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionSphere"));
	ExplosionSphere->SetupAttachment(RootComponent);

	ExplosionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExplosionSphere->SetGenerateOverlapEvents(false);
	ExplosionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExplosionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 4) 투척 이동
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;

	ProjectileMovement->InitialSpeed = 500.f;  // 나중에 BattleItemData로 덮어쓰기 추천
	ProjectileMovement->MaxSpeed = 500.f;
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;

	bReplicates = true;

	// 던지는 모습 클라에서도 보이게 하려면 true가 유리함
	SetReplicateMovement(true);

	// 나이아가라
	AuraNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AuraNiagaraComp"));
	AuraNiagaraComp->SetupAttachment(RootComponent);
	AuraNiagaraComp->bAutoActivate = false;
	AuraNiagaraComp->SetAutoActivate(false);
}

// Called when the game starts or when spawned
void AOZMine::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerSphere->SetSphereRadius(80.f);
	ExplosionSphere->SetSphereRadius(FMath::Max(0.f, ExplosionRadiusCm));

	if (HasAuthority())
	{
		// 착지 감지(비행 충돌)
		CollisionSphere->OnComponentHit.AddDynamic(this, &AOZMine::OnCollisionHit);

		// 설치 후 밟기 감지(설치되면 TriggerSphere collision을 켤 거라 미리 바인딩 가능)
		//TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AOZMine::OnTriggerBeginOverlap);
	}

	if (AuraNiagaraComp)
	{
		if (AuraNiagara)
		{
			AuraNiagaraComp->SetAsset(AuraNiagara);
		}
		AuraNiagaraComp->Deactivate();
	}
	AuraNiagaraComp->SetWorldScale3D(FVector(3.0f));
	UpdateAuraVisibility();
}

// Called every frame
void AOZMine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZMine::OnRep_InstigatorPawn()
{
	UpdateAuraVisibility();
}

void AOZMine::InitInstigator(APawn* InInstigatorPawn)
{
	InstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);

	UpdateAuraVisibility();
}

void AOZMine::InitFromBattleItemData(const FOZBattleItemData Data)
{
	// Form_Extent → ExplosionRadiusCm
	ExplosionRadiusCm = Data.Form_Extent;

	// Item_Damage → DamageRatio
	DamageRatio = Data.Item_Damage;

	// Item_Installation_Time → InstallationTimeSec (지속 시간)
	InstallationTimeSec = static_cast<float>(Data.Item_Installation_Time);

	ItemStartDelaySec = static_cast<float>(Data.Item_Start_Delay);
}

void AOZMine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZMine, InstigatorPawn);
	DOREPLIFETIME(AOZMine, bArmed);
}

//void AOZMine::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp,
//	AActor* OtherActor,
//	UPrimitiveComponent* OtherComp,
//	int32 OtherBodyIndex,
//	bool bFromSweep,
//	const FHitResult& SweepResult)
//{
//	if (!HasAuthority() || bExploded)
//		return;
//
//	if (TriggerSphere->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
//		return;
//
//	if (!OtherActor || OtherActor == this)
//		return;
//
//	UE_LOG(LogTemp, Warning, TEXT("[Mine] OnTriggerBeginOverlap fired. Other=%s"), *GetNameSafe(OtherActor));
//
//	// ��ġ�ڴ� ��Ƶ� �ߵ����� ���� (�ð������δ� �׳� ����)
//	if (InstigatorPawn && OtherActor == InstigatorPawn)
//	{
//		UE_LOG(LogTemp, Log, TEXT("[Mine] Insigator: %p / OtherActor: %p "), InstigatorPawn.Get(), OtherActor);
//
//		return;
//	}
//
//	// Pawn�� �ƴ� ��(��: ����ü ��)�� �����ϰ� ������ ���⼭ �߰� ���͸� ����
//	Explode_Server();
//}

void AOZMine::Explode_Server()
{
	UE_LOG(LogTemp, Log, TEXT("[Mine] Explode_Server called. Auth=%d"), HasAuthority());


	if (!HasAuthority() || bExploded)
		return;

	GetWorldTimerManager().ClearTimer(StartDelayTimerHandle);
	GetWorldTimerManager().ClearTimer(TriggerPollTimerHandle);
	bExploded = true;

	Multicast_PlayExplosionParticle();
	if (AuraNiagaraComp)
	{
		AuraNiagaraComp->Deactivate();
	}

	UAbilitySystemComponent* SourceASC = ResolveSourceASC();
	if (!SourceASC)
	{
		if (bDebugDraw)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Mine] No SourceASC. Instigator=%s"), *GetNameSafe(InstigatorPawn.Get()));
		}
		SetLifeSpan(0.1f);
		return;
	}

	// ===== ���� ���� �� ��� ���� (���� ������ ����) =====
	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn); // ĳ���Ͱ� Pawn�̸� OK

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OZMineExplode), false, this);

	// ��ġ�ڴ� �ƿ� �������� ����
	if (InstigatorPawn)
	{
		QueryParams.AddIgnoredActor(InstigatorPawn.Get());
	}

	const FVector Center = GetActorLocation();
	const float Radius = ExplosionRadiusCm;

	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Center,
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	UE_LOG(LogTemp, Log, TEXT("[Mine] OverlapMulti bHit=%d Num=%d"), bHit, Overlaps.Num());

	TSet<AActor*> DamagedActors;

	// ===== ������ ���� =====
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* TargetActor = Overlap.GetActor();
		if (!TargetActor) continue;

		if (DamagedActors.Contains(TargetActor))
			continue;
		DamagedActors.Add(TargetActor);

		UAbilitySystemComponent* TargetASC = ResolveTargetASC(TargetActor);
		if (!TargetASC) continue;

		ApplyPercentDamageToTarget(SourceASC, TargetActor, TargetASC);
	}

	// Multicast RPC가 클라이언트에 도달할 시간 확보 후 파괴
	SetLifeSpan(0.1f);
}

UAbilitySystemComponent* AOZMine::ResolveTargetASC(AActor* TargetActor)
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

UAbilitySystemComponent* AOZMine::ResolveSourceASC() const
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

bool AOZMine::ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth)
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

void AOZMine::ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC)
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

	// SourceObject=this �� "����" ��ó ���� ����
	InstigatorPlayer->ApplyDamageToASC(SourceASC, TargetASC, FinalDamage, nullptr, this);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[Mine] %s -> %s : %.1f (MaxHP %.1f * %.2f)"),
			*GetNameSafe(this), *GetNameSafe(TargetActor), FinalDamage, TargetMaxHealth, DamageRatio);
	}
}

void AOZMine::OnCollisionHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bExploded)
		return;

	if (bArmed) 
		return;

	ArmMine_Server(Hit);
}

void AOZMine::ArmMine_Server(const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("[Mine] ArmMine_Server Auth=%d Loc=%s"),
		HasAuthority(), *GetActorLocation().ToString());
	
	if (!HasAuthority() || bExploded)
		return;

	// 1) 이동 정지
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 2) 위치 고정 (원하면 바닥에 살짝 띄우기: ImpactPoint + Normal * Offset)
	const float Lift = 10.f; // 5 ~ 20cm 정도 추천
	SetActorLocation(Hit.ImpactPoint + Hit.ImpactNormal * Lift);

	const FRotator Cur = GetActorRotation();
	SetActorRotation(FRotator(0.f, Cur.Yaw, 0.f));

	if (InstallSound && InstigatorPawn)
	{
		if (APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController()))
		{
			// 네 프로젝트 PC 클래스면 Cast 바꿔
			if (AOZPlayerController* OZPC = Cast<AOZPlayerController>(PC))
			{
				// 원하면 거리감쇠 계산해서 볼륨 전달 가능
				const float Vol = 1.0f;
				OZPC->Client_PlayMineInstallSound(InstallSound, Hit.ImpactPoint);
			}
		}
	}

	// 3) 설치 지속시간 적용
	if (InstallationTimeSec > 0.f)
	{
		SetLifeSpan(InstallationTimeSec);
	}

	// 4) Trigger는 아직 OFF 유지 
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 5) Item_Start_Delay 이후 트리거 활성
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
			&AOZMine::OnStartDelayFinished_Server,
			ItemStartDelaySec,
			false
		);
	}
}

void AOZMine::OnRep_Armed()
{
	UpdateAuraVisibility();
}

void AOZMine::UpdateAuraVisibility()
{
	if (!AuraNiagaraComp)
		return;

	if (GetNetMode() == NM_DedicatedServer)
		return;

	if (!bArmed)
	{
		AuraNiagaraComp->Deactivate();
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* LocalPawn = PC ? PC->GetPawn() : nullptr;

	const bool bIsOwnerLocal = (InstigatorPawn && InstigatorPawn->GetController() && InstigatorPawn->GetController()->IsLocalController());

	if (bIsOwnerLocal)
	{
		if (!AuraNiagaraComp->IsActive())
			AuraNiagaraComp->Activate(true);
	}
	else
	{
		if (AuraNiagaraComp->IsActive())
			AuraNiagaraComp->Deactivate();
	}
}

void AOZMine::PollTrigger_Server()
{
	if (!HasAuthority() || bExploded)
		return;

	if (!bArmed)
		return;


	const FVector Center = TriggerSphere->GetComponentLocation();
	const float Radius = TriggerSphere->GetScaledSphereRadius();

	TArray<FOverlapResult> Overlaps;

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OZMineTriggerPoll), false, this);

	if (InstigatorPawn)
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

	if (!bHit)
		return;

	// Pawn이 하나라도 잡히면 폭발
	for (const FOverlapResult& R : Overlaps)
	{
		if (R.GetActor())
		{
			if (MineSound)
			{
				Multicast_PlayExplosionSound(MineSound, GetActorLocation());
			}

			Explode_Server();
			return;
		}
	}
}

void AOZMine::OnStartDelayFinished_Server()
{
	if (!HasAuthority() || bExploded)
		return;

	bArmed = true;

	UpdateAuraVisibility();

	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->UpdateComponentToWorld();
	TriggerSphere->UpdateOverlaps();

	GetWorldTimerManager().ClearTimer(TriggerPollTimerHandle);
	GetWorldTimerManager().SetTimer(
		TriggerPollTimerHandle,
		this,
		&AOZMine::PollTrigger_Server,
		0.05f,
		true
	);

	UE_LOG(LogTemp, Warning, TEXT("[Mine] Armed after delay=%.2f"), ItemStartDelaySec);

	if (bDebugDraw)
	{
		const FVector TriggerCenter = TriggerSphere->GetComponentLocation();
		DrawDebugSphere(GetWorld(), TriggerCenter, TriggerSphere->GetScaledSphereRadius(), 12, FColor::Cyan, false, 5.f);
		DrawDebugSphere(GetWorld(), TriggerCenter, ExplosionRadiusCm, 16, FColor::Red, false, 5.f);
	}
}

void AOZMine::Multicast_PlayExplosionParticle_Implementation()
{
	if (!ExplosionParticle)
		return;

	const FVector Location = GetActorLocation();
	const FRotator Rotation = GetActorRotation();
	const FVector Scale = FVector(ExplosionParticleScale);

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		ExplosionParticle,
		Location,
		Rotation,
		Scale,
		true,
		EPSCPoolMethod::AutoRelease
	);
}

void AOZMine::Multicast_PlayExplosionSound_Implementation(USoundBase* Sound, FVector SoundLocation)
{
	if (!Sound)
		return;

	// 로컬 플레이어 (리스너)
	APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!LocalPawn)
		return;

	float Volume = CalculateVolumeByDistance(SoundLocation, LocalPawn->GetActorLocation());
	if (Volume <= 0.f)
		return;

	UGameplayStatics::PlaySoundAtLocation(this, Sound, SoundLocation, Volume);
}

float AOZMine::CalculateVolumeByDistance(const FVector& SoundLocation, const FVector& ListenerLocation) const
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