#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "OZPuppet.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZPuppet : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AOZPuppet();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY()
	TObjectPtr<class UOZPuppetAttributeSet> PuppetAttributes;
};
