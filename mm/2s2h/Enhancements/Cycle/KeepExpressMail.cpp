#include "libultraship/bridge.h"
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64save.h"
#include "z64scene.h"
#include "variables.h"
}

static HOOK_ID onItemDeleteID = 0;

void CheckScene(s16 sceneId) {
    if (sceneId != SCENE_MILK_BAR && sceneId != SCENE_POSTHOUSE) {
        return;
    }

    onItemDeleteID = REGISTER_VB_SHOULD(VB_MSG_SCRIPT_DEL_ITEM, {
        Actor* actor = va_arg(args, Actor*);
        ItemId itemId = (ItemId)va_arg(args, int);

        // Keep the express mail only on the first trade for the cycle
        // Postman checking for Madame Aroma trade cycle flag
        // Madame Aroma checking for Postman trade cycle flag
        if (itemId == ITEM_LETTER_MAMA && (actor->id == ACTOR_EN_PM && !CHECK_WEEKEVENTREG(WEEKEVENTREG_57_04)) ||
            (actor->id == ACTOR_EN_AL && !CHECK_WEEKEVENTREG(WEEKEVENTREG_86_01))) {
            *should = false;
        }
    });
}

void RegisterKeepExpressMail() {
    static HOOK_ID onSceneInitID = 0;

    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(onItemDeleteID);
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(onSceneInitID);
    onSceneInitID = 0;
    onItemDeleteID = 0;

    if (!CVarGetInteger("gEnhancements.Cycle.KeepExpressMail", 0)) {
        return;
    }

    // Register item hook right away after toggle
    if (gPlayState != nullptr) {
        CheckScene(gPlayState->sceneId);
    }

    onSceneInitID =
        GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s16 sceneId, s8 spawnNum) {
            GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(onItemDeleteID);
            onItemDeleteID = 0;

            CheckScene(sceneId);
        });
}
