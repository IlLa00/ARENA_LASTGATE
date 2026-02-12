// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLobbyWaitRoomUserInfoBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLobbyWaitRoomUserInfoBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Image_Wait;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Image_HasPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Image_Ready;

	UFUNCTION(BlueprintCallable)
	void SetUserInfoWait();

	UFUNCTION(BlueprintCallable)
	void SetUserInfoHasPlayer(FString userName);

	UFUNCTION(BlueprintCallable)
	void SetUserInfoReady(FString userName);

	bool ToggleReadyState();

	UPROPERTY(BlueprintReadWrite)
	bool bIsReadyState = false;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UImage> Image_UserInfoBarBackGround;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_PlayerName;


	
};
