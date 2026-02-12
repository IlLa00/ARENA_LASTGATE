
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZBush.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class ARENA_LASTGATE_API AOZBush : public AActor
{
	GENERATED_BODY()
	
public:	
	AOZBush();



protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bush")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bush")
	TObjectPtr<UBoxComponent> BushCollision;

	UFUNCTION()
	void OnBushBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnBushEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

	void SetInBushTag(AActor* OtherActor, bool bInBush);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bush")
	int32 BushID;
};
