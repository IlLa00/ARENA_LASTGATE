// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OZLobbyJoinRoomInfoWidget.generated.h"

/**
 * 
 */



USTRUCT(BlueprintType)
struct FOZSessionExtraSettinInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString RoomOwner;

	UPROPERTY(BlueprintReadWrite)
	FString RoomName;

	UPROPERTY(BlueprintReadWrite)
	bool bIsPrivateRoom;

	UPROPERTY(BlueprintReadWrite)
	FString RoomPassword;

	UPROPERTY(BlueprintReadWrite)
	int RoomTurn;

	UPROPERTY(BlueprintReadWrite)
	FString RoomGameMode;

	UPROPERTY(BlueprintReadWrite)
	float RoomScrapRatio;

	UPROPERTY(BlueprintReadWrite)
	float RoomDarkSmogeSpeedRatio;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionClicked, FOZSessionExtraSettinInfo, ExtraSettingInfo, int, widgetIndex);


UCLASS()
class ARENA_LASTGATE_API UOZLobbyJoinRoomInfoWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_RoomName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Image_SecurityImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RoomName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UBorder> Border_RoomPlayers;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RoomPlayers;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UBorder> Border_RoomState;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_RoomState;

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Image_Highlight = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Image_Normal = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon_Lock = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon_UnLock = nullptr;

public:

	UPROPERTY(BlueprintReadWrite)
	bool bIsInitializedSucceed = false;

	UPROPERTY(BlueprintReadWrite)
	FOZSessionExtraSettinInfo ExtraSettinInfo;

	UFUNCTION(BlueprintCallable)
	void HightLightInfoBar();

	UFUNCTION(BlueprintCallable)
	void NormalizeInfoBar();

	UFUNCTION(BlueprintCallable)
	void SetRoomInfo(FString roomState, int currPlayers);

public:
	UPROPERTY(BlueprintReadWrite)
	int32 assingedIndex = -1;

	UPROPERTY(BlueprintAssignable)
	FOnSessionClicked OnSessionClicked;

	UFUNCTION(BlueprintCallable)
	void BroadCastSessionClicekd();
};
