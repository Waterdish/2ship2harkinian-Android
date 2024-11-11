#include <libultraship/libultraship.h>
#include "BenPort.h"
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <functions.h>
}

static uint32_t autosaveInterval = 0;
static uint32_t iconTimer = 0;
static uint64_t currentTimestamp = 0;
static uint64_t lastSaveTimestamp = GetUnixTimestamp();

static HOOK_ID autosaveGameStateUpdateHookId = 0;
static HOOK_ID autosaveGameStateDrawFinishHookId = 0;

// Used for saving through Autosaves and Pause Menu saves.
extern "C" int SavingEnhancements_GetSaveEntrance() {
    switch (gPlayState->sceneId) {
        // Woodfall Temple + Odolwa
        case SCENE_MITURIN:
        case SCENE_MITURIN_BS:
            return ENTRANCE(WOODFALL_TEMPLE, 0);
        // Snowhead Temple + Goht
        case SCENE_HAKUGIN:
        case SCENE_HAKUGIN_BS:
            return ENTRANCE(SNOWHEAD_TEMPLE, 0);
        // Great Bay Temple + Gyorg
        case SCENE_SEA:
        case SCENE_SEA_BS:
            return ENTRANCE(GREAT_BAY_TEMPLE, 0);
        // Stone Tower Temple (+ inverted) + Twinmold
        case SCENE_INISIE_N:
        case SCENE_INISIE_R:
        case SCENE_INISIE_BS:
            return ENTRANCE(STONE_TOWER_TEMPLE, 0);
        default:
            return ENTRANCE(SOUTH_CLOCK_TOWN, 0);
    }
}

extern "C" bool SavingEnhancements_CanSave() {
    // Game State
    if (gPlayState == NULL || GET_PLAYER(gPlayState) == NULL) {
        return false;
    }

    // Owl saving available
    if (!gSaveContext.flashSaveAvailable || gSaveContext.fileNum == 255) {
        return false;
    }

    // Not in a blocking cutscene
    if (Player_InBlockingCsMode(gPlayState, GET_PLAYER(gPlayState))) {
        return false;
    }

    // Not in the middle of dialog
    if (gPlayState->msgCtx.msgMode != 0) {
        return false;
    }

    // Hasn't gotten to clock town yet
    if (gPlayState->sceneId == SCENE_SPOT00 || gPlayState->sceneId == SCENE_LOST_WOODS ||
        gPlayState->sceneId == SCENE_OPENINGDAN) {
        return false;
    }

    // Can't save once you've gone to the moon
    if (gPlayState->sceneId == SCENE_SOUGEN || gPlayState->sceneId == SCENE_LAST_LINK ||
        gPlayState->sceneId == SCENE_LAST_DEKU || gPlayState->sceneId == SCENE_LAST_GORON ||
        gPlayState->sceneId == SCENE_LAST_ZORA || gPlayState->sceneId == SCENE_LAST_BS) {
        return false;
    }

    // Not in minigames that set temporary flags
    if (CHECK_WEEKEVENTREG(WEEKEVENTREG_08_01) || CHECK_WEEKEVENTREG(WEEKEVENTREG_82_08) ||
        CHECK_WEEKEVENTREG(WEEKEVENTREG_90_20) || CHECK_WEEKEVENTREG(WEEKEVENTREG_KICKOUT_WAIT) ||
        CHECK_EVENTINF(EVENTINF_34) || CHECK_EVENTINF(EVENTINF_41)) {
        return false;
    }

    return true;
}

void DeleteOwlSave() {
    // Remove Owl Save on time cycle reset, needed when persisting owl saves and/or when
    // creating owl saves without the player being send back to the file select screen.

    // Delete Owl Save
    func_80147314(&gPlayState->sramCtx, gSaveContext.fileNum);

    // Set it to not be an owl save so after reloading the save file it doesn't try to load at the owl's position in
    // clock town
    gSaveContext.save.isOwlSave = false;
}

void DrawAutosaveIcon() {
    // 5 seconds (100 frames) of showing the owl save icon to signify autosave has happened.
    if (iconTimer != 0) {
        float opacity = 255.0;
        // Fade in icon
        if (iconTimer > 80) {
            opacity = 255.0 - (((iconTimer - 80.0) / 20.0) * 255);
            // Fade out icon
        } else if (iconTimer < 20) {
            opacity = (iconTimer / 20.0) * 255.0;
        }
        Interface_DrawAutosaveIcon(gPlayState, uint16_t(opacity));
        iconTimer--;
    }
}

void HandleAutoSave() {
    // Check if the interval has passed in minutes.
    autosaveInterval = CVarGetInteger("gEnhancements.Saving.AutosaveInterval", 5) * 60000;
    currentTimestamp = GetUnixTimestamp();
    if ((currentTimestamp - lastSaveTimestamp) < autosaveInterval) {
        return;
    }

    Player* player = GET_PLAYER(gPlayState);
    if (player == NULL) {
        return;
    }

    // If owl save available to create, do it and reset the interval.
    if (SavingEnhancements_CanSave() && gPlayState->pauseCtx.state == 0) {

        // Reset timestamp, set icon timer to show autosave icon for 5 seconds (100 frames)
        lastSaveTimestamp = GetUnixTimestamp();
        iconTimer = 100;

        // Create owl save
        gSaveContext.save.isOwlSave = true;
        gSaveContext.save.shipSaveInfo.pauseSaveEntrance = SavingEnhancements_GetSaveEntrance();
        Play_SaveCycleSceneFlags(&gPlayState->state);
        gSaveContext.save.saveInfo.playerData.savedSceneId = gPlayState->sceneId;
        func_8014546C(&gPlayState->sramCtx);
        Sram_SetFlashPagesOwlSave(&gPlayState->sramCtx, gFlashOwlSaveStartPages[gSaveContext.fileNum * 2],
                                  gFlashOwlSaveNumPages[gSaveContext.fileNum * 2]);
        Sram_StartWriteToFlashOwlSave(&gPlayState->sramCtx);
        gSaveContext.save.isOwlSave = false;
        gSaveContext.save.shipSaveInfo.pauseSaveEntrance = -1;
    }
}

void RegisterSavingEnhancements() {
    REGISTER_VB_SHOULD(VB_DELETE_OWL_SAVE, {
        if (CVarGetInteger("gEnhancements.Saving.PersistentOwlSaves", 0) ||
            gSaveContext.save.shipSaveInfo.pauseSaveEntrance != -1) {
            *should = false;
        }
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() { DeleteOwlSave(); });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeMoonCrashSaveReset>([]() { DeleteOwlSave(); });
}

void RegisterAutosave() {
    if (autosaveGameStateUpdateHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateUpdate>(autosaveGameStateUpdateHookId);
        autosaveGameStateUpdateHookId = 0;
    }

    if (autosaveGameStateDrawFinishHookId) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnGameStateDrawFinish>(
            autosaveGameStateDrawFinishHookId);
        autosaveGameStateDrawFinishHookId = 0;
    }

    if (CVarGetInteger("gEnhancements.Saving.Autosave", 0)) {
        autosaveGameStateUpdateHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateUpdate>([]() {
                if (gPlayState == nullptr) {
                    return;
                }

                HandleAutoSave();
            });

        autosaveGameStateDrawFinishHookId =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnGameStateDrawFinish>([]() {
                if (gPlayState == nullptr) {
                    return;
                }

                DrawAutosaveIcon();
            });
    }
}
