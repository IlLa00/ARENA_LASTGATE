// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interface/OZIMinimapVisibleActor.h"
#include "Components/OZCameraAimComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GameplayEffectTypes.h"
#include "Character/Components/OZPlayerSoundComponent.h"
#include "OZPlayer.generated.h"

UENUM(BlueprintType)
enum class EPlayerEffectType : uint8
{
	None = 0,
	Fog,
	Fire,
};

class UAbilitySystemComponent;
class AOZPlayerState;
class UGameplayAbility;
class UOZVisionComponent;
class UOZWeaponComponent;
class UOZPlayerAttributeSet;
struct FOnAttributeChangeData;
class AOZBullet;
class UGameplayEffect;
class USkeletalMeshComponent;
class UOZWidgetComponent;
class UOZPlayerSoundComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class ARENA_LASTGATE_API AOZPlayer : public ACharacter, public IAbilitySystemInterface, public IOZIMinimapVisibleActor
{
	GENERATED_BODY()

public:
	AOZPlayer();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

public:	
	virtual void Tick(float DeltaTime) override;
	void InitASCAndAttributes();
	void InitStaminaAttributesForUI();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void OnCharacterHPChanged(float currHP);

	/*UFUNCTION(NetMulticast, Reliable)
	void OnCharacterMaxHPChanged(float currHP);*/

	/*UFUNCTION(NetMulticast, Reliable)
	void OnCharacterMaxSheildChanged(float maxSheild, float maxHP);*/

	UFUNCTION(NetMulticast, Reliable)
	void BroadCastUpdateCharacterSheildHPRatio(float maxSheild, float maxHP);

	UFUNCTION(NetMulticast, Reliable)
	void OnCharacterSheildChanged(float currSheildRatio);

	UFUNCTION(NetMulticast, Reliable)
	void OnWeaponHeatChanged(float heatRatio);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
	UOZWidgetComponent* StatusWidgetComp;

	UFUNCTION(BlueprintCallable, Category = "Component")
	void SetHideUI(bool bStealth);

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Component")
	UOZVisionComponent* VisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UOZCameraAimComponent* CameraAimComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	class USphereComponent* InteractionRangeSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	int32 InteractableOutlineStencilValue = 252;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Damage")
	TSubclassOf<UGameplayEffect> PuppetDamageEffectClass;

