// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Weapon/OZBullet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h" 
#include "Components/CapsuleComponent.h"
#include "Character/OZPlayer.h"
#include "GameplayTagAssetInterface.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Object/OZTaggableActor.h"
#include "Net/UnrealNetwork.h"
#include "Object/OZPenetrationComponent.h"


// Sets default values
AOZBullet::AOZBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // 복제 설정
    bReplicates = true;             
    SetReplicateMovement(true);     

    // 콜리전 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
    CollisionComp->InitSphereRadius(8.f);

    CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);

    CollisionComp->SetGenerateOverlapEvents(true);

    RootComponent = CollisionComp;

    BulletNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BulletNiagaraComp"));
    BulletNiagaraComp->SetupAttachment(RootComponent);
    BulletNiagaraComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BulletNiagaraComp->SetGenerateOverlapEvents(false);
    BulletNiagaraComp->bAutoActivate = true; 

    // ProjectileMovement 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    PenetrationBlockedTag = FGameplayTag::RequestGameplayTag(FName("Object.Penetration.Blocked"));
}

// Called when the game starts or when spawned
void AOZBullet::BeginPlay()
{
	Super::BeginPlay();

    if (CollisionComp)
    {
        CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AOZBullet::OnBulletOverlap);
    }
	
    // 나를 쏜 사람(Instigator) 가져오기
    AActor* MyOwner = GetInstigator();

    if (MyOwner && CollisionComp)
    {
        // 내 콜리전 컴포넌트가 주인과 충돌을 무시하도록 설정
        CollisionComp->IgnoreActorWhenMoving(MyOwner, true);

        ACharacter* OwnerCharacter = Cast<ACharacter>(MyOwner);
        if (OwnerCharacter)
        {
            // 주인의 캡슐에게 "이 총알은 무시해라"고 명령
            OwnerCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(this, true);
        }
    }
}

// Called every frame
void AOZBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZBullet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOZBullet, RepCollisionRadius);
    DOREPLIFETIME(AOZBullet, RepNiagaraScale);
}

void AOZBullet::InitBullet(const FOZBulletInitParams& Params)
{
    Shooter = Params.Shooter;
    CurrentDamage = Params.InitialDamage;

    bRicochet = Params.bRicochet;
    RemainingRicochetCount = FMath::Max(Params.RicochetCount, 0);

    if (CollisionComp)
    {
        RepCollisionRadius = Params.CollisionRadius;

        {
            const float BaseR = FMath::Max(VfxBaseCollisionRadius, 1.f);
            const float TargetR = FMath::Max(VfxTargetCollisionRadius, BaseR + 1.f);

            const float BaseS = FMath::Max(VfxBaseScale, 0.01f);   // 1
            const float TargetS = FMath::Max(VfxTargetScale, BaseS); // 4

            const float T = FMath::Clamp((RepCollisionRadius - BaseR) / (TargetR - BaseR), 0.f, 1.f);
            RepNiagaraScale = FMath::Lerp(BaseS, TargetS, T);
            RepNiagaraScale = FMath::Clamp(RepNiagaraScale, 0.01f, VfxMaxScale);
        }

        OnRep_BulletVisual();
    }

    ExplosionRadius = FMath::Max(Params.ExplosionRadius, 0.f);
    ExplosionDamageMult = FMath::Max(Params.ExplosionDamageMult, 0.f);
    ExplosionFalloffRate = (Params.ExplosionFalloffRate <= 0.f) ? 1.f : Params.ExplosionFalloffRate;

    MaxRangeCm = FMath::Max(Params.MaxRangeCm, 0.f);

    ProjectileMovement->InitialSpeed = Params.ProjectileSpeed;
    ProjectileMovement->MaxSpeed = Params.ProjectileSpeed;

    FVector LaunchVel = Params.InitialVelocity;
    if (LaunchVel.IsNearlyZero())
    {
        LaunchVel = GetActorForwardVector() * Params.ProjectileSpeed;
    }
    else
    {
        LaunchVel = LaunchVel.GetSafeNormal() * Params.ProjectileSpeed;
    }

    ProjectileMovement->Velocity = LaunchVel;
    ProjectileMovement->UpdateComponentVelocity();

    if (HasAuthority())
    {
        const bool bMoved = TrySpawnSafeLocation(LaunchVel);

        if (bMoved)
        {
            if (CollisionComp)
            {
                CollisionComp->UpdateComponentToWorld();
                CollisionComp->UpdateOverlaps();
            }
            if (ProjectileMovement)
            {
                ProjectileMovement->UpdateComponentVelocity();
            }
        }
    }

    if (HasAuthority() && MaxRangeCm > 0.f)
    {
        StartLocation = GetActorLocation();

        if (!GetWorldTimerManager().IsTimerActive(RangeCheckTimerHandle))
        {
            GetWorldTimerManager().SetTimer(
                RangeCheckTimerHandle,
                this,
                &AOZBullet::CheckRangeExceeded,
                0.03f,
                true
            );
        }
    }

    bInitBullet = true;
}

