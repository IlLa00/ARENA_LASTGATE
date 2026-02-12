// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZWidgetComponent.h"
#include "Character/OZPlayerAttributeSet.h"
#include "Widget/OZPlayerStatusBarWidget.h"
#include "Character/OZPlayerState.h"

void UOZWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	// 5.5 엔진 버그( WidgetComponent가 서버에서만 제대로 안보이는 경우가 있다)
	if (UWorld* World = GetWorld())
	{
		OwnerPlayer = World->GetFirstLocalPlayerFromController();
	}
}

void UOZWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* playerPawn = Cast<APawn>(GetOwner());

	if (playerPawn == nullptr)
		return;

	if (OZPlayerState == nullptr)
	{
		OZPlayerState = Cast<AOZPlayerState>(playerPawn->GetPlayerState());
	}
	
	if (OZPlayerState == nullptr)
		return;

	UUserWidget* userWidget = GetUserWidgetObject();
	if (userWidget == nullptr)
		return;

	UOZPlayerStatusBarWidget* statusBarWidget = Cast<UOZPlayerStatusBarWidget>(userWidget);
	if (statusBarWidget == nullptr)
		return;

	statusBarWidget->SetTargetPlayerState(OZPlayerState);

	SetComponentTickEnabled(false);
}
