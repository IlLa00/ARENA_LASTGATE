// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/OZOnlineSessionSubsystem.h"
#include "Instance/OZGameInstance.h"


//void UOZOnlineSessionSubsystem::EraseSession()
//{
//    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//    if (!Subsystem) return;
//
//    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//    if (!SessionInterface.IsValid()) return;
//
//    SessionInterface->EndSession(NAME_GameSession);
//    SessionInterface->RemoveNamedSession(NAME_GameSession);
//}

void UOZOnlineSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    OnlineSub = IOnlineSubsystem::Get();
    Identity = OnlineSub ? OnlineSub->GetIdentityInterface() : nullptr;
}

void UOZOnlineSessionSubsystem::TryLogin()
{
    int32 LocalUserNum = 0;

    if (!Identity.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Online Identity interface is not valid!"));
        return;
    }

    Identity->OnLoginCompleteDelegates[LocalUserNum].AddUObject(this, &UOZOnlineSessionSubsystem::OnLoginComplete);

    Identity->AutoLogin(LocalUserNum);
}

void UOZOnlineSessionSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{

    UserNetUniqueID = UserId.ToString();
    //UserID는 StandALone테스트 상태에서는 동일한 값이 나온다. 임시로 랜덤숫자 10자리 생성하여 사용하자
    FString TempUserID;
    for (int i = 0; i < 10; i++)
    {
        int32 Digit = FMath::RandRange(0, 9);
        TempUserID.AppendInt(Digit);
    }

    if (bWasSuccessful)
    {
        //UserIdentificationCode = UserId.ToString();
        UserIdentificationCode = TempUserID;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Login failed: %s"), *Error);
    }
}

void UOZOnlineSessionSubsystem::OnSessionCreateSuccess(FString CreateLevelName)
{
    UE_LOG(LogTemp, Log, TEXT("OnSessionCreateSuccess"));

}

void UOZOnlineSessionSubsystem::OnSessionJoinSuccess(FString JoinLevelName)
{
    UE_LOG(LogTemp, Log, TEXT("OnSessionJoinSuccess"));
}

void UOZOnlineSessionSubsystem::ExitSession(AOZPlayerController* exitPlayerConerller)
{
    UWorld* World = GetWorld();
    UOZGameInstance* ozGameInstacne = Cast<UOZGameInstance>(GetGameInstance());

    if (World->GetNetMode() != NM_Client)
    {
        //서버일 경우

    }

    else
    {
        //대기방 UI 업데이트
        exitPlayerConerller->ReportSessionExit(exitPlayerConerller);

        //ozGameInstacne->Client_ExitSession();

        //Run in One Progress false 일 경우
        //exitPlayerConerller->ClientTravel(TEXT("/Game/Level/Level_LobbyFlow/Fixed/Level_LobbySelect"), TRAVEL_Absolute);


        //IHGame 버젼
        exitPlayerConerller->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitButton")));


        //ozGameInstacne->Client_ExitSessionWithController(exitPlayerConerller);

        ////클라일 경우
        ////exitPlayerConerller->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitSession")));

        //exitPlayerConerller->ReportSessionExit(exitPlayerConerller);

        //IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        //if (!Subsystem) return;

        //IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        //if (!SessionInterface.IsValid()) return;

        //SessionInterface

        ////FName SessionName = NAME_GameSession;

        ////클라는 세션을 소유하고 있지 않는다.
        ///*auto Session = SessionInterface->GetNamedSession(SessionName);

        //if (Session == nullptr)
        //    return;*/

        ////if (SessionInterface->GetNamedSession(SessionName))
        //{
        //    /*OnDestroySessionCompleteDelegate =FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOZOnlineSessionSubsystem::OnDestroySessionComplete);

        //    OnDestroySessionCompleteDelegateHandle =SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);*/

        //    SessionInterface->EndSession(NAME_GameSession);
        //    SessionInterface->RemoveNamedSession(NAME_GameSession);
        //}

        //GoToLobby();
    }
}

