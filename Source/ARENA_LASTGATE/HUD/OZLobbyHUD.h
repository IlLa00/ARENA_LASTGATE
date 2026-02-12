// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OZHud.h"
#include "OZLobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API AOZLobbyHUD : public AOZHud
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category = "OZ|BPUpdateLobbyUI")
	void RefreshLobbyUI();

	void UpdateUI() override;

	UFUNCTION(BlueprintCallable)
	void SetLobbyUserNames(TArray<FString> UserNames);
	
};
