#include "Object/OZPuppet.h"
#include "AbilitySystemComponent.h"
#include "Object/OZPuppetAttributeSet.h"

AOZPuppet::AOZPuppet()
{
	PrimaryActorTick.bCanEverTick = true;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	PuppetAttributes = CreateDefaultSubobject<UOZPuppetAttributeSet>(TEXT("PuppetAttributeSet"));
}

void AOZPuppet::BeginPlay()
{
	Super::BeginPlay();

	if (ASC && PuppetAttributes)
	{
		ASC->InitAbilityActorInfo(this, this);
		ASC->AddSpawnedAttribute(PuppetAttributes);
	}
}

void AOZPuppet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* AOZPuppet::GetAbilitySystemComponent() const
{
	return ASC;
}

