#include "libultraship/libultraship.h"
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "macros.h"
#include "z64.h"
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
}

static HOOK_ID onActorUpdateHookId = 0;

void RegisterTimeMovesWhenYouMove() {
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(onActorUpdateHookId);
    onActorUpdateHookId = 0;
    gSaveContext.save.timeSpeedOffset = 0;

    if (CVarGetInteger("gModes.TimeMovesWhenYouMove", 0)) {
        onActorUpdateHookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
            ACTOR_PLAYER, [](Actor* actor) {
                Player* player = GET_PLAYER(gPlayState);

                gSaveContext.save.timeSpeedOffset = -R_TIME_SPEED;
                if (player->linearVelocity != 0) {
                    gSaveContext.save.timeSpeedOffset =
                        CLAMP(player->linearVelocity / 2 - R_TIME_SPEED, -R_TIME_SPEED, 10);
                }
            });
    }
}
