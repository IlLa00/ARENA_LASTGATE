#include "Object/OZTowerVisionArea.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "Character/OZPlayer.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AOZTowerVisionArea::AOZTowerVisionArea()
{
	PrimaryActorTick.bCanEverTick = true;

	VisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VisionSphere"));
	SetRootComponent(VisionSphere);

	VisionSphere->SetSphereRadius(VisionRadius);
	VisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisionSphere->SetGenerateOverlapEvents(false);
	VisionSphere->SetHiddenInGame(false);
	VisionSphere->ShapeColor = DebugColorInactive;

	bReplicates = true;
	SetReplicateMovement(false);
}

void AOZTowerVisionArea::BeginPlay()
{
	Super::BeginPlay();

	VisionSphere->SetSphereRadius(VisionRadius);
}

void AOZTowerVisionArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOZTowerVisionArea, bIsActivated);
	DOREPLIFETIME(AOZTowerVisionArea, OwningPlayer);
}

void AOZTowerVisionArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZTowerVisionArea::Activate(AOZPlayer* ActivatingPlayer)
{
	if (!HasAuthority())
		return;

	if (!ActivatingPlayer)
		return;

	OwningPlayer = ActivatingPlayer;
	bIsActivated = true;

	// 서버에서 직접 호출 시 RepNotify 수동 호출
	OnRep_IsActivated();
}

void AOZTowerVisionArea::Deactivate()
{
	if (!HasAuthority())
		return;

	OwningPlayer = nullptr;
	bIsActivated = false;

	// 서버에서 직접 호출 시 RepNotify 수동 호출
	OnRep_IsActivated();
}

void AOZTowerVisionArea::OnRep_IsActivated()
{
	VisionSphere->ShapeColor = bIsActivated ? DebugColorActive : DebugColorInactive;
}
