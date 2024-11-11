#include "2s2h/GameInteractor/GameInteractor.h"
#include "libultraship/libultraship.h"

extern "C" {
#include "variables.h"

extern u8 sPlaybackState;
}

void RegisterFasterSongPlayback() {
    static uint32_t onPlayerUpdate = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(onPlayerUpdate);

    onPlayerUpdate = 0;

    if (!CVarGetInteger("gEnhancements.Songs.FasterSongPlayback", 0)) {
        return;
    }

    onPlayerUpdate =
        GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(ACTOR_PLAYER, [](Actor* actor) {
            if (gPlayState->msgCtx.msgMode >= MSGMODE_SONG_PLAYED && gPlayState->msgCtx.msgMode <= MSGMODE_17) {
                if (gPlayState->msgCtx.stateTimer > 1) {
                    gPlayState->msgCtx.stateTimer = 1;
                }
                gPlayState->msgCtx.ocarinaSongEffectActive = 0;
                sPlaybackState = 0;
            }
        });
}
