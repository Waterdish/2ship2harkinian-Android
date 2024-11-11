#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

static uint32_t bombType;

extern "C" {
#include "global.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
EquipSlot func_8082FDC4(void);
}

void RegisterBlastMaskKeg() {
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldActorInit>(
        ACTOR_EN_BOM, [](Actor* actor, bool* should) {
            Player* player = GET_PLAYER(gPlayState);
            if (CVarGetInteger("gEnhancements.Masks.BlastMaskKeg", 0)) {
                ItemId item;
                EquipSlot i = func_8082FDC4();

                i = ((i >= EQUIP_SLOT_A) && (player->transformation == PLAYER_FORM_FIERCE_DEITY) &&
                     (player->heldItemAction != PLAYER_IA_SWORD_TWO_HANDED))
                        ? EQUIP_SLOT_B
                        : i;

                item = Player_GetItemOnButton(gPlayState, player, i);
                if (item == ITEM_F0) {
                    actor->shape.rot.x = BOMB_EXPLOSIVE_TYPE_POWDER_KEG;
                }
            }
        });
}
