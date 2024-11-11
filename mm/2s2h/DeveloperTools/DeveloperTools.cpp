#include "DeveloperTools.h"
#include "BenPort.h"
#include <libultraship/libultraship.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "macros.h"
#include "z64actor.h"
#include "z64save.h"
#include "variables.h"
#include "overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"

void Sram_InitDebugSave(void);
void Flags_SetWeekEventReg(s32 flag);
void Inventory_ChangeUpgrade(s16 upgrade, u32 value);
void Inventory_SetWorldMapCloudVisibility(s16 tingleIndex);

extern u16 sPersistentCycleWeekEventRegs[ARRAY_COUNT(gSaveContext.save.saveInfo.weekEventReg)];
}

void RegisterDebugSaveCreate() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSaveInit>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.DebugSaveFileMode", DEBUG_SAVE_INFO_NONE) != DEBUG_SAVE_INFO_NONE) {
        hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSaveInit>([](s16 fileNum) {
            u8 playerName[8];

            // Copy player name and set back after debug save init
            memcpy(playerName, gSaveContext.save.saveInfo.playerData.playerName, sizeof(playerName));

            Sram_InitDebugSave();

            memcpy(gSaveContext.save.saveInfo.playerData.playerName, playerName, sizeof(playerName));

            // Place link at entrance from clock tower
            gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
            gSaveContext.save.cutsceneIndex = 0;
            gSaveContext.save.saveInfo.checksum = 1;

            // Prevent first Song of Time reset from forcing Deku Link and having to learn Song of Healing
            gSaveContext.save.saveInfo.playerData.threeDayResetCount = 1;

            if (CVarGetInteger("gDeveloperTools.DebugSaveFileMode", DEBUG_SAVE_INFO_NONE) == DEBUG_SAVE_INFO_COMPLETE) {
                gSaveContext.save.saveInfo.playerData.doubleDefense = true;
                gSaveContext.save.saveInfo.playerData.health = 20 * 0x10;
                gSaveContext.save.saveInfo.playerData.healthCapacity = 20 * 0x10;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
                gSaveContext.save.saveInfo.playerData.magicLevel = 2;
                gSaveContext.save.saveInfo.playerData.magic = MAGIC_DOUBLE_METER;
                gSaveContext.save.saveInfo.playerData.owlActivationFlags = (1 << (OWL_WARP_STONE_TOWER + 1)) - 1;
                gSaveContext.save.saveInfo.playerData.unk_20 = OWL_WARP_CLOCK_TOWN;

                gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
                gSaveContext.save.saveInfo.regionsVisited = (1 << REGION_MAX) - 1;
                gSaveContext.magicCapacity = MAGIC_DOUBLE_METER;

                Inventory_ChangeUpgrade(UPG_WALLET, 2);
                Inventory_ChangeUpgrade(UPG_BOMB_BAG, 3);
                Inventory_ChangeUpgrade(UPG_QUIVER, 3);

                for (int32_t i = 0; i < TINGLE_MAP_MAX; i++) {
                    Inventory_SetWorldMapCloudVisibility(i);
                }

                gSaveContext.save.saveInfo.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
                AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
                AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
                AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
                AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
                AMMO(ITEM_MAGIC_BEANS) = 20;
                AMMO(ITEM_POWDER_KEG) = 1;

                SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_MIRROR);
                SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_GILDED);
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) = ITEM_SWORD_GILDED;

                for (int32_t i = SLOT_BOTTLE_1; i <= SLOT_BOTTLE_6; i++) {
                    if (gSaveContext.save.saveInfo.inventory.items[i] == ITEM_NONE) {
                        gSaveContext.save.saveInfo.inventory.items[i] = ITEM_BOTTLE;
                    }
                }

                for (int32_t i = QUEST_REMAINS_ODOLWA; i <= QUEST_BOMBERS_NOTEBOOK; i++) {
                    if (i != QUEST_SHIELD && i != QUEST_SWORD && i != QUEST_SONG_SARIA && i != QUEST_SONG_SUN) {
                        SET_QUEST_ITEM(i);
                    }
                }

                // Use the persistent cycle events to set what a 100% save would normally keep
                for (int32_t i = 0; i < ARRAY_COUNT(sPersistentCycleWeekEventRegs); i++) {
                    u16 isPersistentBits = sPersistentCycleWeekEventRegs[i];

                    // Force all bits on
                    gSaveContext.save.saveInfo.weekEventReg[i] = 0xFF;

                    // Then unset any bits that aren't persistent
                    for (int32_t j = 0; j < 8; j++) {
                        if (!(isPersistentBits & 3)) {
                            gSaveContext.save.saveInfo.weekEventReg[i] =
                                gSaveContext.save.saveInfo.weekEventReg[i] & (0xFF ^ (1 << j));
                        }
                        isPersistentBits >>= 2;
                    }
                }
            }
        });
    }
}

void RegisterPreventActorUpdateHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::ShouldActorUpdate>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorUpdate", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::ShouldActorUpdate>(
            [](Actor* actor, bool* result) { *result = false; });
    }
}

void RegisterPreventActorDrawHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::ShouldActorDraw>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorDraw", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::ShouldActorDraw>(
            [](Actor* actor, bool* result) { *result = false; });
    }
}

void RegisterPreventActorInitHooks() {
    static HOOK_ID hookId = 0;
    if (hookId != 0) {
        GameInteractor::Instance->UnregisterGameHookForFilter<GameInteractor::ShouldActorInit>(hookId);
        hookId = 0;
    }

    if (CVarGetInteger("gDeveloperTools.PreventActorInit", 0)) {
        hookId = GameInteractor::Instance->RegisterGameHookForFilter<GameInteractor::ShouldActorInit>(
            GameInteractor::HookFilter::SActorNotPlayer, [](Actor* actor, bool* result) { *result = false; });
    }
}

void InitDeveloperTools() {
    RegisterDebugSaveCreate();
    RegisterPreventActorUpdateHooks();
    RegisterPreventActorDrawHooks();
    RegisterPreventActorInitHooks();
    RegisterWarpPoint();
}
