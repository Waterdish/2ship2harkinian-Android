#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "variables.h"
}

void RegisterFierceDeityZTargetMovement() {
    REGISTER_VB_SHOULD(VB_ZTARGET_SPEED_CHECK, {
        Player* player = GET_PLAYER(gPlayState);
        float* speedArg = va_arg(args, float*);

        // If the player is Fierce Deity and targeting,
        if (player->lockOnActor != NULL && player->transformation == PLAYER_FORM_FIERCE_DEITY &&
            CVarGetInteger("gEnhancements.Fixes.FierceDeityZTargetMovement", 0)) {
            // 6.0f is the maximum speed of Zora/Goron/Deku link, whereas FD can be up to 10
            // Clamping to 6.0 keeps z target movement similar to other transformations
            *speedArg = CLAMP_MAX(*speedArg, 6.0f);
        }
    });
}
