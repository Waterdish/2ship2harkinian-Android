#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

void RegisterClimbSpeed() {
    REGISTER_VB_SHOULD(VB_SET_CLIMB_SPEED, {
        if (CVarGetInteger("gEnhancements.Player.ClimbSpeed", 1) > 1) {
            f32* speed = va_arg(args, f32*);
            *speed *= CVarGetInteger("gEnhancements.Player.ClimbSpeed", 1);
        }
    });
}
