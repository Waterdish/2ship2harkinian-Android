#include <libultraship/bridge.h>
#include "GameInteractor/GameInteractor.h"
#include "Enhancements/Enhancements.h"

extern "C" {
#include <z64save.h>
}

void RegisterCremiaHugs() {
    REGISTER_VB_SHOULD(VB_PLAY_CREMIA_HUG_CUTSCENE, {
        uint8_t selectedOption = CVarGetInteger("gEnhancements.Minigames.CremiaHugs", 0);
        if (selectedOption == CREMIA_REWARD_ALWAYS_HUG) {
            *should = true;
        } else if (selectedOption == CREMIA_REWARD_ALWAYS_RUPEE) {
            *should = false;
        }
    });
}
