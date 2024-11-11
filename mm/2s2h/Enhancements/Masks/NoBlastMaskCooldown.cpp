#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterNoBlastMaskCooldown() {
    REGISTER_VB_SHOULD(VB_SET_BLAST_MASK_COOLDOWN_TIMER, {
        if (CVarGetInteger("gEnhancements.Masks.NoBlastMaskCooldown", 0)) {
            *should = false;
        }
    });
}
