
#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64save.h"
#include "overlays/actors/ovl_Bg_Breakwall/z_bg_breakwall.h"
}

typedef enum {
    /*  2 */ BGBREAKWALL_F_2 = 2, // Poisoned Woodfall Mountain
    /*  3 */ BGBREAKWALL_F_3 = 3, // Spring Woodfall Mountain
} BgBreakwallParamEx;

void RegisterWoodfallMountainAppearance() {
    static HOOK_ID breakwallInitID = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorInit>(breakwallInitID);
    breakwallInitID = 0;

    if (!CVarGetInteger("gEnhancements.Restorations.WoodfallMountainAppearance", 0)) {
        return;
    }

    breakwallInitID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_BG_BREAKWALL, [](Actor* actor) {
            if (BGBREAKWALL_GET_F(actor) == BGBREAKWALL_F_2 &&
                CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE)) {
                actor->params = (actor->params & 0xFFF0) | BGBREAKWALL_F_3;
            }
        });
}
