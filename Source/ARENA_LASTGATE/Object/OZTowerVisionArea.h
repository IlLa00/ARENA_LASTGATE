#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OZTowerVisionArea.generated.h"

class USphereComponent;
class AOZPlayer;

UCLASS()
class ARENA_LASTGATE_API AOZTowerVisionArea : public AActor
{
	GENERATED_BODY()

public:
	AOZTowerVisionArea();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tower Vision")
	void Activate(AOZPlayer* ActivatingPlayer);

	UFUNCTION(BlueprintCallable, Category = "Tower Vision")
	void Deactivate();

	UFUNCTION(BlueprintPure, Category = "Tower Vision")
	bool IsActivated() const { return bIsActivated; }

	UFUNCTION(BlueprintPure, Category = "Tower Vision")
	AOZPlayer* GetOwningPlayer() const { return OwningPlayer; }

	UFUNCTION(BlueprintPure, Category = "Tower Vision")
	float GetVisionRadius() const { return VisionRadius; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Vision")
	float VisionRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Vision|Debug")
	bool bShowDebugSphere = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Vision|Debug")
	FColor DebugColorInactive = FColor::Cyan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Vision|Debug")
	FColor DebugColorActive = FColor::Green;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Vision")
	TObjectPtr<USphereComponent> VisionSphere;

	UPROPERTY(ReplicatedUsing = OnRep_IsActivated, BlueprintReadOnly, Category = "Tower Vision")
	bool bIsActivated = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tower Vision")
	TObjectPtr<AOZPlayer> OwningPlayer;

	UFUNCTION()
	void OnRep_IsActivated();
};
