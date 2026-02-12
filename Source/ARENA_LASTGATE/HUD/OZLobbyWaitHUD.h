// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/OZHud.h"
#include "OZLobbyWaitHUD.generated.h"

/**
 * 
 */

class AOZLobbyWaitGameState;

UCLASS()
class ARENA_LASTGATE_API AOZLobbyWaitHUD : public AOZHud
{
	GENERATED_BODY()

public:
	/*void UpdateUI() override;

	UFUNCTION(BlueprintCallable)
	void SetLobbyWaitUserNames(TArray<FString> UserNames);*/

	void BindGameState(TObjectPtr<class AOZLobbyWaitGameState> gameState);

	void RefreshFromGameState();
	void RefreshLobbyChatting();

	UFUNCTION(BlueprintCallable)
	AOZLobbyWaitGameState* GetOwnedGameState() { return OwnedGameState; }

	void OnKeyboardEnterPressed();
	void OnKeyboardESCPressed();

private:
	TObjectPtr<class AOZLobbyWaitGameState> OwnedGameState = nullptr;
	TObjectPtr<class UOZLobbyWaitRoomUI> lobbywaitRoomUI = nullptr;

	int AssingedLeastChattIndex = -1;
	
};
