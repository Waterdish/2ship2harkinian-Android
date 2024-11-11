#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterHideTitleCards() {
    REGISTER_VB_SHOULD(VB_SHOW_TITLE_CARD, {
        if (CVarGetInteger("gEnhancements.Cutscenes.HideTitleCards", 0)) {
            *should = false;
        }
    });
}
