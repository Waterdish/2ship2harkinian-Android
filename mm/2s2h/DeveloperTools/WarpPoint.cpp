#include "WarpPoint.h"

#include <libultraship/libultraship.h>
#include "2s2h/BenGui/UIWidgets.hpp"
#include "window/gui/IconsFontAwesome4.h"

extern "C" {
#include "z64.h"
#include "macros.h"
#include "functions.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

#define CV "gDeveloperTools.WarpPoint."

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

void RenderWarpPointSection() {
    if (gPlayState == NULL)
        return;

    if (UIWidgets::Button("Set Warp Point")) {
        Player* player = GET_PLAYER(gPlayState);

        CVarSetInteger(CV "Entrance", gSaveContext.save.entrance);
        CVarSetInteger(CV "Room", gPlayState->roomCtx.curRoom.num);
        CVarSetFloat(CV "X", player->actor.world.pos.x);
        CVarSetFloat(CV "Y", player->actor.world.pos.y);
        CVarSetFloat(CV "Z", player->actor.world.pos.z);
        CVarSetFloat(CV "Rotation", player->actor.shape.rot.y);
        CVarSetInteger(CV "Saved", 1);
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
    }
    if (CVarGetInteger(CV "Saved", 0)) {
        u32 sceneId = Entrance_GetSceneIdAbsolute(CVarGetInteger(CV "Entrance", ENTRANCE(SOUTH_CLOCK_TOWN, 0)));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s Room %d", warpPointSceneList[sceneId], CVarGetInteger(CV "Room", 0));
        ImGui::SameLine();
        if (UIWidgets::Button(ICON_FA_TIMES, { .size = UIWidgets::Sizes::Inline })) {
            CVarClear(CV "Entrance");
            CVarClear(CV "Room");
            CVarClear(CV "X");
            CVarClear(CV "Y");
            CVarClear(CV "Z");
            CVarClear(CV "Rotation");
            CVarClear(CV "Saved");
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
        }
        ImGui::SameLine();
        if (UIWidgets::Button("Warp", { .size = UIWidgets::Sizes::Inline })) {
            Vec3f pos = { CVarGetFloat(CV "X", 0.0f), CVarGetFloat(CV "Y", 0.0f), CVarGetFloat(CV "Z", 0.0f) };

            Play_SetRespawnData(&gPlayState->state, RESPAWN_MODE_DOWN,
                                CVarGetInteger(CV "Entrance", ENTRANCE(SOUTH_CLOCK_TOWN, 0)),
                                CVarGetInteger(CV "Room", 0), PLAYER_PARAMS(0xFF, PLAYER_INITMODE_D), &pos,
                                CVarGetFloat(CV "Rotation", 0.0f));
            func_80169EFC(&gPlayState->state);
            gSaveContext.respawnFlag = -8;
        }
    }
}
