// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "Utils/Util.h"
#include "TimerManager.h"
#include "GameplayEffectTypes.h"
#include "Object/SlotMachineReward.h"
#include "OZPlayerState.generated.h"

class UAbilitySystemComponent;
struct FOZConvertData;
enum class EConvertGrade : uint8;

// Convert 획득 시 브로드캐스트되는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConvertAcquired, int32, ConvertID, EConvertGrade, Grade);
class UOZPlayerAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class AOZPlayer;

struct FStatusRatioData;

UCLASS()
class ARENA_LASTGATE_API AOZPlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AOZPlayerState();

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    TSet<AActor*> CurrentViewObjects;

    void EraserCurrentViewObjects() { CurrentViewObjects.Empty(); }

protected:
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(EditDefaultsOnly, Category = "GAS|Effect")
    TSubclassOf<UGameplayEffect> GE_BlockFire;

public:
    
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> ASC;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<class UOZInventoryComponent> InventoryComp;

    UPROPERTY()
    TObjectPtr<UOZPlayerAttributeSet> PlayerAttributes;

    UPROPERTY()
    TObjectPtr<class UOZWeaponAttributeSet> WeaponAttributeSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> FireAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> OverheatHandleAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> CoolingAbilityClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<class UGameplayAbility> SprintAbilityClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<class UGameplayAbility> DashAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayAbility> AimingtAbilityClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<class UGameplayAbility> StaminaAbilityClass;
    void GiveDefaultAbilities();

    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayEffect> ShieldRegenGE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
    TSubclassOf<UGameplayEffect> HitMoveSpeedGE;

    //UserInfo
    UPROPERTY(ReplicatedUsing = "OnRep_UserName", BlueprintReadOnly)
    FString UserName;

    UPROPERTY(ReplicatedUsing = "OnRep_UserIdentificationCode", BlueprintReadOnly)
    FString UserIdentificationCode;

    UFUNCTION()
    void OnRep_UserName();

    UFUNCTION()
    void OnRep_UserIdentificationCode();

    UFUNCTION(Server, Unreliable)
    void Server_ReportStateUpdate();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_BroadcastUIUpdate(const TArray<FString>& UserNames);

    UFUNCTION(BlueprintCallable, Category = "GAS|Heat")
    float GetHeatRatio() const;

    UFUNCTION(BlueprintCallable, Category = "GAS|Health")
    FStatusRatioData GetHeatlInfo() const;
    FStatusRatioData GetSteminaInfo() const;
    FStatusRatioData GetSheildInfo() const;

    UPROPERTY(Replicated)
    uint8 Round_SurvivalRanking = 1;

    UPROPERTY(Replicated)
    uint8 Round_KillCount = 0;

    UPROPERTY(Replicated)
    uint8 Total_KillCount = 0;

    UPROPERTY(Replicated)
    float Round_DamageAmount = 0;
    UPROPERTY(Replicated)
    float Round_DamagedAmount = 0;
    UPROPERTY(Replicated)
    float Total_DamageAmount = 0;
    UPROPERTY(Replicated)
    float Total_DamagedAmount = 0;

    UPROPERTY(Replicated)
    ECombatBonus RoundCombatBonus = ECombatBonus::Max;

    UPROPERTY(Replicated)
    uint8 OwningCores = 0;

    UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite)
    int32 OwningScraps = 5000;

    TObjectPtr<AOZPlayerState> LastDamageInstigatorState = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "GAS")
    TSubclassOf<UGameplayEffect> StaminaGE;

    FTimerHandle RecentlyDamagedTimerHandle;
    FTimerHandle HitMoveSpeedTimerHandle;
    FActiveGameplayEffectHandle ActiveHitMoveSpeedHandle;

    UFUNCTION()
    void HandleRecentlyDamagedExpired();

    UFUNCTION()
    void HandleHitMoveSpeedExpired();

    void NotifyDamaged();

    void UpdateClientMiniMapUI(TSet<AActor*> AddedActor, TSet<AActor*> RemovedActor);

    //LobbyWaitState

    UFUNCTION(Server, Reliable)
    void Server_CallUserLobbyReadyState(ELobbyUserState userState);

    UPROPERTY(ReplicatedUsing = OnRep_LoobyReadyState)
    ELobbyUserState userLobbyReadyState = ELobbyUserState::None;

    UFUNCTION()
    void OnRep_LoobyReadyState();

    void SetAttributes();

    void SetRespawnAttributes();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Reward")
    void Server_AddCores(uint8 Amount);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Reward")
    void Server_AddScraps(int32 Amount);

    UPROPERTY(Replicated)
    FOZSlotRewardResult PendingSlotReward;

    UFUNCTION(BlueprintCallable)
    bool HasPendingSlotReward() const;

    UFUNCTION()
    void ClearPendingSlotReward();

    UPROPERTY(ReplicatedUsing = OnRep_InBush)
    bool bInBush = false;

    UFUNCTION()
    void OnRep_InBush();

    UFUNCTION(BlueprintCallable, Category = "Bush")
    bool IsInBush() const { return bInBush; }

    void SetInBush_Server(bool bNewInBush);

    UFUNCTION()
    void OnRep_CurrentBushID();

    UPROPERTY(ReplicatedUsing = OnRep_CurrentBushID, BlueprintReadOnly, Category = "Bush")
    int32 CurrentBushID = INDEX_NONE;

    UFUNCTION(BlueprintCallable, Category = "Bush")
    void SetCurrentBushID(int32 NewBushID);

    UFUNCTION(BlueprintCallable, Category = "Vision")
    int32 GetCurrentBushID() const { return CurrentBushID; }

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Convert")
    TArray<int32> AcquiredUniqueConvertHistory;

    UPROPERTY(ReplicatedUsing = OnRep_AcquiredConvertIDs, BlueprintReadOnly, Category = "Convert")
    TArray<int32> AcquiredConvertIDs;

    // Convert 획득 시 브로드캐스트 
    UPROPERTY(BlueprintAssignable, Category = "Convert")
    FOnConvertAcquired OnConvertAcquired;

    UFUNCTION()
    void OnRep_AcquiredConvertIDs();

    UFUNCTION(BlueprintCallable, Category = "Convert")
    void AddToConvertHistory(int32 ConvertID, EConvertGrade Grade);

    UFUNCTION(BlueprintCallable, Category = "Convert")
    void NotifyConvertAcquired(int32 ConvertID, EConvertGrade Grade);

    // 해당 플레이어의 유니크/레전더리 이력 가져오기
    UFUNCTION(BlueprintPure, Category = "Convert")
    const TArray<int32>& GetUniqueConvertHistory() const { return AcquiredUniqueConvertHistory; }

    UFUNCTION(BlueprintPure, Category = "Convert")
    const TArray<int32>& GetAcquiredConvertIDs() const { return AcquiredConvertIDs; }

    UFUNCTION(BlueprintCallable, Category = "Convert")
    void ClearConvertHistory() { AcquiredUniqueConvertHistory.Empty(); AcquiredConvertIDs.Empty(); }

private:
    float GetShieldRegenDelaySeconds() const;

    FActiveGameplayEffectHandle BlockFireHandle;

    // SetAttributes()가 이미 호출되었는지 (리스폰 시 중복 호출 방지)
    bool bAttributesInitialized = false;
};