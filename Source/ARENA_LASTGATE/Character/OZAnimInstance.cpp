// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/OZAnimInstance.h"
#include "Character/OZPlayer.h"
#include "KismetAnimationLibrary.h"

void UOZAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UOZAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AOZPlayer* OZPlayer = Cast<AOZPlayer>(TryGetPawnOwner());
	if (!OZPlayer)
		return;

	Speed = OZPlayer->GetVelocity().Length();
	Direction = UKismetAnimationLibrary::CalculateDirection(OZPlayer->GetVelocity(), OZPlayer->GetActorRotation());

	if (OZPlayer->GetWeaponMesh())
	{
		FVector LeftHandSocket = OZPlayer->GetWeaponMesh()->GetSocketLocation(TEXT("LHand_Socket"));
		FVector OutLocation;
		FRotator OutRotation;
		OZPlayer->GetMesh()->TransformToBoneSpace(TEXT("hand_r"), LeftHandSocket, FRotator::ZeroRotator, OutLocation, OutRotation);

		LeftHandTransform.SetLocation(OutLocation);
		LeftHandTransform.SetRotation(OutRotation.Quaternion());
	}
}