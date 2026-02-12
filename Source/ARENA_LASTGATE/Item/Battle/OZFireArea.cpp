// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Battle/OZFireArea.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerState.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Data/OZBattleItemData.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values
AOZFireArea::AOZFireArea()
{
	PrimaryActorTick.bCanEverTick = false;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	SetRootComponent(AreaSphere);

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaSphere->SetGenerateOverlapEvents(true);

	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bReplicates = true;
	SetReplicateMovement(false);
}

// Called when the game starts or when spawned
void AOZFireArea::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetSphereRadius(FMath::Max(0.f, RadiusCm));

	// 오버랩 이벤트 바인딩 (서버에서만 처리)
	if (HasAuthority())
	{
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AOZFireArea::OnAreaBeginOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AOZFireArea::OnAreaEndOverlap);
	}

	// 화염 이펙트 재생 (서버/클라이언트 모두 BeginPlay 호출됨)
	if (FireEffect)
	{
		//const FVector Scale = FVector(FireEffectScale);

		FireEffectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			FireEffect,
			AreaSphere,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(1, 1, 1),
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::None
		);

		FireEffectComp->SetNiagaraVariableFloat(TEXT("Radius"), FireEffectScale);
	}

	if (bDebugDraw)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), RadiusCm, 16, FColor::Orange, false, 5.f);
		UE_LOG(LogTemp, Log, TEXT("[FireArea] Spawned. Radius=%.1f, Ratio=%.2f, Tick=%.2f, Life=%.2f, Instigator=%s"),
			RadiusCm, DamageRatio, TickIntervalSec, InstallationTimeSec, *GetNameSafe(InstigatorPawn.Get()));
	}

	if (!HasAuthority())
		return;

	if (InstallationTimeSec > 0.f)
	{
		SetLifeSpan(InstallationTimeSec);
	}

	if (TickIntervalSec > 0.f && DamageRatio > 0.f && RadiusCm > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			TickTimerHandle,
			this,
			&AOZFireArea::TickDamage_Server,
			TickIntervalSec,
			true,
			0.f
		);
	}
}

void AOZFireArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZFireArea::InitInstigator(APawn* InInstigatorPawn)
{
	InstigatorPawn = InInstigatorPawn;
	SetInstigator(InInstigatorPawn);
}

void AOZFireArea::InitFromBattleItemData(const FOZBattleItemData Data)
{
	RadiusCm = Data.Form_Extent;
	DamageRatio = Data.Item_Damage;

	if (Data.Damage_Over_Time > 0.f)
	{
		TickIntervalSec = Data.Damage_Over_Time;
	}


	InstallationTimeSec = static_cast<float>(Data.Item_Installation_Time);
}

void AOZFireArea::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(TickTimerHandle);

		// 영역 내 모든 플레이어의 화염 이펙트 정지
		for (AOZPlayer* Player : PlayersInArea)
		{
			if (Player && IsValid(Player))
			{
				Player->Client_StopEffect();
			}
		}
		PlayersInArea.Empty();
	}

	if (FireEffectComp)
	{
		FireEffectComp->Deactivate();
		FireEffectComp->DestroyComponent();
		FireEffectComp = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AOZFireArea::TickDamage_Server()
{
	if (!HasAuthority())
		return;

	UAbilitySystemComponent* SourceASC = ResolveSourceASC();
	if (!SourceASC)
	{
		if (bDebugDraw)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FireArea] No SourceASC. InstigatorPawn=%s"), *GetNameSafe(InstigatorPawn.Get()));
		}
		return;
	}

	TArray<AActor*> OverlappingActors;
	AreaSphere->GetOverlappingActors(OverlappingActors);

	if (OverlappingActors.Num() == 0)
		return;

	for (AActor* TargetActor : OverlappingActors)
	{
		if (!TargetActor)
			continue;

		UAbilitySystemComponent* TargetASC = ResolveTargetASC(TargetActor);
		if (!TargetASC)
			continue;

		ApplyPercentDamageToTarget(SourceASC, TargetActor, TargetASC);
	}
}

UAbilitySystemComponent* AOZFireArea::ResolveTargetASC(AActor* TargetActor)
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

UAbilitySystemComponent* AOZFireArea::ResolveSourceASC() const
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

bool AOZFireArea::ResolveTargetMaxHealth(UAbilitySystemComponent* TargetASC, float& OutMaxHealth)
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

void AOZFireArea::ApplyPercentDamageToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor, UAbilitySystemComponent* TargetASC)
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
	{
		return;
	}

	InstigatorPlayer->ApplyDamageToASC(SourceASC, TargetASC, FinalDamage, nullptr, this);

	if (bDebugDraw)
	{
		UE_LOG(LogTemp, Log, TEXT("[FireArea] %s -> %s : %.1f (MaxHP %.1f * %.2f)"),
			*GetNameSafe(this), *GetNameSafe(TargetActor), FinalDamage, TargetMaxHealth, DamageRatio);
	}
}

void AOZFireArea::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;

	AOZPlayer* Player = Cast<AOZPlayer>(OtherActor);
	if (!Player)
		return;

	if (PlayersInArea.Contains(Player))
		return;

	PlayersInArea.Add(Player);

	Player->Client_StartEffect(EPlayerEffectType::Fire);
}

void AOZFireArea::OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
		return;

	AOZPlayer* Player = Cast<AOZPlayer>(OtherActor);
	if (!Player)
		return;

	if (!PlayersInArea.Contains(Player))
		return;

	PlayersInArea.Remove(Player);

	Player->Client_StopEffect();
}