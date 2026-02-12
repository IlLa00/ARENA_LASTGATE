// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Utils/Util.h"
#include "Character/OZPlayerController.h"
#include "OZLobbyWaitGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnLobbyWaitSlotChanged);
DECLARE_MULTICAST_DELEGATE(FOnLobbyChattingUpdated);

/**
 * 
 */


USTRUCT(BlueprintType)
struct FLobbySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class AOZPlayerState> PlayerState = nullptr;
};

UCLASS()
class ARENA_LASTGATE_API AOZLobbyWaitGameState : public AGameState
{
	GENERATED_BODY()

protected:
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	AOZLobbyWaitGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_SlotStateChanged)
	TArray<ELobbyUserState> SlotState;

	UPROPERTY(ReplicatedUsing = OnRep_LobbySlots, BlueprintReadOnly)
	TArray<FLobbySlot> LobbySlots;

	void RegisterPlayer(class AOZPlayerState* NewPlayerState);

	void UnregisterPlayer(class AOZPlayerState* LeavingPlayerState);

	int GetIsPlayerReady(class AOZPlayerState* TargetPlayerState);

	int GetAssingedIndex(class AOZPlayerState* RequestPlayerState);

	void ChangePlayerReadyState(int playerIndex, bool bIsReadyState);

	UFUNCTION()
	void OnRep_LobbySlots();

	int32 GetMaxPlayerNum() { return maxPlayerNum; }

private:

	const int32 maxPlayerNum = 4;

	

	UFUNCTION()
	void OnRep_SlotStateChanged();

public:
	FOnLobbyWaitSlotChanged OnLobbyWaitSlotChanged;

	//Chat

	FOnLobbyChattingUpdated OnLobbyChattingUpdated;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ChatMessages)
	TArray<FChatMessage> ChatMessages;
	
	UFUNCTION()
	void OnRep_ChatMessages();

	UFUNCTION(BlueprintCallable)
	void AddChatMessage(FChatMessage msg);

	int GetChattIndex() { return chattIndex; }

	UFUNCTION(BlueprintCallable,NetMulticast, Reliable)
	void ShowLoadingScreen();

private:
	UPROPERTY(Replicated)
	int chattIndex = 0;
};
