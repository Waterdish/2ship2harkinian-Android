#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterUnbreakableRazorSword() {
    REGISTER_VB_SHOULD(VB_LOWER_RAZOR_SWORD_DURABILITY, {
        if (CVarGetInteger("gCheats.UnbreakableRazorSword", 0)) {
            *should = false;
        }
    });
}
