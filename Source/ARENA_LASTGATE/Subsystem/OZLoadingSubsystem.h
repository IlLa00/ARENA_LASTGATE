// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UserInterface/OZLoadingScreen.h"
#include "OZLoadingSubsystem.generated.h"

/**
 * 
 */
UCLASS(Config = Engine)
class ARENA_LASTGATE_API UOZLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UOZLoadingSubsystem();

	UPROPERTY()
	TSubclassOf<UOZLoadingScreen> LoadingWidgetClass;

	UPROPERTY()
	TObjectPtr<UOZLoadingScreen> ActivatedLoadingWidget = nullptr;

	TSharedPtr<SWidget> ActivatedSlateWidget;

public:
	UFUNCTION(BlueprintCallable)
	void PlayLoadingScene();

	UFUNCTION(BlueprintCallable)
	void EraseLoadingScene();

private:
	UFUNCTION()
	void OnClientTravelFailed(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	//LoadingTimeOut
	UPROPERTY()
	float LoadingElapsedTime = 0.f;

	UPROPERTY()
	bool bLoadingActive = false;

	UPROPERTY()
	float LoadingTimeout = 60.f;

	FTSTicker::FDelegateHandle TickHandle;

	bool Tick(float DeltaTime);

	void HandleLoadingTimeout();

	void HandleJoinSessionFail();
};
