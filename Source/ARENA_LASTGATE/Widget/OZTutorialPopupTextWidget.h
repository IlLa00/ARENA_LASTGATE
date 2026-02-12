// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZTutorialPopupTextWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupAnimationEnd);

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZTutorialPopupTextWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

public:

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UTextBlock> Text_Title;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UTextBlock> Text_MSG;

	UFUNCTION(BlueprintCallable)
	void BroadCastOnTutorialTextEnd();

	FOnPopupAnimationEnd OnPopupAnimationEnd;

	void PopupTutorialText(float lifeTime);

	UFUNCTION(BlueprintCallable)
	void CallPopupTutorialText(float lifeTime);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayTutorialText(float lifeTIme);

	/*UFUNCTION(BlueprintCallable)
	void CallSetTutorialText(FString titleText, FString Msg);*/

	void SetText(FString titleText, FString Msg, bool bIsMinimapOpened);

private:
	FVector2D originPos = FVector2D(0.f, -396.f);

};
