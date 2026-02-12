// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZPenetrationComponent.h"

// Sets default values for this component's properties
UOZPenetrationComponent::UOZPenetrationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UOZPenetrationComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyOverlapEventSettings();
}

#if WITH_EDITOR
void UOZPenetrationComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyOverlapEventSettings();
}
#endif


// Called every frame
void UOZPenetrationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UOZPenetrationComponent::ApplyOverlapEventSettings()
{
	if (!bAutoEnableOverlapEvents)
		return;

	AActor* Owner = GetOwner();
	if (!Owner)
		return;

	TArray<UStaticMeshComponent*> Meshes;
	Owner->GetComponents<UStaticMeshComponent>(Meshes);

	for (UStaticMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		if (!Mesh->GetGenerateOverlapEvents())
		{
			Mesh->SetGenerateOverlapEvents(true);
		}
	}
}