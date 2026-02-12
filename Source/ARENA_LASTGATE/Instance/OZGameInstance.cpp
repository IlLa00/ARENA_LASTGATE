// Fill out your copyright notice in the Description page of Project Settings.


#include "Instance/OZGameInstance.h"
#include "Subsystem/OZOnlineSessionSubsystem.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Subsystem/OZItemSubsystem.h"
#include "Character/OZPlayerController.h"
#include "Subsystem/OZModuleSubsystem.h"

void UOZGameInstance::Init()
{
	Super::Init();

	SessionSubsystem = GetSubsystem<UOZOnlineSessionSubsystem>();
	ItemSubsystem = GetSubsystem<UOZItemSubsystem>();
	ModuleSubsystem = GetSubsystem<UOZModuleSubsystem>();
	ConvertSubsystem = GetSubsystem<UOZConvertSubsystem>();
}

void UOZGameInstance::PrintLog(const FString& LogText, FColor textColor, float time)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			time,
			textColor,
			LogText
		);
	}
}

//void UOZGameInstance::Client_ExitSession()
//{
//	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	if (!Subsystem) return;
//
//	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//	if (!SessionInterface.IsValid()) return;
//
//	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
//
//	{
//		OnDestroySessionCompleteDelegate =FOnDestroySessionCompleteDelegate::CreateUObject(this, &UOZGameInstance::OnDestroySessionComplete);
//
//		OnDestroySessionCompleteDelegateHandle =SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegate);
//
//
//		SessionInterface->DestroySession(NAME_GameSession);
//	}
//}
//
//void UOZGameInstance::Client_ExitSessionWithController(TObjectPtr<class AOZPlayerController> ozController)
//{
//	ozController->ClientReturnToMainMenuWithTextReason(FText::FromString(TEXT("ExitButton")));
//}
//
//void UOZGameInstance::EraseSession()
//{
//	/*IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	if (!Subsystem) return;
//
//	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//	if (!SessionInterface.IsValid()) return;
//
//	SessionInterface->EndSession(NAME_GameSession);
//	SessionInterface->RemoveNamedSession(NAME_GameSession);*/
//}
//
//void UOZGameInstance::Server_ExitSession()
//{
//}
//
//void UOZGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
//{
//	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	if (Subsystem)
//	{
//		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
//		if (SessionInterface.IsValid())
//		{
//			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
//				OnDestroySessionCompleteDelegateHandle
//			);
//		}
//	}
//
//	GoToLobby();
//}
//
//void UOZGameInstance::GoToLobby()
//{
//	UWorld* World = GetWorld();
//	if (World && GEngine)
//	{
//		GEngine->SetClientTravel(World, TEXT("/Game/Level/Level_LobbyFlow/Fixed/Level_LobbySelect"), TRAVEL_Absolute);
//	}
//}


#if WITH_EDITOR
void UOZGameInstance::UpdateAndBuildMesh(UStaticMesh* Mesh)
{
	if(Mesh)
		Mesh->PostEditChange();
}
#endif