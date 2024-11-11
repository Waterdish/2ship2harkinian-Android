#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterSideRoll() {
    REGISTER_VB_SHOULD(VB_PATCH_SIDEROLL, {
        if (CVarGetInteger("gEnhancements.Restorations.SideRoll", 0)) {
            *should = false;
        }
    });
}
