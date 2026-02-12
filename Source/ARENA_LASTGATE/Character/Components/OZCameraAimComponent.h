// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OZCameraAimComponent.generated.h"

class USpringArmComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENA_LASTGATE_API UOZCameraAimComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOZCameraAimComponent();
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartAim();
	void StopAim();

	void SetAdditionalCameraDistance(float NewDistance);
	void SetDefaultTargetArmLength(float NewDefaultLength);

private:
	bool bIsAiming = false;

	UPROPERTY()
	USpringArmComponent* CameraBoom = nullptr;

	FVector DefaultSocketOffset = FVector::ZeroVector;
	float DefaultTargetArmLength = 0.f;

	UPROPERTY(EditAnywhere, Category = "Camera Aim")
	FVector BaseAimOffset = FVector(0.f, 40.f, 10.f);

	UPROPERTY(EditAnywhere, Category = "Camera Aim")
	float AdditionalCameraDistance = 0.f;
	

	UPROPERTY(EditAnywhere, Category = "Camera Aim")
	float HorizontalIntensity = 30.f;   // �¿�
	UPROPERTY(EditAnywhere, Category = "Camera Aim")
	float VerticalIntensity = 15.f;   // ����

	UPROPERTY(EditAnywhere, Category = "Camera Aim")
	float InterpSpeed = 5.f;
};
