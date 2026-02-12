#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/OZInteractable.h"
#include "OZTower.generated.h"

class AOZTowerVisionArea;
class AOZPlayer;
class UNiagaraComponent;
class USoundBase;

UENUM(BlueprintType)
enum class EOZTowerState : uint8
{
	Inactive,
	Activating,
	Active,
	Deactivating
};

UCLASS()
class ARENA_LASTGATE_API AOZTower : public AActor, public IOZInteractable
{
	GENERATED_BODY()

public:
	AOZTower();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnInteract(AActor* InteractActor);

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void ActivateTower(AOZPlayer* ActivatingPlayer);

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void DeactivateTower();

	// 라운드 시작 시 타워를 초기 상태로 리셋
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void ResetTower();

	UFUNCTION(BlueprintPure, Category = "Tower")
	bool IsActivated() const { return TowerState == EOZTowerState::Active; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	AOZPlayer* GetOwningPlayer() const { return OwningPlayer; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	float GetRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "Tower")
	bool IsInsideBlackFog() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartEffect(FLinearColor BandanaColor);

	UFUNCTION(BlueprintImplementableEvent)
	void StartEffect(FLinearColor BandanaColor);

	// 모든 플레이어에게 활성화 사운드 재생 (서버에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Tower|Sound")
	void PlayActivationSoundToAll();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayActivationSound();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Mesh")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower|Effect")
	TObjectPtr<UNiagaraComponent> TowerEffectComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Effect")
	float EffectRadiusOffset = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Effect")
	FLinearColor InactiveEffectColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Effect")
	FLinearColor SelfActivatedEffectColor = FLinearColor(0.f, 1.f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Effect")
	FLinearColor OtherActivatedEffectColor = FLinearColor(1.f, 0.05f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Vision Areas")
	TArray<TObjectPtr<AOZTowerVisionArea>> LinkedVisionAreas;

	// 타워 활성화 시 재생되는 사운드 (블루프린트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Sound")
	TObjectPtr<USoundBase> ActivationSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Settings", meta = (ClampMin = "1.0"))
	float ActiveDurationSec = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Settings", meta = (ClampMin = "0.0"))
	float TransitionTimeSec = 0.2f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TowerState, Category = "Tower|State")
	EOZTowerState TowerState = EOZTowerState::Inactive;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OwningPlayer, Category = "Tower|State")
	TObjectPtr<AOZPlayer> OwningPlayer;

protected:
	UFUNCTION()
	void OnRep_TowerState();

	UFUNCTION()
	void OnRep_OwningPlayer(AOZPlayer* OldPlayer);

	void HandleOwnerChanged(AOZPlayer* OldPlayer);

	void UpdateTowerEffect();
	FLinearColor GetEffectColorForLocalPlayer() const;

	void StartActivation();
	void FinishActivation();
	void StartDeactivation();
	void FinishDeactivation();

	void CheckBlackFog();

	void ActivateAllLinkedAreas();
	void DeactivateAllLinkedAreas();

protected:
	FTimerHandle Timer_ActiveDuration;
	FTimerHandle Timer_Transition;
	FTimerHandle Timer_FogCheck;

	float ActivationStartTime = 0.f;

private:
	void UpdateTowerVisionToControllerUI(AOZPlayer* ActivatingPlayer, AOZPlayer* OldPlayer);
};
