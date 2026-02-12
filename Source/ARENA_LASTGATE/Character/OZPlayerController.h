#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "EnhancedInputLibrary.h"
#include "Utils/Util.h"
#include "Object/OZWorldBoundCacher.h"
#include "Net/UnrealNetwork.h"
#include "OZPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentStaminaChanged, float, staminaratio);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxStaminaChanged, float, maxStamina);

class UInputMappingContext;
class UInputAction;
class AOZPlayer;
class AOZTower;
class UAbilitySystemComponent;
class UAudioComponent;
enum class EOZItemType : uint8;

USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Sender;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	uint8 ChatIndex;
};

USTRUCT(BlueprintType)
struct FClientControllerReadyState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	bool bUIConstructed = false;

	UPROPERTY(BlueprintReadWrite)
	bool bPlayerStateReplicated = false;

	UPROPERTY(BlueprintReadWrite)
	bool bBeginPlayCalled = false;

	bool IsReady() const
	{
		return bUIConstructed && bPlayerStateReplicated && bBeginPlayCalled;
	}
};


UCLASS()
class ARENA_LASTGATE_API AOZPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AOZPlayerController();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float Deltatime) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_PlayerState() override;

	void OnUINativeConstructClear();
	bool bUIReady = false;

	bool bEdgeAimActive = false;

	UPROPERTY(EditAnywhere, Category = "Camera|EdgeAim")
	float EdgeBorderThickness = 50.f;

	void SetOzInputMode(EInputMode inputMode) { controllerInputMode = inputMode; }

	UFUNCTION(Server, Reliable)
	void Server_ReportcombatResultRecived();

	void InteractionStarted();
	void InteractionCompleted();
	void UpdateHoldInteraction(float DeltaTime);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Shop")
	void Server_PurchaseItem(int32 ItemID, EOZItemType ItemType, int32 Quantity = 1);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Shop")
	void Server_SellItem(int32 ItemID, EOZItemType ItemType, int32 Quantity = 1);

	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void BP_OnLocalPlayerDead();

	void InitializeInGameMinimap(FOZWorldBoundInfo worldBound);

	UFUNCTION(Client, Reliable)
	void ShowKillLog(const FString& KillerName, const FString& DeadName, int numSurvivalPlayers);

	UFUNCTION(Client, Reliable)
	void StartDarkSmogeShrink(float shrinkingTime);

	UFUNCTION(Client, Reliable)
	void ShowCombatResultPopup(bool bIsWin);

	UPROPERTY(BlueprintReadWrite)
	int assingedWaitRoomIndex = -1;

	UFUNCTION(BlueprintCallable)
	int GetIsPlayerReadyState();

	void ChangeLobbyWaitState(TObjectPtr<class UOZLobbyWaitRoomUI> lobbyWaitRoomUI);

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Aim")
	void SendBattleItemThrowDistanceToServer(float MaxRangeCm, float FallbackDistanceCm = 5000.f);

	UFUNCTION(Server, Reliable)
	void Server_SetBattleItemThrowDistance(float InDistanceCm);

	UFUNCTION(BlueprintCallable, Category = "BattleItem|Aim")
	float GetLastBattleItemThrowDistanceCm() const { return LastBattleItemThrowDistanceCm; }

	UFUNCTION(Server, Reliable)
	void ReportRoundCombatEnd();

	UFUNCTION(Server, Reliable)
	void ReportSessionExit(AOZPlayerController* exitPlayercontroller);

	UFUNCTION(Client, Reliable)
	void Client_ForceCloseSlotUIAndReturnCamera();

	UPROPERTY(EditDefaultsOnly, Category = "Fog|SFX")
	USoundBase* FogShrinkStartSfx = nullptr;

	// 타워 상호작용 시작 시 재생되는 사운드 (로컬 플레이어에게만)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|SFX")
	USoundBase* TowerInteractionStartSound = nullptr;

	UPROPERTY()
	UAudioComponent* TowerInteractionAudioComp = nullptr;

	FTimerHandle TowerInteractionSoundTimerHandle;

	FTimerHandle FogSfxRepeatTimer;
	int32 FogSfxRepeatCount = 0;

	void PlayFogShrinkStartSfxOnce();
	void PlayFogShrinkStartSfxRepeated();


	void ReleaseAbilityInput(int32 InputID);


	UFUNCTION(Client, Reliable)
	void OnRoundStart();

	UFUNCTION(Server, Reliable)
	void Server_ActivateTower(class AOZTower* Tower);

	UFUNCTION(Server, Reliable)
	void Server_ReportCombatBonusRewardPopupEnd();


	//Chatting
	UFUNCTION(BlueprintCallable)
	void Call_SendChattingMsg(const FString& Message, bool bIsServerMSG = false);

	UFUNCTION(Server, Reliable)
	void Server_SendChat(FChatMessage ChattingMSGInfo);

	//등대
	UFUNCTION(Client, Reliable)
	void OnTowerActivationChanged(bool bIsActivated);

	UPROPERTY(BlueprintReadWrite, Category = "OZ|Input")
	bool bBlockFireByUI = false;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AimingAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> OpenShopAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Interaction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> UseItem_1_Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> UseItem_2_Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> UseItem_3_Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> UseItem_4_Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> KeyboardTabAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> KeyboardEnterAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> KeyboardESCAction;

	void Move(const FInputActionValue& Value);

	void FireStarted(const FInputActionValue& Value);

	void FireEnded(const FInputActionValue& Value);

	void DashStarted(const FInputActionValue& Value);

	void ShopOpenPressedAndUp(const FInputActionValue& Value);

	void OnAimStarted(const FInputActionInstance& Instance);
	void OnAimCompleted(const FInputActionInstance& Instance);

	void AimStartedInternal();
	void AimCompletedInternal();

	void Use_1_Item();
	void Use_2_Item();
	void Use_3_Item();
	void Use_4_Item();

	void ToggleMinimap(const FInputActionValue& Value);

	void PressEnter(const FInputActionValue& Value);
	void PressESC(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_UseItemAtSlot(int32 SlotIndex);

	void UpdateMouseEdgeAim();

	FClientControllerReadyState ClientReadyState;

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerInfo(const FString& name, const FString& ID);

	void CheckClientReady();

	UFUNCTION(Server, Reliable)
	void Server_ReportClientReady();

	void OnAbilityPressed(const FInputActionValue& Value, int32 InputID);
	void OnAbilityReleased(const FInputActionValue& Value, int32 InputID);

	//LobbyWait Function
	UFUNCTION(Server, Reliable)
	void Server_RequestEnroll(AOZPlayerState* NewPlayerState);

	UFUNCTION(Server, Reliable)
	void Server_RequestChangePlayerState(int playerIndex, bool bIsReadyState);

private:
	EInputMode controllerInputMode = EInputMode::GameAndUI;

	bool bIsHoldingFire = false;

	// 홀드 상호작용 시스템
	bool bIsHoldingInteraction = false;
	float InteractionHoldTime = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRequiredTime = 2.0f;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentInteractionTarget;

	FTimerHandle AutoFireTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Fire")
	float AutoFirePollInterval = 0.05f;

	void TryEnrollplayerStateAtLobbyWait();

	void AutoFireTick();

	UPROPERTY(Transient)
	float LastBattleItemThrowDistanceCm = 0.f;

	UPROPERTY(Transient)
	float LastBattleItemThrowTimeSec = 0.f;

	// 튜토리얼 팝업용 - 처음 타워 활성화 추적
	bool bFirstTowerActivationTriggered = false;

	// 튜토리얼 팝업용 - 처음 상점 열기 추적
	bool bFirstShopOpenTriggered = false;

	void SendPlayerEnrollMsg();

	UFUNCTION(Server, Reliable)
	void Server_UpdateCharacterHealthWidget();

public:
	UPROPERTY()
	FOnCurrentStaminaChanged OnCurrentStaminaChanged;

	UFUNCTION(Client, Reliable)
	void Client_PlayOpeningCutscene();

	UFUNCTION(Client, Reliable)
	void Client_RecievedCommonTutorialPopupCall(int index);

	UFUNCTION(Server, Reliable)
	void Server_ReportCutsceneFinished();

	UFUNCTION()
	void OnOpeningCutsceneFinished_Local();

	UFUNCTION(Client, Reliable)
	void Client_PlayMineInstallSound(USoundBase* Sound, FVector Location);
};
