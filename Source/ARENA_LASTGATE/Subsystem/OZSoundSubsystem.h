#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OZSoundSubsystem.generated.h"

UCLASS(Config=Engine)
class ARENA_LASTGATE_API UOZSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void PlayBGM(USoundBase* BGMSound, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopBGM(float FadeOutDuration = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void StopBGMAndPlayNext(USoundBase* NextBGMSound, float FadeOutDuration = 0.5f, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound")
	bool IsBGMPlaying() const;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	bool IsPlayingThisBGM(USoundBase* BGMSound) const;

private:
	void OnFadeOutComplete();

	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMAudioComponent;

	UPROPERTY()
	TObjectPtr<USoundBase> CurrentBGM;

	UPROPERTY()
	TObjectPtr<USoundBase> PendingNextBGM;

	float PendingNextBGMVolume = 1.0f;

	FTimerHandle FadeOutTimerHandle;
};
