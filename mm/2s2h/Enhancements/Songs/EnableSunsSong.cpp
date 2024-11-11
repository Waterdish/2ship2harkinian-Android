#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include <z64ocarina.h>
}

void RegisterEnableSunsSong() {
    REGISTER_VB_SHOULD(VB_SONG_AVAILABLE_TO_PLAY, {
        uint8_t* songIndex = va_arg(args, uint8_t*);
        // If the enhancement is on, and the currently played song is Sun's Song, set it to be available to be played.
        if (CVarGetInteger("gEnhancements.Songs.EnableSunsSong", 0) && *songIndex == OCARINA_SONG_SUNS) {
            *should = true;
        }
    });
}
