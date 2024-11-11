#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterPowerCrouchStab() {
    REGISTER_VB_SHOULD(VB_PATCH_POWER_CROUCH_STAB, {
        if (CVarGetInteger("gEnhancements.Restorations.PowerCrouchStab", 0)) {
            *should = false;
        }
    });
}
