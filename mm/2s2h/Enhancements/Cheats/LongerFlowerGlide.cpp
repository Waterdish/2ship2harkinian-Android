#include <libultraship/bridge.h>

extern "C" {
#include "z64.h";
extern f32 D_8085D958[2];
}

void RegisterLongerFlowerGlide() {
    if (CVarGetInteger("gCheats.LongerFlowerGlide", 0)) {
        D_8085D958[0] = 99999.9f;
        D_8085D958[1] = 99999.9f;
    } else {
        D_8085D958[0] = 600.0f;
        D_8085D958[1] = 960.0f;
    }
}
