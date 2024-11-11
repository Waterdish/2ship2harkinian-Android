#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterDisableTakkuriSteal() {
    static HOOK_ID thiefBirdStealHookID = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(thiefBirdStealHookID);
    thiefBirdStealHookID = 0;

    if (!CVarGetInteger("gEnhancements.Cheats.DisableTakkuriSteal", 0)) {
        return;
    }

    thiefBirdStealHookID = REGISTER_VB_SHOULD(VB_THIEF_BIRD_STEAL, { *should = false; });
}