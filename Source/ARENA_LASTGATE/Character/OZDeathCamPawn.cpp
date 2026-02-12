// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OZDeathCamPawn.h"

// Sets default values
AOZDeathCamPawn::AOZDeathCamPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOZDeathCamPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOZDeathCamPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AOZDeathCamPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

