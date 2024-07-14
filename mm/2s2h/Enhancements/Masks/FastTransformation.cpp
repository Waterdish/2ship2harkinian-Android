#include <libultraship/bridge.h>
#include "Enhancements/GameInteractor/GameInteractor.h"

extern "C" {
#include <z64.h>
#include <macros.h>
extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern ActorOverlay gActorOverlayTable[ACTOR_ID_MAX];
extern s16 gPlayerFormObjectIds[PLAYER_FORM_MAX];
void Player_PlaySfx(Player* player, u16 sfxId);
void PlayerCall_Init(Actor* thisx, PlayState* play);
void PlayerCall_Update(Actor* thisx, PlayState* play);
void PlayerCall_Draw(Actor* thisx, PlayState* play);
void TransitionFade_SetColor(void* thisx, u32 color);
}

void RegisterFastTransformation() {
    REGISTER_VB_SHOULD(GI_VB_PREVENT_MASK_TRANSFORMATION_CS, {
        if (CVarGetInteger("gEnhancements.Masks.FastTransformation", 0)) {
            *should = true;
            Player* player = GET_PLAYER(gPlayState);

            // This was mostly copied directly from func_8012301C within z_player_lib.c
            s16 objectId = gPlayerFormObjectIds[GET_PLAYER_FORM];

            gActorOverlayTable[ACTOR_PLAYER].initInfo->objectId = objectId;
            func_8012F73C(&gPlayState->objectCtx, player->actor.objectSlot, objectId);
            player->actor.objectSlot = Object_GetSlot(&gPlayState->objectCtx, GAMEPLAY_KEEP);

            s32 objectSlot =
                Object_GetSlot(&gPlayState->objectCtx, gActorOverlayTable[ACTOR_PLAYER].initInfo->objectId);
            player->actor.objectSlot = objectSlot;
            player->actor.shape.rot.z = GET_PLAYER_FORM + 1;
            player->actor.init = PlayerCall_Init;
            player->actor.update = PlayerCall_Update;
            player->actor.draw = PlayerCall_Draw;
            gSaveContext.save.equippedMask = PLAYER_MASK_NONE;

            TransitionFade_SetColor(&gPlayState->unk_18E48, 0x000000);
            R_TRANS_FADE_FLASH_ALPHA_STEP = -1;
            Player_PlaySfx(GET_PLAYER(gPlayState), NA_SE_SY_TRANSFORM_MASK_FLASH);

            // Clear previous mask to prevent crashing with masks being drawn while we switch transformations
            if (player->transformation == PLAYER_FORM_HUMAN) {
                player->prevMask = PLAYER_MASK_NONE;
            }
        }
    });
}