	UPROPERTY()
	class UOZPlayerAttributeSet* PAS;

	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);
	void OnWalkSpeedChanged(const FOnAttributeChangeData& Data);
	void OnAimingDistanceChanged(const FOnAttributeChangeData& Data);
	void OnCameraDepthChanged(const FOnAttributeChangeData& Data);
	void OnViewingAngleChanged(const FOnAttributeChangeData& Data);

	void OnCurrentStaminaChanged(const FOnAttributeChangeData& Data);
	//void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);

	void HandleProjectileHit(AActor* HitActor, const FHitResult& Hit, AOZBullet* Bullet);

	UPROPERTY(BlueprintReadOnly, Category = "Dash")
	FVector2D LastMoveInput;
	UFUNCTION(BlueprintCallable, Category = "Dash")
	FVector2D GetLastMoveInput() const { return LastMoveInput; }

	FActiveGameplayEffectHandle WalkSpeedGEHandle;
	FActiveGameplayEffectHandle AimingStaminaGEHandle;

	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetBandanaTexture(UTexture2D* NewBandanaTexture, FLinearColor NewBandanaColor);

	UFUNCTION(BlueprintPure, Category = "Appearance")
	UTexture2D* GetBandanaTexture() const { return BandanaTexture; }

	UFUNCTION(BlueprintPure, Category = "Appearance")
	FLinearColor GetBandanaColor() const { return BandanaColor; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DisableVision(float Duration);

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Appearance")
	TObjectPtr<UTexture2D> BandanaTexture;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Appearance")
	FLinearColor BandanaColor;

	void HandleOutOfHealth();

	void ApplyDamageToASC(
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		float Damage,
		const FHitResult* OptionalHit,
		UObject* OptionalSourceObject
	);

	// Interaction System
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetClosestInteractableActor() const;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TArray<TObjectPtr<AActor>> InteractableActorsInRange;

	UFUNCTION(BlueprintCallable, Category = "SlotMachine")
	void EnterSlotMachineView();

	UFUNCTION(BlueprintCallable, Category = "SlotMachine")
	void ExitSlotMachineView();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sound")
	UOZPlayerSoundComponent* SoundComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TObjectPtr<UNiagaraComponent> ScreenEffectComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TMap<EPlayerEffectType, TObjectPtr<UNiagaraSystem>> EffectSystemMap;

	// Shield Effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effect|Shield")
	TObjectPtr<UNiagaraComponent> ShieldEffectComponent;

	// 실드 이펙트 활성화 상태 (Multicast로 동기화됨)
	UPROPERTY(BlueprintReadOnly, Category = "Effect|Shield")
	bool bShieldEffectActive = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateShieldEffect(bool bHasShield);

	void UpdateShieldEffectLocal(bool bHasShield);

	// 사망 시 모든 나이아가라 이펙트 비활성화
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DeactivateAllNiagaraEffects();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void Client_StartEffect(EPlayerEffectType EffectType);

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void Client_StopEffect();

	// 현재 재생 중인 이펙트 타입
	UPROPERTY(BlueprintReadOnly, Category = "Effect")
	EPlayerEffectType CurrentEffectType = EPlayerEffectType::None;

	UFUNCTION(Client, Unreliable)
	void Client_PlayFogDamageSound();

	UFUNCTION(BlueprintCallable, Client, Unreliable)
	void Client_PlayBattleItemThrowSound(float ThrowDistanceCm);

	UFUNCTION(BlueprintCallable, NetMulticast, Unreliable)
	void Multicast_PlayGunFireSound(const FVector& Location,
		const FOZGunSoundParams& Params);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_PlayFireMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_PlayFireMontage(UAnimMontage* Montage, float PlayRate);

	UFUNCTION()
	void UpdateBushVision();

	// 처음 Puppet 데미지 시 튜토리얼 팝업 호출
	UFUNCTION(Client, Reliable)
	void Client_OnFirstPuppetDamage();

	// 섬광탄 피격 이펙트 재생 (피격된 로컬 플레이어에게만)
	UFUNCTION(Client, Reliable)
	void Client_PlayFlashBangHitEffect(UNiagaraSystem* EffectSystem);

private:
	UFUNCTION()
	void OnInteractionRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool bMoveSpeedDelegateBound = false;
	bool bWalkSpeedDelegateBound = false;
	bool bAimingDistanceDelegateBound = false;
	bool bCameraDepthDelegateBound = false;
	bool bViewingAngleDelegateBound = false;

	// 튜토리얼 팝업용 - 처음 오버랩 추적
	bool bFirstTowerOverlapTriggered = false;
	bool bFirstSlotMachineOverlapTriggered = false;
	bool bFirstPuppetDamageTriggered = false;

protected:
	void Input_Move(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "OZ|Death")
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION(Server, Reliable)
	void Server_ReportPlayerDead();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OZ|Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDeathMontage();
	void MulticastPlayDeathMontage_Implementation();

	void RecordHit(TObjectPtr<APawn> hittedPawn, TObjectPtr<APawn> instigatorPawn, float damage);

	


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|ObjectMinimapIcon")
	TObjectPtr<UTexture2D> MinimapIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ|ObjectMinimapCategory")
	EMinimapActorCategory MinimapActorCategory = EMinimapActorCategory::Character;

	FTimerHandle CombatStateCheckTimer;
	UFUNCTION()
	void ResetCombatState();
	
public:
	virtual UTexture2D* GetMinimapIcon_Implementation() const override
	{
		return MinimapIcon;
	}

	virtual EMinimapActorCategory GetMinimapCategory_Implementation() const override
	{
		return MinimapActorCategory;
	}

	bool GetIsPlayerDead() { return bIsDead; }

	UFUNCTION()
	void SetBushVisibility(bool bVisible);
};
