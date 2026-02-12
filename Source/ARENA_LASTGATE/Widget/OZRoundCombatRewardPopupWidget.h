// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZRoundCombatRewardPopupWidget.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnPopupAnimEnd);

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZRoundCombatRewardPopupWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;
	
public:

	FOnPopupAnimEnd OnPopupAnimEnd;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAnim();

	UFUNCTION(BlueprintCallable)
	void CallPalyPopup();

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZRoundComBatResultUI> CallerRoundComBatResultUI = nullptr;
};
