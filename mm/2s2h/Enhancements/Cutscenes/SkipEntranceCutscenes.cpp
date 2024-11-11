#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterSkipEntranceCutscenes() {
    REGISTER_VB_SHOULD(VB_PLAY_ENTRANCE_CS, {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipEntranceCutscenes", 0)) {
            *should = false;
        }
    });
}
