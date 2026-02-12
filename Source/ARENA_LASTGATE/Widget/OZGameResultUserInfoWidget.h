// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZGameResultUserInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZGameResultUserInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UTextBlock> Text_Excape;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UImage> Image_Excape;*/

	/*UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_UserName;*/

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_TotalNumcores;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_TotalNumKills;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_TotalNumAtack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_TotalNumAttacked;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Attack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Attacked;

public:
	void SetDefaultState();
	void SetResult(bool bIsExcape, class AOZPlayerState* playerState, int gameMaxAttack, int gameMaxAttacked);

	UFUNCTION(BlueprintImplementableEvent)
	void DisplayGameResult(bool bIsWin, const FString& userName);

	UFUNCTION(BlueprintImplementableEvent)
	void HighlithAtkPercentagebar();

	UFUNCTION(BlueprintImplementableEvent)
	void HighlithHitPercentagebar();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Inage_Win;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Inage_Lose;

};
