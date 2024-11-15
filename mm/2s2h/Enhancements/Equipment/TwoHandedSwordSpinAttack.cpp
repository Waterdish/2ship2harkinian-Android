#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "overlays/actors/ovl_En_M_Thunder/z_en_m_thunder.h"

extern "C" {
extern PlayState* gPlayState;
extern Input* sPlayerControlInput;
}

void RegisterTwoHandedSwordSpinAttack() {

    REGISTER_VB_SHOULD(VB_CHECK_HELD_ITEM_BUTTON_PRESS, {
        if (CVarGetInteger("gEnhancements.Equipment.TwoHandedSwordSpinAttack", 0)) {
            // Instead of checking B only, check whichever button is assigned to the currently held item. This allows
            // the Great Fairy Sword, a C button item, to be held for charged spin attacks.
            Player* player = GET_PLAYER(gPlayState);
            u16* sDpadItemButtons = va_arg(args, u16*);
            u16* sPlayerItemButtons = va_arg(args, u16*);

            if (player->heldItemButton < 0) {
                *should = false;
            } else {
                uint16_t buttonToCheck;
                if (IS_HELD_DPAD(player->heldItemButton)) {
                    buttonToCheck = sDpadItemButtons[HELD_ITEM_TO_DPAD(player->heldItemButton)];
                } else {
                    buttonToCheck = sPlayerItemButtons[player->heldItemButton];
                }
                *should = CHECK_BTN_ALL(sPlayerControlInput->cur.button, buttonToCheck);
            }
        }
    });

    REGISTER_VB_SHOULD(VB_MAGIC_SPIN_ATTACK_CHECK_FORM, {
        if (CVarGetInteger("gEnhancements.Equipment.TwoHandedSwordSpinAttack", 0)) {
            // Additionally allow the Fierce Deity form to use charged spin attacks
            Player* player = GET_PLAYER(gPlayState);
            if (player->transformation == PLAYER_FORM_FIERCE_DEITY) {
                *should = true;
            }
        }
    });

    REGISTER_VB_SHOULD(VB_TRANSFORM_THUNDER_MATRIX, {
        if (CVarGetInteger("gEnhancements.Equipment.TwoHandedSwordSpinAttack", 0)) {
            EnMThunder* enMThunder = va_arg(args, EnMThunder*);
            // Define new thunder matrix transformation for two-handed sword magic. Applies to both the GFS and the
            // Fierce Deity sword.
            if (enMThunder->type == ENMTHUNDER_TYPE_GREAT_FAIRYS_SWORD) {
                Matrix_Translate(0.0f, 220.0f, 0.0f, MTXMODE_APPLY);
                Matrix_Scale(-1.7f, -1.0f, -0.6f, MTXMODE_APPLY);
                Matrix_RotateXS(0x4000, MTXMODE_APPLY);
                *should = false;
            }
        }
    });
}
