// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/OzPracticeGameMode.h"
#include "Character/OZPlayer.h"
#include "Character/OZPlayerController.h"
#include "Character/OZPlayerState.h"
#include "HUD/OZPracticeHUD.h"
#include "Subsystem/OZConvertSubsystem.h"

AOZPracticeGameMode::AOZPracticeGameMode()
{
	static ConstructorHelpers::FClassFinder<AOZPlayer> OZPlayer(TEXT("/Game/Blueprint/Player/BP_OZPlayer"));
	if (OZPlayer.Class)
	{
		DefaultPawnClass = OZPlayer.Class;
	}

	static ConstructorHelpers::FClassFinder<AOZPlayerController> OZPlayerController(TEXT("/Game/Blueprint/Player/BP_OZPlayerController"));
	if (OZPlayerController.Class)
	{
		PlayerControllerClass = OZPlayerController.Class;
	}

	static ConstructorHelpers::FClassFinder<AOZPracticeHUD> OZPracticeHUD(TEXT("/Game/UI/HUD/BP_PracticeHUD"));
	if (OZPracticeHUD.Class)
	{
		HUDClass = OZPracticeHUD.Class;
	}

	static ConstructorHelpers::FClassFinder<AOZPlayerState> OZPlayerState(TEXT("/Game/Blueprint/Player/BP_OZPlayerState"));
	if (OZPlayerState.Class)
	{
		PlayerStateClass = OZPlayerState.Class;
	}
}

/*
TArray<FEffectData> AOZPracticeGameMode::GetAllEffects()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
		return TArray<FEffectData>();

	UOZEffectSubsystem* EffectSubsystem = GameInstance->GetSubsystem<UOZEffectSubsystem>();
	if (!EffectSubsystem)
		return TArray<FEffectData>();

	UOZEffectDataAsset* EffectDataAsset = EffectSubsystem->GetEffectDataAsset();
	if (!EffectDataAsset)
		return TArray<FEffectData>();

	return EffectDataAsset->AllEffects;
}
*/
      
