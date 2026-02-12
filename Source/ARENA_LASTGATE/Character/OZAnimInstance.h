// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "OZAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Direction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimPlayRate = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LeftHandTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsCombat = false;
};