void AOZBullet::OnRep_BulletVisual()
{
    if (CollisionComp)
    {
        CollisionComp->SetSphereRadius(RepCollisionRadius, true);
        CollisionComp->UpdateOverlaps();
    }

    if (BulletNiagaraComp)
    {
        BulletNiagaraComp->SetVariableFloat(BulletScaleUserParamName, RepNiagaraScale);
    }
}


void AOZBullet::OnBulletOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bHandlingOverlap)
        return;
    TGuardValue<bool> Guard(bHandlingOverlap, true);

    if (!bInitBullet)
        return;
    
    if (!HasAuthority())
        return;

    if (!OtherActor || OtherActor == this)
        return;

    // 자기 자신(발사자) 맞는 경우 무시
    if (OtherActor == GetInstigator() || OtherActor == Shooter)
        return;

    // APawn 또는 IAbilitySystemInterface를 구현한 Actor 처리
    bool bIsTargetActor = OtherActor->IsA<APawn>() || OtherActor->Implements<UAbilitySystemInterface>();

    if (bIsTargetActor)
    {
        if (APawn* InstPawn = GetInstigator())
        {
            if (AOZPlayer* ShooterChar = Cast<AOZPlayer>(InstPawn))
            {
                ShooterChar->HandleProjectileHit(OtherActor, SweepResult, this);
                const FVector Center = !SweepResult.ImpactPoint.IsZero() ? SweepResult.ImpactPoint : SweepResult.Location;
                ExecuteExplosion(Center);
            }
        }

        Destroy();
        return;
    }

    const FVector Center = !SweepResult.ImpactPoint.IsZero() ? SweepResult.ImpactPoint : SweepResult.Location;
    ExecuteExplosion(Center);

    if (bRicochet)
    {
        if (RemainingRicochetCount <= 0)
        {
            Destroy();
            return;
        }
        
        PerformRicochet(OtherActor, OtherComp, SweepResult); 
        return;
    }

    if (PenetratedActors.Contains(OtherActor))
    {
        return;
    }

    UE_LOG(LogTemp, Log,
        TEXT("[Penetration] TRY | BulletDamage=%.2f | Actor=%s"),
        CurrentDamage,
        *OtherActor->GetName()
    );

    UOZPenetrationComponent* PenComp =
        OtherActor->FindComponentByClass<UOZPenetrationComponent>();

    if (!PenComp)
    {
        Destroy();
        return;
    }

    if (PenComp->bPenetrationBlocked)
    {
        Destroy();
        return;
    }

    const int32 BulletPierceLevel = CalcPierceLevel(CurrentDamage);

    if (BulletPierceLevel == 0)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[Penetration] FAIL | BulletLv=0 | Damage=%.2f"),
            CurrentDamage
        );

        Destroy();
        return;
    }

    const int32 ObjectResistLevel = PenComp->PierceResistLevel;

    // 레벨 비교
    if (BulletPierceLevel < ObjectResistLevel)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[Penetration] FAIL | BulletLv=%d Damage=%.2f | Object=%s ResistLv=%d"),
            BulletPierceLevel,
            CurrentDamage,
            *OtherActor->GetName(),
            ObjectResistLevel
        );


        Destroy();
        return;
    }

    // ===== 관통 성공 =====

    PenetratedActors.Add(OtherActor);

    if (CollisionComp)
    {
        // 이후 동일 액터에서 Overlap가 연타로 들어오는 것 방지
        CollisionComp->IgnoreActorWhenMoving(OtherActor, true);
    }

    const float PrevDamage = CurrentDamage;
    const float ReduceRate = GetPenetrationReduceRate(ObjectResistLevel);

    CurrentDamage *= (1.f - ReduceRate);
    CurrentDamage = FMath::Max(CurrentDamage, 0.f);   

    const int32 NewBulletLevel = CalcPierceLevel(CurrentDamage);

    UE_LOG(LogTemp, Log,
        TEXT("[Penetration] SUCCESS | Actor=%s | ResistLv=%d | Rate=%.2f | %.2f -> %.2f | NewLv=%d"),
        *OtherActor->GetName(),
        ObjectResistLevel,
        ReduceRate,
        PrevDamage,
        CurrentDamage,
        NewBulletLevel
    );

}

