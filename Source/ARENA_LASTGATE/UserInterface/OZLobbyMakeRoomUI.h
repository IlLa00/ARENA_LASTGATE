// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLobbyMakeRoomUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLobbyMakeRoomUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AOZLobbySelectHUD> lobbyHud = nullptr;
	
};
