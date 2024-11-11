#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterUnrestrictedItems() {
    REGISTER_VB_SHOULD(VB_ITEM_BE_RESTRICTED, {
        if (CVarGetInteger("gCheats.UnrestrictedItems", 0)) {
            *should = false;
        }
    });
}
