// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZUI.h"
#include "OZLobbyUI.generated.h"

UCLASS()
class ARENA_LASTGATE_API UOZLobbyUI : public UOZUI
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

public:

	TArray<class UOZLobbyUserInfoWidget*> UserInfoWidgets;
	void SetUserInfo(TArray<FString> UserNames);

	/*UFUNCTION()
	void OnDelayedInitialize();*/

protected:
	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo1;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo2;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo3;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo4;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo5;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo6;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo7;

	UPROPERTY(BlueprintReadOnly, Category = "OZ|UserInfo", meta = (BindWidget))
	TObjectPtr<class UOZLobbyUserInfoWidget> UserInfo8;

};
