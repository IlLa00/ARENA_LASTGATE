// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUD/OZHud.h"
#include "Utils/Util.h"
#include "OZLobbySelectHUD.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbySelectHUD : public AOZHud
{
	GENERATED_BODY()

public:

	/*UFUNCTION(BlueprintImplementableEvent, Category = "OZ|BPUpdateLobbySelectUI")
	void RefreshLobbyUI();*/

	UFUNCTION(BlueprintImplementableEvent)
	void CallOpenSelectionUI();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|LobbySelectUIArray")
	TArray<TObjectPtr<UUserWidget>> LobbySelectUIArray;

	void UpdateUI() override;

	UFUNCTION(BlueprintCallable)
	void SetLobbySelectUserNames(TArray<FString> UserNames);

	UFUNCTION(BlueprintCallable)
	void SetLobbySelectUI(ELobbyUIType UiType, bool active);

	//MakeRoom Func
	UFUNCTION(BlueprintCallable)
	void OnMakeRoomButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnMakeRoomCloseButtonClicked();

	//Join Room Func
	UFUNCTION(BlueprintCallable)
	void OnJoinRoomButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnJoinRoomCloseButtonClicked();

	UFUNCTION(BlueprintCallable)
	void OnJoinRoomCreateButtonClicked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLobbyUIShow(ELobbyUIType openedLobbyType);
	
};
