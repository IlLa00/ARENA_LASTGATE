// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OZGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARENA_LASTGATE_API UOZGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|UserName")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|CheckRegisted")
	bool bIsLoggedIn = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OZ|AccountTotalWinCount")
	int32 TotalWinCount = 0;

	UPROPERTY(BlueprintReadOnly)
	class UOZOnlineSessionSubsystem* SessionSubsystem;

	UPROPERTY(BlueprintReadOnly)
	class UOZItemSubsystem* ItemSubsystem;

	UPROPERTY(BlueprintReadOnly)
	class UOZModuleSubsystem* ModuleSubsystem;

	UPROPERTY(BlueprintReadOnly)
	class UOZConvertSubsystem* ConvertSubsystem;

	void PrintLog(const FString& LogText, FColor textColor, float time);

//public:
//	void Client_ExitSession();
//
//	void Client_ExitSessionWithController(TObjectPtr<class AOZPlayerController> ozController);
//
//	void Server_ExitSession();
//
//private:
//
//	void EraseSession();
//
//	void GoToLobby();
//
//	//Session_Client
//	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
//
//	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
//
//	UFUNCTION()
//	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

#if WITH_EDITOR
	// 블루프린트에서 호출 가능하도록 노출
	UFUNCTION(BlueprintCallable, Category = "Editor Scripting")
	static void UpdateAndBuildMesh(UStaticMesh* Mesh);
#endif
};
