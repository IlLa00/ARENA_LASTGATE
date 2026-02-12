// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZTaggableActor.h"

// Sets default values
AOZTaggableActor::AOZTaggableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PenetrationBlockedTag = FGameplayTag::RequestGameplayTag(
		FName("Object.Penetration.Blocked")
	);
}

// Called when the game starts or when spawned
void AOZTaggableActor::BeginPlay()
{
	Super::BeginPlay();
	SyncPenetrationTags();
	SyncVisualFlags();
}

#if WITH_EDITOR
void AOZTaggableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AOZTaggableActor, bPenetrationBlocked))
	{
		SyncPenetrationTags();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AOZTaggableActor, bTransparentVisual))
	{
		SyncVisualFlags();
	}
}
#endif

// Called every frame
void AOZTaggableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOZTaggableActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(ObjectTags);
}

void AOZTaggableActor::SyncPenetrationTags()
{
	if (bPenetrationBlocked)
	{
		ObjectTags.AddTag(PenetrationBlockedTag);
	}
	else
	{
		ObjectTags.RemoveTag(PenetrationBlockedTag);
	}
}

void AOZTaggableActor::SyncVisualFlags()
{
	OnTransparentVisualChanged(bTransparentVisual);
}
