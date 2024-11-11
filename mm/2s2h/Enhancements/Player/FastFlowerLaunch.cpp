#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "macros.h"
#include "z64.h"
extern PlayState* gPlayState;
extern Input* sPlayerControlInput;
void Player_Action_93(Player* player, PlayState* play);
}

void RegisterFastFlowerLaunch() {
    static HOOK_ID playerUpdateHook = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(playerUpdateHook);
    playerUpdateHook = 0;

    if (!CVarGetInteger("gEnhancements.Player.FastFlowerLaunch", 0)) {
        return;
    }

    playerUpdateHook =
        GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(ACTOR_PLAYER, [](Actor* actor) {
            Player* player = GET_PLAYER(gPlayState);
            if (player->actionFunc != Player_Action_93) {
                return;
            }

            DynaPolyActor* dyna;
            Input* input = &gPlayState->state.input[0];

            if (player->av1.actionVar1 != 0 && !(player->av1.actionVar1 == 1 && player->unk_B48 > -170.0f) &&
                player->av2.actionVar2 != 10 && !CHECK_BTN_ALL(input->cur.button, BTN_A)) {
                player->unk_ABC = -3900.0f;
                player->unk_B48 = -170.0f;
                player->av1.actionVar1 = 2;
                player->av2.actionVar2 = 10;
                player->actor.scale.y = 0.01f;
                dyna = DynaPoly_GetActor(&gPlayState->colCtx, player->actor.floorBgId);

                if (dyna != NULL) {
                    player->actor.world.pos.x = dyna->actor.world.pos.x;
                    player->actor.world.pos.z = dyna->actor.world.pos.z;
                }
            }
        });
}
