// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/OZLoadingSubsystem.h"
#include "Instance/OZGameInstance.h"
#include "UserInterface/SLoadingScreen.h"
#include "MoviePlayer.h"
#include "Engine/Engine.h"

void UOZLoadingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GEngine)
	{
		GEngine->OnTravelFailure().AddUObject(this, &UOZLoadingSubsystem::OnClientTravelFailed);
	}
}

UOZLoadingSubsystem::UOZLoadingSubsystem()
{
	static ConstructorHelpers::FClassFinder<UOZLoadingScreen> LoadingSceneClass(TEXT("/Game/UI/Layouts/WBP_LoadingScreenUI.WBP_LoadingScreenUI_C"));

	if (LoadingSceneClass.Class != nullptr)
	{
		LoadingWidgetClass = LoadingSceneClass.Class;
	}

	static ConstructorHelpers::FObjectFinder<UOZLoadingScreenData>DataAsset(TEXT("/Game/Data/DA_LoadingScreen.DA_LoadingScreen"));

	if (DataAsset.Object != nullptr)
	{
		LoadingScreenData = DataAsset.Object;
	}

}

void UOZLoadingSubsystem::PlayLoadingScene()
{
	//무비버젼
	{
		//if (!ActivatedSlateWidget.IsValid())
		//{
		//	ActivatedSlateWidget =
		//		SNew(SLoadingScreen)
		//		.BackGroundTextures(LoadingScreenData->BackGroundTextures)
		//		.HintTextures(LoadingScreenData->HintTexts)
		//		.LogoTexture(LoadingScreenData->LogoTexture)
		//		.TrobberTexture(LoadingScreenData->TrobberTexture)
		//		.CoreTexture(LoadingScreenData->CoreTexture);
		//}

		////Add to viewprot 버젼, 어젼히 멈춤
		///*if (GEngine && GEngine->GameViewport)
		//{
		//	GEngine->GameViewport->AddViewportWidgetContent(ActivatedSlateWidget.ToSharedRef(), 999);
		//}*/

		////무비 플레이만이 유일한 해결책
		//FLoadingScreenAttributes LoadingAttributes;
		//LoadingAttributes.WidgetLoadingScreen = ActivatedSlateWidget;
		//LoadingAttributes.bAutoCompleteWhenLoadingCompletes = false;
		//LoadingAttributes.bWaitForManualStop = true;
		//LoadingAttributes.MinimumLoadingScreenDisplayTime = 0.f;

		//GetMoviePlayer()->SetupLoadingScreen(LoadingAttributes);
	}

	//기존버젼
	{
		if (ActivatedLoadingWidget == nullptr)
		{
			ActivatedLoadingWidget = CreateWidget<UOZLoadingScreen>(GetGameInstance(), LoadingWidgetClass);
		}

		if (ActivatedLoadingWidget && GEngine && GEngine->GameViewport)
		{

			if (!ActivatedSlateWidget.IsValid())
			{
				ActivatedSlateWidget = ActivatedLoadingWidget->TakeWidget();
			}

			GEngine->GameViewport->AddViewportWidgetContent(ActivatedSlateWidget.ToSharedRef(), 99);
		}

		//AddtoViewport�� ���忡 �����ִ� ���̶�, ���� ��ȯ�� ������
		//ActivatedLoadingWidget->AddToViewport(999);

		LoadingElapsedTime = 0.f;
		bLoadingActive = true;

		if (!TickHandle.IsValid())
		{
			TickHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UOZLoadingSubsystem::Tick)
			);
		}

		Cast<UOZGameInstance>(GetGameInstance())->PrintLog(TEXT("PlayLoadingScene"), FColor::Green, 1.f);
	}

}

void UOZLoadingSubsystem::EraseLoadingScene()
{
	{
		// 먼저 로딩 상태 플래그 해제 (타임아웃 로직 정리)
		bLoadingActive = false;

		if (ActivatedLoadingWidget == nullptr)
			return;

		if (GEngine && GEngine->GameViewport && ActivatedSlateWidget.IsValid())
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ActivatedSlateWidget.ToSharedRef());
			ActivatedSlateWidget.Reset();
		}

		if (TickHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
			TickHandle.Reset();
		}


		ActivatedLoadingWidget = nullptr;

		Cast<UOZGameInstance>(GetGameInstance())->PrintLog(TEXT("EraseLoadingScene"), FColor::Green, 1.f);
	}


	//MoviePlayer 버젼
	{
		///*if (ActivatedSlateWidget.IsValid() && GEngine && GEngine->GameViewport)
		//{
		//	GEngine->GameViewport->RemoveViewportWidgetContent(
		//		ActivatedSlateWidget.ToSharedRef()
		//	);
		//}*/

		//if (GetMoviePlayer())
		//{
		//	GetMoviePlayer()->StopMovie();
		//}

		//ActivatedSlateWidget.Reset();
	}
}

void UOZLoadingSubsystem::OnClientTravelFailed(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Client Travel Failed! FailureType=%d Error=%s"), (int32)FailureType, *ErrorString);

	HandleJoinSessionFail();
}

bool UOZLoadingSubsystem::Tick(float DeltaTime)
{
	if (!bLoadingActive)
		return true;

	LoadingElapsedTime += DeltaTime;

	if (LoadingElapsedTime >= LoadingTimeout)
	{
		HandleLoadingTimeout();
	}

	return true;
}

void UOZLoadingSubsystem::HandleLoadingTimeout()
{
	bLoadingActive = false;

	ActivatedLoadingWidget->OnTimeOut();

	EraseLoadingScene();
}

void UOZLoadingSubsystem::HandleJoinSessionFail()
{
	bLoadingActive = false;

	ActivatedLoadingWidget->OnClientLoadingFail();

	EraseLoadingScene();
}