//void UOZOnlineSessionSubsystem::ExitSession(TObjectPtr<class AOZPlayerController> exitPlayerConerller)
//{
//    UWorld* World = GetWorld();
//    UOZGameInstance* ozGameInstacne = Cast<UOZGameInstance>(GetGameInstance());
//
//    if (World->GetNetMode() != NM_Client)
//    {
//        //서버일 경우
//
//    }
//
//    else
//    {
//        //대기방 UI 업데이트
//        exitPlayerConerller->ReportSessionExit(exitPlayerConerller);
//
//        //ozGameInstacne->Client_ExitSession();
//
//        //Run in One Progress false 일 경우
//        //exitPlayerConerller->ClientTravel(TEXT("/Game/Level/Level_LobbyFlow/Fixed/Level_LobbySelect"), TRAVEL_Absolute);
//
//
//        //IHGame 버젼
//        exitPlayerConerller->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitButton")));
//
//
//        //ozGameInstacne->Client_ExitSessionWithController(exitPlayerConerller);
//
//        ////클라일 경우
//        ////exitPlayerConerller->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitSession")));
//
//        //exitPlayerConerller->ReportSessionExit(exitPlayerConerller);
//
//        //IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//        //if (!Subsystem) return;
//
//        //IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//        //if (!SessionInterface.IsValid()) return;
//
//        //SessionInterface
//
//        ////FName SessionName = NAME_GameSession;
//
//        ////클라는 세션을 소유하고 있지 않는다.
//        ///*auto Session = SessionInterface->GetNamedSession(SessionName);
//
//        //if (Session == nullptr)
//        //    return;*/
//
//        ////if (SessionInterface->GetNamedSession(SessionName))
//        //{
//        //    /*OnDestroySessionCompleteDelegate =FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOZOnlineSessionSubsystem::OnDestroySessionComplete);
//
//        //    OnDestroySessionCompleteDelegateHandle =SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);*/
//
//        //    SessionInterface->EndSession(NAME_GameSession);
//        //    SessionInterface->RemoveNamedSession(NAME_GameSession);
//        //}
//
//        //GoToLobby();
//    }
//    
//}

//void UOZOnlineSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
//{
//    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//    if (Subsystem)
//    {
//        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//        if (SessionInterface.IsValid())
//        {
//            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
//                OnDestroySessionCompleteDelegateHandle
//            );
//        }
//    }
//
//    UWorld* World = GetWorld();
//    if (!World)
//        return;
//
//    World->GetTimerManager().SetTimerForNextTick(
//        FTimerDelegate::CreateUObject(
//            this,
//            &UOZOnlineSessionSubsystem::GoToLobby
//        )
//    );
//}

//void UOZOnlineSessionSubsystem::GoToLobby()
//{
//    UWorld* World = GetWorld();
//    if (World && GEngine)
//    {
//        GEngine->SetClientTravel(World, TEXT("/Game/Level/Level_LobbyFlow/Fixed/Level_LobbySelect"), TRAVEL_Absolute);
//    }
//}

void UOZOnlineSessionSubsystem::CheckHasSessionAlready()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    if (!SessionInterface.IsValid()) return;


    ////////////////
    //IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    //if (Subsystem)
    //{
    //    //IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
    //    if (SessionInterface.IsValid())
    //    {
    //        // 1. 현재 세션 상태를 강제로 체크
    //        EOnlineSessionState::Type State = SessionInterface->GetSessionState(NAME_GameSession);

    //        if (State != EOnlineSessionState::NoSession)
    //        {
    //            UE_LOG(LogTemp, Warning, TEXT("Session state is %d, forcing destroy..."), (int32)State);

    //            // 콜백을 등록하고 파괴 성공 후 Join을 하거나, 
    //            // 안전하게 무조건 DestroySession을 한 번 더 호출합니다.
    //            SessionInterface->DestroySession(NAME_GameSession);
    //        }
    //    }
    //}
    /////////////////////////////////



    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);

    {
        OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOZOnlineSessionSubsystem::OnDestroySessionComplete);

        OnDestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);


        SessionInterface->DestroySession(NAME_GameSession);
    }
}

void UOZOnlineSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
                OnDestroySessionCompleteDelegateHandle
            );
        }
    }

    OnSessionCheckedEnd.Broadcast();
}
