#pragma once

UENUM(BlueprintType)
enum class EOZAbilityInputID : uint8
{
    None        UMETA(DisplayName = "None"),   // 0

    // UI / Target-related
    Confirm     UMETA(DisplayName = "Confirm"), // 1
    Cancel      UMETA(DisplayName = "Cancel"),  // 2

    // Movement / Combat Abilities
    Sprint      UMETA(DisplayName = "Sprint"),
    Aiming      UMETA(DisplayName = "Aiming"),
    Dash        UMETA(DisplayName = "Dash"),
    Fire        UMETA(DisplayName = "Fire"),
};