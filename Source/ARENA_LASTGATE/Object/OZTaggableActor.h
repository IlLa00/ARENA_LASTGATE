// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "OZTaggableActor.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZTaggableActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOZTaggableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 이 Actor가 가진 GameplayTag들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
	FGameplayTagContainer ObjectTags;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Penetration")
	bool bPenetrationBlocked = false;

	// 관통 저항 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Penetration")
	int32 PierceResistLevel = 1;

	UFUNCTION(BlueprintImplementableEvent, Category = "Visual")
	void OnTransparentVisualChanged(bool bNewTransparent);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	bool bTransparentVisual = false;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Penetration|Tags")
	FGameplayTag PenetrationBlockedTag;

private:
	void SyncPenetrationTags();
	void SyncVisualFlags();
};
