// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZGameEndUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZGameEndUI : public UOZUI
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZGameResultUserInfoWidget> GameResultWidget1;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZGameResultUserInfoWidget> GameResultWidget2;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZGameResultUserInfoWidget> GameResultWidget3;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZGameResultUserInfoWidget> GameResultWidget4;

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<class UOZGameResultUserInfoWidget>> GameResultWidgetArray;
	
public:
	void SetGameResult(TArray<TObjectPtr<class AOZPlayerState>>& ozPlayerStates);
};
