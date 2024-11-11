#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
void Flags_SetWeekEventReg(s32 flag);
extern SaveContext gSaveContext;
extern PlayState* gPlayState;
}

void RegisterSkipClockTowerOpen() {
    // This will handle skipping if you are around the Clock Town area, but not directly in south clock town
    REGISTER_VB_SHOULD(VB_CLOCK_TOWER_OPENING_CONSIDER_THIS_FIRST_CYCLE, {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipStoryCutscenes", 0)) {
            *should = false;
        }
    });

    // This will handle skipping if you are directly in South Clock Town
    REGISTER_VB_SHOULD(VB_PLAY_TRANSITION_CS, {
        if ((gSaveContext.save.entrance == ENTRANCE(SOUTH_CLOCK_TOWN, 0) ||
             gSaveContext.save.entrance == ENTRANCE(TERMINA_FIELD, 0)) &&
            gSaveContext.save.cutsceneIndex == 0xFFF1 &&
            CVarGetInteger("gEnhancements.Cutscenes.SkipStoryCutscenes", 0)) {
            // Copied from ObjTokeidai_TowerOpening_EndCutscene
            SET_WEEKEVENTREG(WEEKEVENTREG_CLOCK_TOWER_OPENED);
            gSaveContext.save.cutsceneIndex = 0;
            gSaveContext.respawnFlag = 2;
            gSaveContext.save.entrance = gSaveContext.respawn[RESPAWN_MODE_RETURN].entrance;
            if (gSaveContext.respawn[RESPAWN_MODE_RETURN].playerParams ==
                PLAYER_PARAMS(0xFF, PLAYER_INITMODE_TELESCOPE)) {
                gSaveContext.nextTransitionType = TRANS_TYPE_CIRCLE;
            } else {
                gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK;
            }
        }
    });
}
