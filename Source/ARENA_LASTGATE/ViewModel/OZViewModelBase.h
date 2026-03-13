#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "OZViewModelBase.generated.h"

class AOZPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOZOnFloatPropertyChanged, FName, PropertyName, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOZOnIntPropertyChanged, FName, PropertyName, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOZOnBoolPropertyChanged, FName, PropertyName, bool, NewValue);

UCLASS(Abstract, BlueprintType)
class ARENA_LASTGATE_API UOZViewModelBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "ViewModel")
	FOZOnFloatPropertyChanged OnFloatPropertyChanged;

	UPROPERTY(BlueprintAssignable, Category = "ViewModel")
	FOZOnIntPropertyChanged OnIntPropertyChanged;

	UPROPERTY(BlueprintAssignable, Category = "ViewModel")
	FOZOnBoolPropertyChanged OnBoolPropertyChanged;

	UFUNCTION(BlueprintCallable, Category = "ViewModel")
	virtual void Initialize(APlayerController* OwningController);

	UFUNCTION(BlueprintCallable, Category = "ViewModel")
	virtual void Deinitialize();

	virtual void OnPlayerStateReady(AOZPlayerState* InPlayerState);

	UFUNCTION(BlueprintPure, Category = "ViewModel")
	bool IsInitialized() const { return bInitialized; }

protected:
	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPC;

	bool bInitialized = false;

	void NotifyFloatChanged(FName PropertyName, float NewValue);
	void NotifyIntChanged(FName PropertyName, int32 NewValue);
	void NotifyBoolChanged(FName PropertyName, bool NewValue);

	AOZPlayerState* GetLocalPlayerState() const;

	void StartPlayerStatePolling();

private:
	FTimerHandle PlayerStatePollTimer;
	void PollForPlayerState();
};