void AOZBullet::PerformRicochet(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult)
{
    if (!HasAuthority() || !ProjectileMovement)
        return;

    const float Now = GetWorld()->GetTimeSeconds();

    // 1) 도탄 쿨다운(벽에 붙어서 떨림 방지)
    if (Now - LastRicochetTimeSeconds < RicochetCooldownSeconds)
        return;

    // 같은 컴포넌트 연타 방지 보조
    if (LastRicochetComp.IsValid() && LastRicochetComp.Get() == OtherComp)
    {
        return;
    }

    const FVector V = ProjectileMovement->Velocity;
    const float Speed = V.Size();
    if (Speed <= KINDA_SMALL_NUMBER)
        return;

    // 2) Normal 확보 (실패해도 무조건 도탄해야 하므로 fallback)
    FVector N;
    if (!TryGetRicochetNormal(OtherActor, OtherComp, SweepResult, N))
    {
        N = LastValidRicochetNormal;
        if (N.IsNearlyZero())
            N = FVector::UpVector;
    }
    N = N.GetSafeNormal();

    // 3) 정석 물리 반사
    FVector R = V - 2.f * FVector::DotProduct(V, N) * N;
    FVector NewDir = R.GetSafeNormal();
    if (NewDir.IsNearlyZero())
        NewDir = (-V).GetSafeNormal();

    float NewSpeed = Speed * RicochetSpeedRetain;
    if (NewSpeed < 1.f)
        NewSpeed = 1.f;

    const FVector NewVel = NewDir * NewSpeed;

    ProjectileMovement->Velocity = NewVel;            
    ProjectileMovement->UpdateComponentVelocity();
    SetActorRotation(NewVel.Rotation());

    // 4) Push-out (표면에서 떼어내기)
    if (RicochetPushOutDistance > 0.f)
    {
        SetActorLocation(GetActorLocation() + N * RicochetPushOutDistance, false, nullptr, ETeleportType::TeleportPhysics);
    }

    // 5) 캐시 업데이트
    LastRicochetTimeSeconds = Now;
    LastValidRicochetNormal = N;
    LastRicochetComp = OtherComp;

    RemainingRicochetCount = FMath::Max(RemainingRicochetCount - 1, 0);
}

