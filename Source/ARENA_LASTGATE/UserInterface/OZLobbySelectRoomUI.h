// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLobbySelectRoomUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLobbySelectRoomUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void DisplayUserAccountInfo();

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class AOZLobbySelectHUD> lobbyHud = nullptr;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_UserName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_UserScroe;
};
