#include "OZBush.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Character/OZPlayerState.h"
#include "Tags/OZGameplayTags.h"


// Sets default values
AOZBush::AOZBush()
{
    bReplicates = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BushCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BushCollision"));
    BushCollision->SetupAttachment(Root);
    BushCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BushCollision->SetGenerateOverlapEvents(true);

    BushCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    BushCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOZBush::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        BushCollision->OnComponentBeginOverlap.AddDynamic(this, &AOZBush::OnBushBeginOverlap);
        BushCollision->OnComponentEndOverlap.AddDynamic(this, &AOZBush::OnBushEndOverlap);
    }
	
}

void AOZBush::OnBushBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    SetInBushTag(OtherActor, true);
}

void AOZBush::OnBushEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    SetInBushTag(OtherActor, false);
}

void AOZBush::SetInBushTag(AActor* OtherActor, bool bEnter)
{
    if (!HasAuthority() || !OtherActor)
        return;

    APawn* Pawn = Cast<APawn>(OtherActor);
    if (!Pawn) return;

    AOZPlayerState* PS = Pawn->GetPlayerState<AOZPlayerState>();
    if (!PS) return;

    PS->SetInBush_Server(bEnter);

    if (bEnter)
    {
        PS->SetCurrentBushID(BushID);
    }
    else
    {
        if (PS->GetCurrentBushID() == BushID)
            PS->SetCurrentBushID(INDEX_NONE);
        
    }
}

// Called every frame
void AOZBush::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

