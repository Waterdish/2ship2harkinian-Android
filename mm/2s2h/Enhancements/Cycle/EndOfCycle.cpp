#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include <variables.h>
#include <overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h>
}

SaveInfo saveInfoCopy;
ShipSaveInfo shipSaveInfoCopy;
s32 timeSpeedOffsetCopy = 0;

void RegisterEndOfCycleSaveHooks() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::BeforeEndOfCycleSave>([]() {
        memcpy(&saveInfoCopy, &gSaveContext.save.saveInfo, sizeof(SaveInfo));
        memcpy(&shipSaveInfoCopy, &gSaveContext.save.shipSaveInfo, sizeof(ShipSaveInfo));
        timeSpeedOffsetCopy = gSaveContext.save.timeSpeedOffset;
    });

    GameInteractor::Instance->RegisterGameHook<GameInteractor::AfterEndOfCycleSave>([]() {
        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRupees", 0)) {
            gSaveContext.save.saveInfo.playerData.rupees = saveInfoCopy.playerData.rupees;
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetConsumables", 0)) {
            if (INV_CONTENT(ITEM_BOMB) == ITEM_BOMB) {
                INV_CONTENT(ITEM_BOMB) = saveInfoCopy.inventory.items[ITEM_BOMB];
                AMMO(ITEM_BOMB) = saveInfoCopy.inventory.ammo[ITEM_BOMB];
            }

            if (INV_CONTENT(ITEM_DEKU_NUT) == ITEM_DEKU_NUT) {
                INV_CONTENT(ITEM_DEKU_NUT) = saveInfoCopy.inventory.items[ITEM_DEKU_NUT];
                AMMO(ITEM_DEKU_NUT) = saveInfoCopy.inventory.ammo[ITEM_DEKU_NUT];
            }

            if (INV_CONTENT(ITEM_DEKU_STICK) == ITEM_DEKU_STICK) {
                INV_CONTENT(ITEM_DEKU_STICK) = saveInfoCopy.inventory.items[ITEM_DEKU_STICK];
                AMMO(ITEM_DEKU_STICK) = saveInfoCopy.inventory.ammo[ITEM_DEKU_STICK];
            }

            if (INV_CONTENT(ITEM_BOW) == ITEM_BOW) {
                INV_CONTENT(ITEM_BOW) = saveInfoCopy.inventory.items[ITEM_BOW];
                AMMO(ITEM_BOW) = saveInfoCopy.inventory.ammo[ITEM_BOW];
            }

            for (int i = 0; i < ITEM_NUM_SLOTS; i++) {
                if (gAmmoItems[i] != ITEM_NONE) {
                    if ((gSaveContext.save.saveInfo.inventory.items[i] != ITEM_NONE) && (i != SLOT_PICTOGRAPH_BOX)) {
                        gSaveContext.save.saveInfo.inventory.items[i] = saveInfoCopy.inventory.items[i];
                        gSaveContext.save.saveInfo.inventory.ammo[i] = saveInfoCopy.inventory.ammo[i];
                    }
                }
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetBottleContent", 0)) {
            int stolenBottles = (((saveInfoCopy.stolenItems & 0xFF000000) >> 0x18) == ITEM_BOTTLE) +
                                (((saveInfoCopy.stolenItems & 0x00FF0000) >> 0x10) == ITEM_BOTTLE);

            // Replace bottles back, accounting for any stolen bottles
            for (int i = SLOT_BOTTLE_1; i <= SLOT_BOTTLE_6; i++) {
                gSaveContext.save.saveInfo.inventory.items[i] = saveInfoCopy.inventory.items[i];

                if (stolenBottles > 0 && saveInfoCopy.inventory.items[i] == ITEM_NONE) {
                    stolenBottles--;
                    gSaveContext.save.saveInfo.inventory.items[i] = ITEM_BOTTLE;
                }
            }

            // Set back button equips to the correct bottle type
            for (int j = EQUIP_SLOT_C_LEFT; j <= EQUIP_SLOT_C_RIGHT; j++) {
                if (GET_CUR_FORM_BTN_ITEM(j) == ITEM_BOTTLE) {
                    SET_CUR_FORM_BTN_ITEM(j, saveInfoCopy.equips.buttonItems[0][j]);
                }
            }
            for (int j = EQUIP_SLOT_D_RIGHT; j <= EQUIP_SLOT_D_UP; j++) {
                if (DPAD_GET_CUR_FORM_BTN_ITEM(j) == ITEM_BOTTLE) {
                    DPAD_SET_CUR_FORM_BTN_ITEM(j, shipSaveInfoCopy.dpadEquips.dpadItems[0][j]);
                }
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetRazorSword", 0)) {
            u8 curSword =
                (saveInfoCopy.equips.equipment & gEquipMasks[EQUIP_TYPE_SWORD]) >> gEquipShifts[EQUIP_TYPE_SWORD];

            // Check for razor sword equipped, stolen, or turned into the smithy
            if (curSword == EQUIP_VALUE_SWORD_RAZOR ||
                (curSword == EQUIP_VALUE_SWORD_NONE &&
                 ((saveInfoCopy.permanentSceneFlags[SCENE_KAJIYA].unk_14 & 4) ||
                  (((saveInfoCopy.stolenItems & 0xFF000000) >> 0x18) == ITEM_SWORD_RAZOR) ||
                  (((saveInfoCopy.stolenItems & 0x00FF0000) >> 0x10) == ITEM_SWORD_RAZOR)))) {

                SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_RAZOR);
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_RAZOR;
            }
        }

        if (CVarGetInteger("gEnhancements.Cycle.DoNotResetTimeSpeed", 0)) {
            gSaveContext.save.timeSpeedOffset = timeSpeedOffsetCopy;
        }
    });
}
