#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogOZ, Log, All);

// 값 출력 (UE_LOG) 
#define LOG_VALUE(Value) \
    UE_LOG(LogOZ, Log, TEXT("[%s:%d] %s = %s"), *FString(__FUNCTION__), __LINE__, TEXT(#Value), *FString::Printf(TEXT("%s"), *OZLogUtils::ToString(Value)))

// 값 출력 (화면) 
#define SCREEN_VALUE(Value) \
    if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("[%s:%d] %s = %s"), *FString(__FUNCTION__), __LINE__, TEXT(#Value), *OZLogUtils::ToString(Value))); }

// 현재 라인 출력
#define PRINTLINE() \
    UE_LOG(LogOZ, Log, TEXT("[%s:%d] >> HERE"), *FString(__FUNCTION__), __LINE__)


// 이건 사용하는거 아님
namespace OZLogUtils
{
    inline FString ToString(int32 Value) { return FString::FromInt(Value); }
    inline FString ToString(float Value) { return FString::SanitizeFloat(Value); }
    inline FString ToString(bool Value) { return Value ? TEXT("true") : TEXT("false"); }
    inline FString ToString(const FString& Value) { return Value; }
    inline FString ToString(const FName& Value) { return Value.ToString(); }
    inline FString ToString(const FVector& Value) { return Value.ToString(); }
    inline FString ToString(const FRotator& Value) { return Value.ToString(); }
    inline FString ToString(const UObject* Value) { return Value ? Value->GetName() : TEXT("nullptr"); }
}

//Enums
UENUM(BlueprintType)
enum class EGameStateType : uint8
{
    None            UMETA(DisplayName = "None"),
    STATE_PREP      UMETA(DisplayName = "Prepare"),
    STATE_COMBAT    UMETA(DisplayName = "Battle"),
    STATE_RESULT    UMETA(DisplayName = "Result"),
    STATE_GAMEEND   UMETA(DisplayName = "GameEnd"),
    MAX             UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EInGameUIType : uint8
{
    INGAMEFLOORUI       UMETA(DisplayName = "Floor UI"),
    ROUND_PREPARE       UMETA(DisplayName = "Prepare"),
    SHOP_UI             UMETA(DisplayName = "Shop"),
    AVILITY_UI          UMETA(DisplayName = "Avility"),
    ROUND_COMBAT        UMETA(DisplayName = "Combat"),
    ROUND_COMBATRESULT  UMETA(DisplayName = "CombatResult"),
    GAME_END            UMETA(DisplayName = "GameEnd")
};

UENUM(BlueprintType)
enum class ELobbyUIType : uint8
{
    LOBBYUI             UMETA(DisplayName = "Lobby"),
    LOBBYSELECTUI       UMETA(DisplayName = "LobbySelect"),
    LOBBYMAKEROOMUI     UMETA(DisplayName = "LobbyMakeRoom"),
    LOBBYJOINROOMUI     UMETA(DisplayName = "LobbyJoinRoom"),
    LOGINPOPUPUI        UMETA(DisplayName = "LogInPopup")
};

UENUM(BlueprintType)
enum class EInputMode : uint8
{
    GameAndUI,
    GameOnly,
    UIOnly,
};

UENUM(BlueprintType)
enum class ECombatBonus : uint8
{
    MaxKill,
    MaxDamage,
    MaxHit,
    Max
};

UENUM(BlueprintType)
enum class EOZFogState : uint8
{
    Idle,
    Waiting,
    Shrinking,
    Final // Phase5 shrink 끝나고 안전구역 0 유지(전체 안개)
};

UENUM(BlueprintType)
enum class ELobbyUserState : uint8
{
    None,
    Participate,
    Ready
};

UENUM(BlueprintType)
enum class EMinimapActorCategory : uint8
{
    Structure UMETA(DisplayName = "Structure"),
    Character UMETA(DisplayName = "Character")
};
