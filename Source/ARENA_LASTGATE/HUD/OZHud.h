// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OZHud.generated.h"

UCLASS()
class ARENA_LASTGATE_API AOZHud : public AHUD
{
	GENERATED_BODY()
	
public:

    UPROPERTY()
    TArray<FString> CachedUserNames;

    void SetUserNames(const TArray<FString>& UserNames)
    {
        CachedUserNames = UserNames;
    }

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<class AOZPlayerController> OZPlayer = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<class UOZUI> BindingUI = nullptr;

    UFUNCTION(BlueprintCallable)
    void ListenUIConstructed();

    UFUNCTION()
    void PostUIInit();

    void SetInputModeUIOnly(TObjectPtr<class AOZPlayerController> controller, TObjectPtr<UUserWidget> focusWidget, bool bCursorActive);
    void SetInputModeUIANDGame(TObjectPtr<class AOZPlayerController> controller, bool bCursorActive);
    void SetInputModeGameOnly(TObjectPtr<class AOZPlayerController> controller, bool bCursorActive);

	virtual void UpdateUI() {};
};
