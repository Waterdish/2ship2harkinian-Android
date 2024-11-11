#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "functions.h"
#include "src/overlays/actors/ovl_En_Kakasi/z_en_kakasi.h"
extern PlayState* gPlayState;
}

void RegisterSkipScarecrowSong() {
    REGISTER_VB_SHOULD(VB_NEED_SCARECROW_SONG, {
        if (CVarGetInteger("gEnhancements.Playback.SkipScarecrowSong", 0)) {
            EnKakasi* enKakasi = va_arg(args, EnKakasi*);
            /*
             * This is somewhat similar to the condition that the scarecrow normally checks, except it checks if the
             * instrument is being played at all instead of having played the Scarecrow's Song in particular, and it
             * bypasses the check that Link has taught Pierre a song this cycle.
             */
            if ((enKakasi->picto.actor.xzDistToPlayer < enKakasi->songSummonDist) &&
                ((BREG(1) != 0) || (gPlayState->msgCtx.ocarinaMode == OCARINA_MODE_ACTIVE))) {
                *should = true;
                // Properly get out of the ocarina playing state
                AudioOcarina_SetInstrument(OCARINA_INSTRUMENT_OFF);
                Message_CloseTextbox(gPlayState);
            }
        }
    });
}
