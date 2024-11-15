#include <libultraship/libultraship.h>
#include "2s2h/BenGui/UIWidgets.hpp"
#include "window/gui/IconsFontAwesome4.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
#include "overlays/gamestates/ovl_select/z_select.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern GameState* gGameState;
}

// 2S2H Added columns to scene table: entranceSceneId, betterMapSelectIndex, humanName
#define DEFINE_SCENE(_name, enumValue, _textId, _drawConfig, _restrictionFlags, _persistentCycleFlags, \
                     _entranceSceneId, _betterMapSelectIndex, humanName)                               \
    { enumValue, humanName },
#define DEFINE_SCENE_UNSET(_enumValue)

std::unordered_map<s16, const char*> warpPointSceneList = {
#include "tables/scene_table.h"
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

void Warp() {
    Vec3f pos = { CVarGetFloat(WARP_POINT_CVAR "X", 0.0f), CVarGetFloat(WARP_POINT_CVAR "Y", 0.0f),
                  CVarGetFloat(WARP_POINT_CVAR "Z", 0.0f) };
    s32 entrance = CVarGetInteger(WARP_POINT_CVAR "Entrance", ENTRANCE(SOUTH_CLOCK_TOWN, 0));

    if (gPlayState == NULL) {
        // If gPlayState is NULL, it means the the user opted into BootToWarpPoint and the game is starting up. This is
        // a hidden cvar for developers, while it is extremely useful for quick testing and debugging, I am not 100%
        // confident in it's stability and ability to initialize the game properly in all cases. So for now, I'm going
        // to leave it as a hidden cvar. This is incompatible with the SkipToFileSelect enhancement. To enable it open
        // the Console and type: `set gDeveloperTools.WarpPoint.BootToWarpPoint 1`
        gSaveContext.gameMode = GAMEMODE_NORMAL;
        Sram_InitNewSave();
        gSaveContext.sceneLayer = 0;
        gSaveContext.save.time = CLOCK_TIME(8, 0);
        gSaveContext.save.day = 1;
        gSaveContext.save.cutsceneIndex = 0;
        gSaveContext.save.playerForm = PLAYER_FORM_HUMAN;
        gSaveContext.save.linkAge = 0;
        gSaveContext.fileNum = 0xFF;
        MapSelect_LoadGame((MapSelectState*)gGameState, CVarGetInteger(WARP_POINT_CVAR "Entrance", 0), 0);
    } else {
        // The else case, and the rest of this function is primarily relevant code copied from Play_SetRespawnData and
        // func_80169EFC, minus the parts that copy scene flags to scene we are warping to (this is obviously
        // undesirable)
        gPlayState->nextEntrance = Entrance_Create(entrance >> 9, 0, entrance & 0xF);
        gPlayState->transitionTrigger = TRANS_TRIGGER_START;
        gPlayState->transitionType = TRANS_TYPE_INSTANT;
    }
    gSaveContext.respawn[RESPAWN_MODE_DOWN].entrance = Entrance_Create(entrance >> 9, 0, entrance & 0xF);
    gSaveContext.respawn[RESPAWN_MODE_DOWN].roomIndex = CVarGetInteger(WARP_POINT_CVAR "Room", 0);
    gSaveContext.respawn[RESPAWN_MODE_DOWN].pos = pos;
    gSaveContext.respawn[RESPAWN_MODE_DOWN].yaw = CVarGetFloat(WARP_POINT_CVAR "Rotation", 0.0f);
    gSaveContext.respawn[RESPAWN_MODE_DOWN].playerParams = PLAYER_PARAMS(0xFF, PLAYER_INITMODE_D);
    gSaveContext.nextTransitionType = TRANS_TYPE_FADE_BLACK_FAST;
    gSaveContext.respawnFlag = -8;
}

void RegisterWarpPoint() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnConsoleLogoUpdate>([]() {
        if (!CVarGetInteger("gEnhancements.Cutscenes.SkipToFileSelect", 0) &&
            CVarGetInteger(WARP_POINT_CVAR "BootToWarpPoint", 0) && CVarGetInteger(WARP_POINT_CVAR "Saved", 0)) {
            // Normally called on console logo screen
            gSaveContext.seqId = (u8)NA_BGM_DISABLED;
            gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
            gSaveContext.gameMode = GAMEMODE_TITLE_SCREEN;
            Warp();
        }
    });
}

void RenderWarpPointSection() {
    if (gPlayState == NULL)
        return;

    if (UIWidgets::Button("Set Warp Point")) {
        Player* player = GET_PLAYER(gPlayState);

        CVarSetInteger(WARP_POINT_CVAR "Entrance", gSaveContext.save.entrance);
        CVarSetInteger(WARP_POINT_CVAR "Room", gPlayState->roomCtx.curRoom.num);
        CVarSetFloat(WARP_POINT_CVAR "X", player->actor.world.pos.x);
        CVarSetFloat(WARP_POINT_CVAR "Y", player->actor.world.pos.y);
        CVarSetFloat(WARP_POINT_CVAR "Z", player->actor.world.pos.z);
        CVarSetFloat(WARP_POINT_CVAR "Rotation", player->actor.shape.rot.y);
        CVarSetInteger(WARP_POINT_CVAR "Saved", 1);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
    }
    if (CVarGetInteger(WARP_POINT_CVAR "Saved", 0)) {
        u32 sceneId =
            Entrance_GetSceneIdAbsolute(CVarGetInteger(WARP_POINT_CVAR "Entrance", ENTRANCE(SOUTH_CLOCK_TOWN, 0)));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s Room %d", warpPointSceneList[sceneId], CVarGetInteger(WARP_POINT_CVAR "Room", 0));
        ImGui::SameLine();
        if (UIWidgets::Button(ICON_FA_TIMES, { .size = UIWidgets::Sizes::Inline })) {
            CVarClear(WARP_POINT_CVAR "Entrance");
            CVarClear(WARP_POINT_CVAR "Room");
            CVarClear(WARP_POINT_CVAR "X");
            CVarClear(WARP_POINT_CVAR "Y");
            CVarClear(WARP_POINT_CVAR "Z");
            CVarClear(WARP_POINT_CVAR "Rotation");
            CVarClear(WARP_POINT_CVAR "Saved");
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Warp", { .size = UIWidgets::Sizes::Inline })) {
            Warp();
        }
    }
}