bool AOZBullet::TryGetRicochetNormal(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& SweepResult, FVector& OutNormal) const
{
    // 1) SweepResult에 정상 normal 있으면 우선 사용
    if (!SweepResult.ImpactNormal.IsNearlyZero())
    {
        OutNormal = SweepResult.ImpactNormal.GetSafeNormal();
        return true;
    }
    if (!SweepResult.Normal.IsNearlyZero())
    {
        OutNormal = SweepResult.Normal.GetSafeNormal();
        return true;
    }

    // 2) 보강 sweep: 현재 위치에서 진행방향 반대로 짧게 쏴서 표면 normal 확보
    if (!ProjectileMovement)
        return false;

    const FVector V = ProjectileMovement->Velocity;
    const FVector Dir = V.GetSafeNormal();
    if (Dir.IsNearlyZero())
        return false;

    const FVector Start = GetActorLocation();
    const FVector End = Start - Dir * RicochetTraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(RicochetSweep), false, this);
    Params.AddIgnoredActor(this);
    if (Shooter) Params.AddIgnoredActor(Shooter.Get());

    // 탄 반경 기반 sweep (CollisionComp 반경 사용)
    const float Radius = CollisionComp ? CollisionComp->GetScaledSphereRadius() : 8.f;

    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_Visibility, // 필요하면 프로젝트용 채널로 교체
        FCollisionShape::MakeSphere(Radius),
        Params
    );

    if (bHit && !Hit.ImpactNormal.IsNearlyZero())
    {
        OutNormal = Hit.ImpactNormal.GetSafeNormal();
        return true;
    }

    return false;
}

void AOZBullet::ExecuteExplosion(const FVector& Center)
{
    if (!HasAuthority())
        return;

    if (ExplosionRadius <= 0.f || ExplosionDamageMult <= 0.f)
        return;

    AOZPlayer* ShooterPlayer = Cast<AOZPlayer>(Shooter);
    if (!ShooterPlayer)
        return;

    UAbilitySystemComponent* SourceASC = ShooterPlayer->GetAbilitySystemComponent();
    if (!SourceASC)
        return;

    const float R = ExplosionRadius;
    const float MaxExplosionDamage = CurrentDamage * ExplosionDamageMult;

    // 플레이어만 수집
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ExplosionOverlap), false, this);
    Params.AddIgnoredActor(this);
    if (Shooter) Params.AddIgnoredActor(Shooter);

    // Pawn만 잡히게 ObjectType을 제한하는 게 좋음
    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_Pawn);

    GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        Center,
        FQuat::Identity,
        ObjParams,
        FCollisionShape::MakeSphere(R),
        Params
    );

    TSet<TWeakObjectPtr<AOZPlayer>> UniqueTargets;

    for (const FOverlapResult& O : Overlaps)
    {
        AOZPlayer* TargetPlayer = Cast<AOZPlayer>(O.GetActor());
        if (!TargetPlayer) continue;

        // 발사자 제외(정책에 맞게 유지/삭제)
        if (TargetPlayer == ShooterPlayer) continue;

        UniqueTargets.Add(TargetPlayer);
    }

    Multicast_PlayExplosionFX(Center, ExplosionRadius);

    for (TWeakObjectPtr<AOZPlayer> TgtPtr : UniqueTargets)
    {
        if (!TgtPtr.IsValid()) continue;

        AOZPlayer* TargetPlayer = TgtPtr.Get();
        UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
        if (!TargetASC) continue;

        const float D = FVector::Distance(TargetPlayer->GetActorLocation(), Center);
        if (D >= R) continue;

        const float T = 1.f - (D / R) * ExplosionFalloffRate;
        const float Final = MaxExplosionDamage * FMath::Clamp(T, 0.f, 1.f);
        if (Final <= 0.f) continue;

        if (!HasExplosionLineOfSight(Center, TargetPlayer))
            continue;

        // HitResult는 폭발이라 "없어도 됨". 필요하면 간단히 만들어서 넣을 수도 있음.
        ShooterPlayer->ApplyDamageToASC(SourceASC, TargetASC, Final, /*OptionalHit*/ nullptr, /*SourceObject*/ this);
    }

    if (bDrawExplosionDebug)
    {
        DrawDebugSphere(GetWorld(), Center, R, 16, FColor::Orange, false, 0.8f);
    }
}

