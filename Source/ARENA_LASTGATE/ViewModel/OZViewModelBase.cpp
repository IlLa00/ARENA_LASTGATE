#include "ViewModel/OZViewModelBase.h"
#include "Character/OZPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UOZViewModelBase::Initialize(APlayerController* InController)
{
	OwningPC = InController;

	if (AOZPlayerState* PS = GetLocalPlayerState())
	{
		OnPlayerStateReady(PS);
		bInitialized = true;
	}
	else
		StartPlayerStatePolling();
}

void UOZViewModelBase::Deinitialize()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(PlayerStatePollTimer);

	bInitialized = false;
}

void UOZViewModelBase::OnPlayerStateReady(AOZPlayerState* InPlayerState)
{
	
}

AOZPlayerState* UOZViewModelBase::GetLocalPlayerState() const
{
	if (OwningPC.IsValid())
		return OwningPC->GetPlayerState<AOZPlayerState>();

	return nullptr;
}

void UOZViewModelBase::StartPlayerStatePolling()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PlayerStatePollTimer,
			this,
			&UOZViewModelBase::PollForPlayerState,
			0.1f,
			true
		);
	}
}

void UOZViewModelBase::PollForPlayerState()
{
	if (AOZPlayerState* PS = GetLocalPlayerState())
	{
		if (UWorld* World = GetWorld())
			World->GetTimerManager().ClearTimer(PlayerStatePollTimer);

		OnPlayerStateReady(PS);
		bInitialized = true;
	}
}

void UOZViewModelBase::NotifyFloatChanged(FName PropertyName, float NewValue)
{
	OnFloatPropertyChanged.Broadcast(PropertyName, NewValue);
}

void UOZViewModelBase::NotifyIntChanged(FName PropertyName, int32 NewValue)
{
	OnIntPropertyChanged.Broadcast(PropertyName, NewValue);
}

void UOZViewModelBase::NotifyBoolChanged(FName PropertyName, bool NewValue)
{
	OnBoolPropertyChanged.Broadcast(PropertyName, NewValue);
}
