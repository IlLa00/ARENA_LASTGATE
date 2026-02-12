// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZCombatResultPopupWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCombatResultDelegate, bool, bIsWin);

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZCombatResultPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> Image_combatResult;

	UPROPERTY(BlueprintAssignable)
	FCombatResultDelegate CombatResultDelegate;

	void ShowCombatResult(bool bIsWin);

	UFUNCTION(BlueprintImplementableEvent)
	void CallShowCombatResult(bool bIsWin);

	UFUNCTION(BlueprintCallable)
	void BroadCastCombatResult(bool bIsWin);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ")
	TObjectPtr<UTexture2D> CombatWinImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OZ")
	TObjectPtr<UTexture2D> CombatDefeatImage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "OZ")
	float VictoryAnimDurationTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "OZ")
	float DefeatAnimDurationTime = 3.0f;

public:
	UFUNCTION(BlueprintCallable)
	void PlayResultSound(bool bIsWin);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX")
	USoundBase* WinSfx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX")
	USoundBase* LoseSfx;

};
