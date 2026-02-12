#include "OZCameraAimComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UOZCameraAimComponent::UOZCameraAimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UOZCameraAimComponent::BeginPlay()
{
	Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner) return;

    CameraBoom = Owner->FindComponentByClass<USpringArmComponent>();
    if (CameraBoom)
    {
        DefaultSocketOffset = CameraBoom->SocketOffset;
        DefaultTargetArmLength = CameraBoom->TargetArmLength;
    }
}

void UOZCameraAimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CameraBoom) return;

    FVector TargetOffset = DefaultSocketOffset;
    float TargetArmLength = DefaultTargetArmLength;

    if (bIsAiming)
    {
        TargetOffset += BaseAimOffset;

        TargetArmLength += AdditionalCameraDistance;

        APawn* OwnerPawn = Cast<APawn>(GetOwner());
        if (OwnerPawn)
        {
            APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
            if (PC)
            {
                float MouseX, MouseY;
                int32 ViewX, ViewY;
                if (PC->GetMousePosition(MouseX, MouseY))
                {
                    PC->GetViewportSize(ViewX, ViewY);

                    float NX = (MouseX - ViewX * 0.5f) / (ViewX * 0.5f);
                    float NY = (MouseY - ViewY * 0.5f) / (ViewY * 0.5f);

                    NX = FMath::Clamp(NX, -1.f, 1.f);
                    NY = FMath::Clamp(NY, -1.f, 1.f);

                    FVector MouseOffset(
                        0.f,                          
                        NX * HorizontalIntensity,     
                        -NY * VerticalIntensity       
                    );

                    TargetOffset += MouseOffset;
                }
            }
        }
    }

    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaTime, InterpSpeed);
    CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);
}

void UOZCameraAimComponent::StartAim()
{
    bIsAiming = true;
}

void UOZCameraAimComponent::StopAim()
{
    bIsAiming = false;
}

void UOZCameraAimComponent::SetAdditionalCameraDistance(float NewDistance)
{
    AdditionalCameraDistance = NewDistance;
}

void UOZCameraAimComponent::SetDefaultTargetArmLength(float NewDefaultLength)
{
    DefaultTargetArmLength = NewDefaultLength;

    if (CameraBoom && !bIsAiming)
    {
        CameraBoom->TargetArmLength = NewDefaultLength;
    }
}

