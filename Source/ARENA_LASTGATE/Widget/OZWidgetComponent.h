// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "OZWidgetComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARENA_LASTGATE_API UOZWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	//5.5엔진 버그로 인해, owner 설정 있어야 함
	void BeginPlay() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TObjectPtr<class AOZPlayerState> OZPlayerState = nullptr;
};
