// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OZMinimapVisibleObject.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AOZMinimapVisibleObject::AOZMinimapVisibleObject()
{
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	// Set as Root Component
	RootComponent = StaticMeshComponent;
}

// Called when the game starts or when spawned
void AOZMinimapVisibleObject::BeginPlay()
{
	Super::BeginPlay();
	
}

