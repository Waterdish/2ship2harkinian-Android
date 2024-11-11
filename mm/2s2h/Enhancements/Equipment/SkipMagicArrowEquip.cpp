#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "src/overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
extern s16 sEquipState;
extern s16 sEquipAnimTimer;
}

void RegisterSkipMagicArrowEquip() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnKaleidoUpdate>([](PauseContext* pauseCtx) {
        if (CVarGetInteger("gEnhancements.Equipment.MagicArrowEquipSpeed", 0)) {
            if (sEquipState == EQUIP_STATE_MAGIC_ARROW_GROW_ORB) {
                sEquipState = EQUIP_STATE_MAGIC_ARROW_MOVE_TO_BOW_SLOT;
                sEquipAnimTimer = 2;
                pauseCtx->equipAnimAlpha = 255;
            }
        }
    });
}
