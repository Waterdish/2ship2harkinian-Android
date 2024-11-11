#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterTatlISG() {
    REGISTER_VB_SHOULD(VB_TATL_CONVERSATION_AVAILABLE, {
        if (CVarGetInteger("gEnhancements.Restorations.TatlISG", 0)) {
            *should = false;
        }
    });
}
