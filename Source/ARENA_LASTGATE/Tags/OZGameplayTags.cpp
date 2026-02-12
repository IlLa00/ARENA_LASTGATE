#include "OZGameplayTags.h"

namespace OZGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Fire, "Player.Ability.Fire");
    UE_DEFINE_GAMEPLAY_TAG(Player_Ability_Fire2, "Player.Ability.Fire2");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_Fireing, "Player.State.Firing");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_StaminaBlockRegen, "Player.State.StaminaBlockRegen");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_Dead, "Player.State.Dead");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_RecentlyDamaged, "Player.State.RecentlyDamaged");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_HealthNotFull, "Player.State.HealthNotFull");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_InBush, "Player.State.InBush");
    UE_DEFINE_GAMEPLAY_TAG(Player_State_Revealed, "Player.State.Revealed");

    // 버프 아이템 태그 (Duration UI 표시용, Item.Buff의 자식 태그들도 매칭)
    UE_DEFINE_GAMEPLAY_TAG(Item_Buff, "Item.Buff");
}