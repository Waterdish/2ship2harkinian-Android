#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include <variables.h>
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
void Flags_SetWeekEventReg(s32 flag);
}

void RegisterSkipIntroSequence() {
    REGISTER_VB_SHOULD(VB_PLAY_TRANSITION_CS, {
        // Intro cutscene
        if (!(gSaveContext.save.entrance == ENTRANCE(CUTSCENE, 0) && gSaveContext.save.cutsceneIndex == 0))
            return;

        if (CVarGetInteger("gEnhancements.Cutscenes.SkipIntroSequence", 0)) {
            gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
            gSaveContext.save.hasTatl = true;
            gSaveContext.cycleSceneFlags[SCENE_INSIDETOWER].switch0 |= (1 << 0);
            gSaveContext.cycleSceneFlags[SCENE_OPENINGDAN].switch0 |= (1 << 2);
            gSaveContext.cycleSceneFlags[SCENE_OPENINGDAN].switch0 |= (1 << 0);
            gSaveContext.save.playerForm = PLAYER_FORM_DEKU;

            // Mark chest as opened and give the player the Deku Nuts
            gSaveContext.cycleSceneFlags[SCENE_OPENINGDAN].chest |= (1 << 0);
            Item_Give(gPlayState, ITEM_DEKU_NUTS_10);

            if (CVarGetInteger("gEnhancements.Cutscenes.SkipFirstCycle", 0)) {
                gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                Item_Give(gPlayState, ITEM_OCARINA_OF_TIME);
                Item_Give(gPlayState, ITEM_MASK_DEKU);
                gSaveContext.save.saveInfo.inventory.questItems = (1 << QUEST_SONG_TIME) | (1 << QUEST_SONG_HEALING);
                if (gSaveContext.save.saveInfo.playerData.threeDayResetCount < 1) {
                    gSaveContext.save.saveInfo.playerData.threeDayResetCount = 1;
                }
                gSaveContext.save.isFirstCycle = false;

                // Lose nuts
                AMMO(ITEM_DEKU_NUT) = 0;

                // Tatl's text at seeing the broken great fairy
                gSaveContext.cycleSceneFlags[SCENE_YOUSEI_IZUMI].switch0 |= (1 << 10);

                // Tatl's text when first entering South Clock Town if not already set
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_59_04)) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_59_04);
                }

                // Set Tatl second cycle (?) text if not already set
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_31_04)) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_31_04);
                }

                // Set Entrance Cutscene flags for Clock Town if not already set
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_EAST_CLOCK_TOWN)) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_ENTERED_EAST_CLOCK_TOWN);
                }
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WEST_CLOCK_TOWN)) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_ENTERED_WEST_CLOCK_TOWN);
                }
                if (!CHECK_WEEKEVENTREG(WEEKEVENTREG_ENTERED_NORTH_CLOCK_TOWN)) {
                    SET_WEEKEVENTREG(WEEKEVENTREG_ENTERED_NORTH_CLOCK_TOWN);
                }
            }
        }
    });
}
