// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Character/OZPlayerController.h"
#include "OZOnlineSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSessionCheckedEnd);

/**
 * 
 */
UCLASS()

class ARENA_LASTGATE_API UOZOnlineSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite)
	FString Session_ID;

	UPROPERTY(BlueprintReadWrite)
	FString Session_Unique_BuildID;

	UPROPERTY(BlueprintReadWrite)
	FString joined_SessionID;

	/*UFUNCTION(BlueprintCallable)
	void EraseSession();*/

	UPROPERTY(BlueprintAssignable)
	FOnSessionCheckedEnd OnSessionCheckedEnd;

	UFUNCTION(BlueprintCallable)
	void CheckHasSessionAlready();

private:
	IOnlineSubsystem* OnlineSub;
	IOnlineIdentityPtr Identity;

	FString UserIdentificationCode;
	//TMap<FString , TArray

	FString UserNetUniqueID;

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void TryLogin();

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION(BlueprintCallable)
	void OnSessionCreateSuccess(FString CreateLevelName);

	UFUNCTION(BlueprintCallable)
	void OnSessionJoinSuccess(FString JoinLevelName);

	inline FString GetUserIdentifyCode() { return UserIdentificationCode; }

	UPROPERTY(BlueprintReadWrite)
	int Session_NumTurns = 0;

	UFUNCTION(BlueprintCallable)
	void ExitSession(AOZPlayerController* exitPlayerConerller);

	UPROPERTY(BlueprintReadWrite)
	bool bIsHost = false;


protected:
	//Client

	//Session_Client

	UFUNCTION()
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;

	FDelegateHandle OnDestroySessionCompleteDelegateHandle;

	// 
	// 
//UCreateSessionCallbackProxyAdvanced* UCreateSessionCallbackProxyAdvanced::CreateAdvancedSession(UObject* WorldContextObject, const TArray<FSessionPropertyKeyPair>& ExtraSettings, class APlayerController* PlayerController, int32 PublicConnections, int32 PrivateConnections, bool bUseLAN, bool bAllowInvites, bool bIsDedicatedServer, /*bool bUsePresence,*/ bool bUseLobbiesIfAvailable, bool bAllowJoinViaPresence, bool bAllowJoinViaPresenceFriendsOnly, bool bAntiCheatProtected, bool bUsesStats, bool bShouldAdvertise, bool bUseLobbiesVoiceChatIfAvailable, bool bStartAfterCreate)
// 여기서 return 하는 Proxy의 콜백을 직접 받아와서 호출 받을 수도 있다. 하지만 일단 블루프린트로 작업하자 나중에 복잡하게 할거면 그때 추가하던가

	
};
