// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "OZLobbyJoinRoomUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLobbyJoinRoomUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AOZLobbySelectHUD> lobbyHud = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle FindSessionTimerHandle;

	void ShowJoinRoom();
	void LeaveJoinRoom();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateSessionList();
	
};
