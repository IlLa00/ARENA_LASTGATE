// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OZPenetrationComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARENA_LASTGATE_API UOZPenetrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOZPenetrationComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Penetration")
	bool bPenetrationBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Penetration", meta = (ClampMin = "0"))
	int32 PierceResistLevel = 1;

	// 컴포넌트 붙이면 메시 OverlapEvents 자동 활성화
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Penetration|Collision")
	bool bAutoEnableOverlapEvents = true;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyOverlapEventSettings();
};
