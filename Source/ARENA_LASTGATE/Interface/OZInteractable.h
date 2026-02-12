#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OZInteractable.generated.h"

UINTERFACE(MinimalAPI)
class UOZInteractable : public UInterface
{
	GENERATED_BODY()
};

class ARENA_LASTGATE_API IOZInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void OnInteract(AActor* InteractActor);
};
