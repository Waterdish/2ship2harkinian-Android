#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "z64game.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"
#include "overlays/gamestates/ovl_title/z_title.h"
extern SaveContext gSaveContext;
extern GameState* gGameState;
}

void RegisterSkipToFileSelect() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnConsoleLogoUpdate>([]() {
        if (CVarGetInteger("gEnhancements.Cutscenes.SkipToFileSelect", 0)) {
            ConsoleLogoState* consoleLogoState = (ConsoleLogoState*)gGameState;

            // Wait for the console logo to fade out
            if (consoleLogoState->exit) {
                // Normally the PRNG seed is set at least once from the title opening running Play_Init
                // We need to call it manually before file select creates RNG values for new saves
                Rand_Seed(osGetTime());
                // Normally called on console logo screen
                gSaveContext.seqId = (u8)NA_BGM_DISABLED;
                gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
                gSaveContext.gameMode = GAMEMODE_TITLE_SCREEN;

                STOP_GAMESTATE(gGameState);
                SET_NEXT_GAMESTATE(gGameState, FileSelect_Init, sizeof(FileSelectState));
            }
        }
    });
}
