#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
extern SaveContext gSaveContext;
void Flags_SetWeekEventReg(s32 flag);
}

void RegisterSkipDekuSalesman() {
    // prevents him from doing his "fly in" animation
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorInit>(
        ACTOR_EN_SELLNUTS, [](Actor* actor, bool* should) {
            if (CVarGetInteger("gEnhancements.Cutscenes.SkipMiscInteractions", 0) &&
                !CHECK_WEEKEVENTREG(WEEKEVENTREG_73_04)) {
                SET_WEEKEVENTREG(WEEKEVENTREG_73_04);
            }
        });
}
