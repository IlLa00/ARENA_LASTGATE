// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UserInterface/OZUI.h"
#include "OZLobbyWaitRoomUI.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZLobbyWaitRoomUI : public UOZUI
{
	GENERATED_BODY()

public:

	void NativeConstruct() override;

	void OnDelayedInitialize();

	int ChangeUserInfoReadyState(int index);

	UFUNCTION(BlueprintCallable)
	int GetPlayerReadyState();

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo")
	TArray<TObjectPtr<class UOZLobbyWaitRoomUserInfoBarWidget>> UserNameWidgets;

protected:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<UUserWidget> WBP_PopupWarningWidget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSubclassOf<UUserWidget> WBP_PopupSelectionWidget;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UTexture2D> Image_ButtonActive;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TObjectPtr<UTexture2D> Image_ButtonDeActive;

protected:

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyWaitRoomUserInfoBarWidget> UserNameWidget_1;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyWaitRoomUserInfoBarWidget> UserNameWidget_2;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyWaitRoomUserInfoBarWidget> UserNameWidget_3;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyWaitRoomUserInfoBarWidget> UserNameWidget_4;

	UFUNCTION(BlueprintCallable)
	void OnExitLobbyWait();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "OZ|CheckAllPlayerReady")
	bool bCheckPlayerAllReady = false;


public:
	UFUNCTION(BlueprintCallable)
	void ToggleReadyState();

	UFUNCTION(BlueprintCallable)
	bool CheckCanMoveToInGame();

	UFUNCTION(BlueprintImplementableEvent)
	void ActiveOwnersGameStartBtn();

	UFUNCTION(BlueprintImplementableEvent)
	void DeActiveOwnersGameStartBtn();

	UFUNCTION(BlueprintCallable)
	FString GetInGameLevelPath();
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateChatting(int leastIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void OnPressedEnter();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPressedESC();
};
