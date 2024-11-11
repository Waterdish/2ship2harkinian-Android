#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterVariableFlipHop() {
    REGISTER_VB_SHOULD(VB_FLIP_HOP_VARIABLE, {
        if (CVarGetInteger("gEnhancements.Restorations.ConstantFlipsHops", 0)) {
            *should = false;
        }
    });
}
