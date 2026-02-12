// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/OZTutoMessageData.h"
#include "OZInGameTutorialPopupSubSystem.generated.h"

/**
 * 
 */

USTRUCT()
struct FTutorialPopupRequest
{
	GENERATED_BODY()

	int32 MsgID;
	float LifeTime;
};

UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZInGameTutorialPopupSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(Config, EditDefaultsOnly, Category = "Fog Data")
	TSoftObjectPtr<UDataTable> TutorialMsgTableRef;

	UPROPERTY()
	TObjectPtr<UDataTable> TutorialMsgTable;

	FOZTutoMessageData* GetTutorialMsgData(int MsgID);

	UFUNCTION()
	void OnTutorialPopupEnd();

public:

	void BindPopupEndEvent();
	
	UFUNCTION(BlueprintCallable)
	void CallTutorialPopupByID(int MsgID, float LifeTime);

	TObjectPtr<class UOZInGameFloorUI> inGameFloorUI = nullptr;

private:
	TQueue<FTutorialPopupRequest> PopupRequestQueue;

	bool bIsPopupPlaying = false;

	void TryPlayNextPopup();
	
};
