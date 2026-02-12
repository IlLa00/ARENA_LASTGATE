#pragma once

#include "NativeGameplayTags.h"

namespace OZGameplayTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Fire);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_Ability_Fire2);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Fireing);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_StaminaBlockRegen);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_RecentlyDamaged);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_HealthNotFull);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_InBush);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player_State_Revealed);

    // 버프 아이템 태그 (Duration UI 표시용, Item.Buff의 자식 태그들도 매칭)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Buff);
}