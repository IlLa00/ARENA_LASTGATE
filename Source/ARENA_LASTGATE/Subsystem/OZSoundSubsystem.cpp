#include "Subsystem/OZSoundSubsystem.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UOZSoundSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UOZSoundSubsystem::Deinitialize()
{
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
	}

	if (BGMAudioComponent)
	{
		BGMAudioComponent->Stop();
		BGMAudioComponent = nullptr;
	}
	CurrentBGM = nullptr;
	PendingNextBGM = nullptr;

	Super::Deinitialize();
}

void UOZSoundSubsystem::PlayBGM(USoundBase* BGMSound, float Volume)
{
	if (!BGMSound)
		return;

	if (CurrentBGM == BGMSound && BGMAudioComponent && BGMAudioComponent->IsPlaying())
		return;

	if (BGMAudioComponent && BGMAudioComponent->IsPlaying())
		BGMAudioComponent->Stop();

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	BGMAudioComponent = UGameplayStatics::SpawnSound2D(
		World,
		BGMSound,
		Volume,
		1.0f,   
		0.0f,   
		nullptr, 
		true,   // bPersistAcrossLevelTransition: ���� �̵� �ÿ��� ����!
		true    
	);

	if (BGMAudioComponent)
	{
		// �߰� ������ġ : ���� ��ȯ �� ���� Flush ����
		BGMAudioComponent->bIgnoreForFlushing = true;
		CurrentBGM = BGMSound;
	}
}

void UOZSoundSubsystem::StopBGM(float FadeOutDuration)
{
	if (!BGMAudioComponent || !BGMAudioComponent->IsPlaying())
		return;

	BGMAudioComponent->FadeOut(FadeOutDuration, 0.0f);
	CurrentBGM = nullptr;
}

void UOZSoundSubsystem::StopBGMAndPlayNext(USoundBase* NextBGMSound, float FadeOutDuration, float Volume)
{
	if (!NextBGMSound)
		return;

	PendingNextBGM = NextBGMSound;
	PendingNextBGMVolume = Volume;

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
		return;

	if (BGMAudioComponent && BGMAudioComponent->IsPlaying())
	{
		BGMAudioComponent->FadeOut(FadeOutDuration, 0.0f);
		CurrentBGM = nullptr;

		World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		World->GetTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&UOZSoundSubsystem::OnFadeOutComplete,
			FadeOutDuration,
			false
		);
	}
	else
	{
		OnFadeOutComplete();
	}
}

void UOZSoundSubsystem::OnFadeOutComplete()
{
	if (PendingNextBGM)
	{
		PlayBGM(PendingNextBGM, PendingNextBGMVolume);
		PendingNextBGM = nullptr;
	}
}

bool UOZSoundSubsystem::IsBGMPlaying() const
{
	return BGMAudioComponent && BGMAudioComponent->IsPlaying();
}

bool UOZSoundSubsystem::IsPlayingThisBGM(USoundBase* BGMSound) const
{
	return BGMSound && CurrentBGM == BGMSound && BGMAudioComponent && BGMAudioComponent->IsPlaying();
}
