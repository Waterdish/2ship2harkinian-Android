#include "libultraship/libultraship.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "z64.h"
#include "functions.h"
#include "overlays/actors/ovl_En_Slime/z_en_slime.h"
extern PlayState* gPlayState;
void EnItem00_Draw(Actor* thisx, PlayState* play);
}

bool ItemShouldSpinWhen3D(Actor* actor) {
    EnItem00* enItem00 = (EnItem00*)actor;

    // Exclude actors that already have spin normally, or shouldn't spin ever
    if ((actor->params <= ITEM00_RUPEE_RED) || ((actor->params == ITEM00_RECOVERY_HEART) && (enItem00->unk152 < 0)) ||
        (actor->params == ITEM00_HEART_PIECE) || (actor->params == ITEM00_HEART_CONTAINER) ||
        (actor->params == ITEM00_RUPEE_HUGE) || (actor->params == ITEM00_RUPEE_PURPLE) ||
        (actor->params == ITEM00_SHIELD_HERO) || (actor->params == ITEM00_MAP) || (actor->params == ITEM00_COMPASS)) {
        return false;
    }

    return true;
}

void EnItem00_3DItemsDraw(Actor* actor, PlayState* play) {
    EnItem00* enItem00 = (EnItem00*)actor;

    if (!(enItem00->unk14E & enItem00->unk150)) {
        Player* player = GET_PLAYER(play);
        bool itemOnPlayer = player->actor.home.pos.x == enItem00->actor.world.pos.x &&
                            player->actor.home.pos.z == enItem00->actor.world.pos.z;
        FrameInterpolation_RecordOpenChild(enItem00, itemOnPlayer ? 1 : 0);
        FrameInterpolation_IgnoreActorMtx();

        switch (enItem00->actor.params) {
            case ITEM00_RUPEE_GREEN:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_GREEN);
                break;
            case ITEM00_RUPEE_BLUE:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_BLUE);
                break;
            case ITEM00_RUPEE_RED:
                Matrix_Scale(25.0f, 25.0f, 25.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_RED);
                break;
            case ITEM00_RUPEE_HUGE:
                Matrix_Scale(17.5f, 17.5f, 17.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_HUGE);
                break;
            case ITEM00_RUPEE_PURPLE:
                Matrix_Scale(17.5f, 17.5f, 17.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RUPEE_PURPLE);
                break;
            case ITEM00_HEART_PIECE:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_HEART_PIECE);
                break;
            case ITEM00_HEART_CONTAINER:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_HEART_CONTAINER);
                break;
            case ITEM00_RECOVERY_HEART:
                Matrix_Scale(16.0f, 16.0f, 16.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_RECOVERY_HEART);
                break;
            case ITEM00_BOMBS_A:
            case ITEM00_BOMBS_B:
            case ITEM00_BOMBS_0:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_BOMB);
                break;
            case ITEM00_ARROWS_10:
                Matrix_Scale(7.0f, 7.0f, 7.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_ARROWS_SMALL);
                break;
            case ITEM00_ARROWS_30:
                Matrix_Scale(7.0f, 7.0f, 7.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_ARROWS_MEDIUM);
                break;
            case ITEM00_ARROWS_40:
            case ITEM00_ARROWS_50:
                Matrix_Scale(7.0f, 7.0f, 7.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_ARROWS_LARGE);
                break;
            case ITEM00_MAGIC_JAR_SMALL:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_MAGIC_JAR_SMALL);
                break;
            case ITEM00_MAGIC_JAR_BIG:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_MAGIC_JAR_BIG);
                break;
            case ITEM00_DEKU_STICK:
                Matrix_Scale(7.5f, 7.5f, 7.5f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_DEKU_STICK);
                break;
            case ITEM00_SMALL_KEY:
                Matrix_Scale(8.0f, 8.0f, 8.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_KEY_SMALL);
                break;
            case ITEM00_DEKU_NUTS_1:
            case ITEM00_DEKU_NUTS_10:
                Matrix_Scale(9.0f, 9.0f, 9.0f, MTXMODE_APPLY);
                GetItem_Draw(play, GID_DEKU_NUTS);
                break;
            case ITEM00_SHIELD_HERO:
                GetItem_Draw(play, GID_SHIELD_HERO);
                break;
            case ITEM00_MAP:
                GetItem_Draw(play, GID_DUNGEON_MAP);
                break;
            case ITEM00_COMPASS:
                GetItem_Draw(play, GID_COMPASS);
                break;
        }

        FrameInterpolation_RecordCloseChild();
    }
}

void DrawSlime3DItem(Actor* actor, bool* should) {
    *should = false;
    EnSlime* slime = (EnSlime*)actor;

    // Rotate 3D item with chu body
    Matrix_RotateYS(slime->actor.shape.rot.y, MTXMODE_APPLY);
    Matrix_Scale(0.25f, 0.25f, 0.25f, MTXMODE_APPLY);

    switch (slime->actor.params) {
        case EN_SLIME_TYPE_RED:
            GetItem_Draw(gPlayState, GID_RECOVERY_HEART);
            break;
        case EN_SLIME_TYPE_GREEN:
            GetItem_Draw(gPlayState, GID_MAGIC_JAR_SMALL);
            break;
        case EN_SLIME_TYPE_YELLOW:
            GetItem_Draw(gPlayState, GID_ARROWS_SMALL);
            break;
    }
}

void Register3DItemDrops() {
    static HOOK_ID actorInitHookID = 0;
    static HOOK_ID actorUpdateHookID = 0;
    static HOOK_ID slimeVBHookID = 0;
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorInit>(actorInitHookID);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnActorUpdate>(actorUpdateHookID);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(slimeVBHookID);
    actorInitHookID = 0;
    actorUpdateHookID = 0;
    slimeVBHookID = 0;

    if (gPlayState != NULL) {
        Actor* actor = gPlayState->actorCtx.actorLists[ACTORCAT_MISC].first;

        while (actor != NULL) {
            if (actor->id == ACTOR_EN_ITEM00) {
                if (CVarGetInteger("gEnhancements.Graphics.3DItemDrops", 0)) {
                    actor->draw = EnItem00_3DItemsDraw;
                } else {
                    actor->draw = EnItem00_Draw;

                    // Reset rotation for bill-boarded sprites
                    if (ItemShouldSpinWhen3D(actor)) {
                        actor->shape.rot.y = 0;
                    }
                }
            }

            actor = actor->next;
        }
    }

    if (!CVarGetInteger("gEnhancements.Graphics.3DItemDrops", 0)) {
        return;
    }

    actorInitHookID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_EN_ITEM00, [](Actor* actor) { actor->draw = EnItem00_3DItemsDraw; });
    actorUpdateHookID = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorUpdate>(
        ACTOR_EN_ITEM00, [](Actor* actor) {
            // Add spin to normally bill-boarded items
            if (ItemShouldSpinWhen3D(actor)) {
                actor->shape.rot.y += 0x3C0;
            }
        });
    slimeVBHookID = REGISTER_VB_SHOULD(VB_DRAW_SLIME_BODY_ITEM, {
        Actor* actor = va_arg(args, Actor*);
        DrawSlime3DItem(actor, should);
    });
}
