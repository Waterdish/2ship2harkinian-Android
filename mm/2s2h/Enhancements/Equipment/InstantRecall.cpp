#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "src/overlays/actors/ovl_En_Boom/z_en_boom.h"
extern PlayState* gPlayState;
}

static HOOK_ID onActorUpdateHookId = 0;

void Player_ReturnBoomerangs() {
    Player* player = GET_PLAYER(gPlayState);

    if (player == NULL) {
        return;
    }

    EnBoom* boomerangs = (EnBoom*)player->boomerangActor;

    // Kill both boomerangs
    if (boomerangs != NULL) {
        Actor_Kill(&boomerangs->actor);
        if (boomerangs->actor.child != NULL) {
            Actor_Kill(boomerangs->actor.child);
        }
    }
}

void RegisterInstantRecall() {
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(onActorUpdateHookId);
    onActorUpdateHookId = 0;

    onActorUpdateHookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
        ACTOR_EN_BOOM, [](Actor* outerActor) {
            if (CVarGetInteger("gEnhancements.PlayerActions.InstantRecall", 0)) {
                if (CHECK_BTN_ALL(gPlayState->state.input->press.button, BTN_B)) {
                    Player_ReturnBoomerangs();
                }
            }
        });
}