bool AOZBullet::HasExplosionLineOfSight(const FVector& Center, const AActor* TargetActor) const
{
    if (!HasAuthority())
        return false;

    if (!TargetActor)
        return false;

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ExplosionLOS), false, this);
    TraceParams.AddIgnoredActor(this);

    if (Shooter)
        TraceParams.AddIgnoredActor(Shooter.Get()); // Shooter는 막는 대상이 아니므로 무시

    TraceParams.AddIgnoredActor(TargetActor); // 자기 자신 캡슐에 막히는 것 방지

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic); 

    FHitResult BlockHit;
    const FVector Start = Center;
    const FVector End = TargetActor->GetActorLocation();

    const bool bBlocked = GetWorld()->LineTraceSingleByObjectType(
        BlockHit,
        Start,
        End,
        ObjParams,
        TraceParams
    );

    if (bDrawExplosionDebug)
    {
        const FVector DebugEnd = bBlocked ? BlockHit.ImpactPoint : End;
        DrawDebugLine(GetWorld(), Start, DebugEnd, bBlocked ? FColor::Red : FColor::Green, false, 0.8f, 0, 1.f);
    }

    return !bBlocked;
}


void AOZBullet::Multicast_PlayExplosionFX_Implementation(FVector Center, float Radius)
{
    // 전용 서버에서는 VFX 필요 없음
    if (GetNetMode() == NM_DedicatedServer)
        return;

    if (!ExplosionNiagara)
        return;

    UNiagaraComponent* NiagaraComp =
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ExplosionNiagara,
            Center,
            FRotator::ZeroRotator,
            FVector::OneVector,
            true,   // AutoDestroy
            true,   // AutoActivate
            ENCPoolMethod::AutoRelease,
            true
        );

    if (!NiagaraComp)
        return;

    // User.Scale 사용
    const float NiagaraScale = FMath::Max(Radius * ExplosionVfxScalePerRadius, 0.01f);
    NiagaraComp->SetVariableFloat(TEXT("User.Scale"), NiagaraScale);
}

int32 AOZBullet::CalcPierceLevel(float Damage) const
{
    if (Damage >= 720.f) return 3;
    if (Damage >= 280.f) return 2;
    if (Damage >= 85.f)  return 1;
    return 0; // Lv0: 관통 불가
}

float AOZBullet::GetPenetrationReduceRate(int32 ObjectResistLevel) const
{
    if (ObjectResistLevel <= 1) return 0.20f;
    if (ObjectResistLevel == 2) return 0.45f;
    return 0.80f; // 3 이상
}

void AOZBullet::CheckRangeExceeded()
{
    if (!HasAuthority())
        return;

    const float Dist = FVector::DistSquared(GetActorLocation(), StartLocation);
    if (Dist >= FMath::Square(MaxRangeCm))
    {
        Destroy();
    }
}


bool AOZBullet::TrySpawnSafeLocation(const FVector& ShootDir)
{
    if (!HasAuthority() || !CollisionComp)
        return false;

    UWorld* World = GetWorld();
    if (!World)
        return false;

    const float Radius = CollisionComp->GetScaledSphereRadius();
    const float CheckRadius = Radius + 1.0f; 

    FCollisionQueryParams Params(SCENE_QUERY_STAT(BulletSpawnOverlap), false, this);
    Params.AddIgnoredActor(this);
    if (Shooter) Params.AddIgnoredActor(Shooter.Get());
    if (APawn* Inst = GetInstigator()) Params.AddIgnoredActor(Inst);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
    ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

    auto IsOverlappingAt = [&](const FVector& Loc) -> bool
        {
            TArray<FOverlapResult> Overlaps;
            const bool bAny = World->OverlapMultiByObjectType(
                Overlaps,
                Loc,
                FQuat::Identity,
                ObjParams,
                FCollisionShape::MakeSphere(CheckRadius),
                Params
            );
            return bAny;
        };

    const FVector Dir = ShootDir.IsNearlyZero()
        ? GetActorForwardVector()
        : ShootDir.GetSafeNormal();

    const FVector Back = -Dir;
    FVector Loc = GetActorLocation();

    if (!IsOverlappingAt(Loc))
        return false;

    const float Step = FMath::Max(Radius * 1.0f, 5.f);
    const int32 MaxSteps = 24;                          

    for (int32 i = 0; i < MaxSteps; ++i)
    {
        Loc += Back * Step;

        if (!IsOverlappingAt(Loc))
        {
            SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
            return true;
        }
    }

    return false;
}