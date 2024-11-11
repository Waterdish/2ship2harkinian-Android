#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterElegyAnywhere() {
    REGISTER_VB_SHOULD(VB_ELEGY_CHECK_SCENE, {
        if (CVarGetInteger("gCheats.ElegyAnywhere", 0)) {
            *should = true;
        }
    });
}
