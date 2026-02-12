#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Utils/Util.h"
#include "Data/OZConvertData.h"
#include "Data/OZModuleData.h"
#include "Object/OZWorldBoundCacher.h"
#include "Kismet/GameplayStatics.h"
#include "OZInGameGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatStateEntered);

UCLASS()
class ARENA_LASTGATE_API AOZInGameGameState : public AGameState
{
	GENERATED_BODY()

protected:
	void BeginPlay() override;

public:

	UFUNCTION(NetMulticast, Reliable)
	void HidLoadingScreen();

	UPROPERTY(BlueprintReadWrite)
	uint8 CurrentRound;

	UPROPERTY(Replicated)
	bool bIsAvilityRound = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStartRound(double ServerTime);
	float RoundStartTime;
	float ClientServerRoundStartDelayTime = 0;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStartCombat(double ServerTime);
	float CombatStartTime;
	float ClientServerCombatStartDelayTime = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TargetShrinkingTimeSetted)
	float TargetShrinkingTime;
	UFUNCTION()
	void OnRep_TargetShrinkingTimeSetted();

	UPROPERTY(Replicated)
	int32 totalPlayerNum = 0;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStartResult(double ServerTime);
	float ResultStartTime;
	float ClientServerResultDelayTime = 0;

	float RoundPrepareRemainTime;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentRoundState, meta = (HideInInspector))
	EGameStateType CurrentRoundState = EGameStateType::None;

	UFUNCTION()
	void OnRep_CurrentRoundState();

	UPROPERTY(Replicated, meta = (HideInInspector))
	float prepareStateTime = -1.0f;

	UPROPERTY(Replicated, meta = (HideInInspector))
	float combatStateTime = -1.0f;

	UPROPERTY(Replicated, meta = (HideInInspector))
	float resultStateTime = -1.0f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly)
	TArray<FOZConvertData> SelectedEffectArray;

	UPROPERTY(Replicated, BlueprintReadOnly)
	EConvertGrade CurrentConvertGrade;

	UPROPERTY(BlueprintReadOnly)
	TArray<FOZModuleData> SelectedModuleArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Convert|Probability", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float LegendaryProbability = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Convert|Probability", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float UniqueProbability = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Convert|Probability", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RareProbability = 35.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Convert|Probability")
	EConvertGrade LastGrantedConvertGrade = EConvertGrade::None;

	// 특정 플레이어를 위한 랜덤 증강 선택 (해당 플레이어의 유니크/레전더리 이력 참조)
	UFUNCTION(BlueprintCallable)
	void SelectRandomEffectsForPlayer(class AOZPlayerState* TargetPlayerState, int32 Count = 3);

	UFUNCTION(BlueprintCallable)
	void SelectRandomEffects(int32 Count = 3);

	// 서버에서 계산된 모듈 FinalValue를 모든 클라이언트에 동기화
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SyncModuleFinalValues(const TArray<int32>& ModuleIDs, const TArray<float>& FinalValues);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SyncModuleMinValues(const TArray<int32>& ModuleIDs, const TArray<float>& MinValues);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SyncModuleMaxValues(const TArray<int32>& ModuleIDs, const TArray<float>& MaxValues);

	UFUNCTION(BlueprintCallable)
	void SelectRandomModules(int32 Count = 3);

	UPROPERTY(BlueprintReadOnly, Category = "Module|Reroll")
	TArray<bool> ModuleSlotRerollUsed;

	UPROPERTY(BlueprintReadOnly, Category = "Module|Reroll")
	TArray<int32> OriginalModuleIDs;

	UFUNCTION(BlueprintCallable, Category = "Module|Reroll")
	bool RerollSelectedModule(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Module|Reroll")
	bool CanRerollModuleSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Module|Reroll")
	void ResetModuleRerollStates();

	UPROPERTY(BlueprintReadOnly, Category = "Convert|Reroll")
	TArray<bool> SlotRerollUsed;

	UFUNCTION(BlueprintCallable, Category = "Convert|Reroll")
	bool RerollSelectedEffect(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Convert|Reroll")
	bool CanRerollSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Convert|Reroll")
	void ResetRerollStates();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
	class USoundBase* PrepareBGM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGM")
	class USoundBase* ResultBGM;

	/*UPROPERTY(ReplicatedUsing = OnRep_CombatResultPlayerStates)
	TArray<TObjectPtr<class AOZPlayerState>> roundCombatPlayerState;*/

	/*UFUNCTION()
	void OnRep_CombatResultPlayerStates();*/

	UPROPERTY(Replicated)
	TArray<TObjectPtr<class AOZPlayerState>> roundCombatPlayerState;

	// ===== Fog Rep Snapshot =====
	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	EOZFogState RepFogState = EOZFogState::Idle;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	int32 RepFogPhaseIndex = 1;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	FVector RepFogCenter = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	float RepFogSafeRadiusCm = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	float RepFogInitialMapRadiusCm = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	FVector RepFogPrevCenter = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	float RepFogPrevRadiusCm = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	FVector RepFogNextCenter = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_FogSnapshot)
	float RepFogNextRadiusCm = 0.f;

	UFUNCTION()
	void OnRep_FogSnapshot();

	UFUNCTION()
	void UpdateFogVisual_Local();

public:
	UPROPERTY(Replicated)
	ECombatBonus roundCombatBonus;

	UPROPERTY(Replicated)
	TArray<FString> bonusePlayerNames;

	UPROPERTY(ReplicatedUsing = OnRep_CombatResultSetted)
	bool bIsCombatResultSetted = false;

	UFUNCTION()
	void OnRep_CombatResultSetted();

public:
	//WorldBound For Minimpa Nomalize
	void SetWorldBounds(const FVector& Center, const FVector& Extent);

	UPROPERTY(BlueprintReadOnly, Replicated)
	class AOZShopManager* ShopManager;

	UFUNCTION(BlueprintPure, Category = "Shop")
	class AOZShopManager* GetShopManager() const { return ShopManager; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|PrepareState")
	FName PrepareStateActorTag = FName("PrepareActor");

	void InitializePrepareStateActors();

	void SpawnPrepareStateActors();
	void DestroyPrepareStateActors();

	UFUNCTION(Reliable, NetMulticast)
	void DeActivatePlayerWidgetComponent();

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<AActor>> SpawnedPrepareActors;

	UPROPERTY(EditDefaultsOnly, Category = "SFX|Round")
	USoundBase* CombatStartSfx;

	UPROPERTY(EditDefaultsOnly, Category = "SFX|Round")
	USoundBase* PrepFiveSecSfx;

	FTimerHandle PrepCountdownSfxTimer;
	float PrevPrepRemainTime = 9999.f;
	bool bPlayedPrep5SecSfx = false;

	double GetPrepareRemainingTime_Local() const;

	void TickPrepCountdownSfx();

private:
	// 스폰 정보 (클래스 + 위치)를 기록
	struct FPrepareActorSpawnInfo
	{
		TSubclassOf<AActor> ActorClass;
		FTransform SpawnTransform;
		FVector SpawnScale = FVector::OneVector;
	};

	TArray<FPrepareActorSpawnInfo> PrepareActorSpawnInfos;

private:
	UPROPERTY(ReplicatedUsing = On_worldBoundRep)
	FOZWorldBoundInfo worldBound;

	UFUNCTION()
	void On_worldBoundRep();

	UPROPERTY(Transient)
	TObjectPtr<class AOZFogVisualActor> CachedFogVisual = nullptr;

	class AOZFogVisualActor* GetFogVisualActor();

public:
	// Combat State 진입 시 호출되는 델리게이트 (블루프린트 바인딩 가능)
	UPROPERTY(BlueprintAssignable, Category = "GameState|Events")
	FOnCombatStateEntered OnCombatStateEntered;

	// 클라이언트들이 동시에 컷신을 틀게 만드는 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayOpeningCutscene();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CommonTutorialMsg(int32 index);

private:
	void EnterPrepareState();
	void EnterComBatState();
	void EnterResultState();
	void EnterGameEndState();

	void CheckCombatResultfullReplicated();

	APlayerController* GetLocallPlayercontroller();
};
