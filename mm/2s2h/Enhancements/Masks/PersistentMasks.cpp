#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"

extern "C" {
#include "z64.h"
#include "z64player.h"
#include "functions.h"
#include "macros.h"
#include "gfx.h"
#include "src/overlays/kaleido_scope/ovl_kaleido_scope/z_kaleido_scope.h"
#include "assets/interface/parameter_static/parameter_static.h"

void func_8082E1F0(Player* player, u16 sfxId);
void Player_DrawBunnyHood(PlayState* play);
extern PlayState* gPlayState;
extern const char* D_801C0B20[28];
extern SaveContext gSaveContext;
}

void UpdatePersistentMasksState() {
    static Vtx* persistentMasksVtx;
    static HOOK_ID beforePageDrawHook = 0;
    static HOOK_ID onPlayerPostLimbDrawHook = 0;
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::BeforeKaleidoDrawPage>(beforePageDrawHook);
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(onPlayerPostLimbDrawHook);

    if (!CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0)) {
        CVarClear("gEnhancements.Masks.PersistentBunnyHood.State");
        return;
    }

    // If the mask is equipped, unequip it
    if (gSaveContext.save.equippedMask == PLAYER_MASK_BUNNY) {
        gSaveContext.save.equippedMask = PLAYER_MASK_NONE;
        CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 1);

        if (gPlayState != NULL) {
            Player* player = GET_PLAYER(gPlayState);
            player->prevMask = player->currentMask;
            player->currentMask = PLAYER_MASK_NONE;
        }
    }

    // If they don't have the mask, clear the state
    if (INV_CONTENT(ITEM_MASK_BUNNY) != ITEM_MASK_BUNNY) {
        CVarClear("gEnhancements.Masks.PersistentBunnyHood.State");
    }

    // This hook draws the mask on the players head when it's active and they aren't in first person
    onPlayerPostLimbDrawHook = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnPlayerPostLimbDraw>(
        PLAYER_LIMB_HEAD, [](Player* player, s32 limbIndex) {
            // Ensure they aren't in first person
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0) &&
                !(player->stateFlags1 & PLAYER_STATE1_100000)) {
                OPEN_DISPS(gPlayState->state.gfxCtx);
                Matrix_Push();
                Player_DrawBunnyHood(gPlayState);
                gSPDisplayList(POLY_OPA_DISP++,
                               (Gfx*)D_801C0B20[PLAYER_MASK_BUNNY - 1]); // D_801C0B20 is an array of mask DLs
                Matrix_Pop();
                CLOSE_DISPS(gPlayState->state.gfxCtx);
            }
        });

    // This hook sets up the quad and draws the "active" blue border around the mask in the pause menu
    beforePageDrawHook = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::BeforeKaleidoDrawPage>(
        PAUSE_MASK, [](PauseContext* _, u16 __) {
            GraphicsContext* gfxCtx = gPlayState->state.gfxCtx;
            PauseContext* pauseCtx = &gPlayState->pauseCtx;
            s16 i = 0;
            s16 j = 0;
            s16 k;

            OPEN_DISPS(gfxCtx);

            persistentMasksVtx = (Vtx*)GRAPH_ALLOC(gfxCtx, (4 * 4) * sizeof(Vtx));

            if ((pauseCtx->state == PAUSE_STATE_MAIN)) {
                s16 slot = SLOT_MASK_BUNNY - ITEM_NUM_SLOTS;
                s16 slotX = slot % MASK_GRID_COLS;
                s16 slotY = slot / MASK_GRID_COLS;
                s16 initialX = 0 - (MASK_GRID_COLS * MASK_GRID_CELL_WIDTH) / 2;
                s16 initialY = (MASK_GRID_ROWS * MASK_GRID_CELL_HEIGHT) / 2 - 6;
                s16 vtxX = (initialX + (slotX * MASK_GRID_CELL_WIDTH)) + MASK_GRID_QUAD_MARGIN;
                s16 vtxY = (initialY - (slotY * MASK_GRID_CELL_HEIGHT)) + pauseCtx->offsetY - MASK_GRID_QUAD_MARGIN;
                persistentMasksVtx[i + 0].v.ob[0] = persistentMasksVtx[i + 2].v.ob[0] =
                    vtxX + MASK_GRID_SELECTED_QUAD_MARGIN;
                persistentMasksVtx[i + 1].v.ob[0] = persistentMasksVtx[i + 3].v.ob[0] =
                    persistentMasksVtx[i + 0].v.ob[0] + MASK_GRID_SELECTED_QUAD_WIDTH;
                persistentMasksVtx[i + 0].v.ob[1] = persistentMasksVtx[i + 1].v.ob[1] =
                    vtxY - MASK_GRID_SELECTED_QUAD_MARGIN;

                persistentMasksVtx[i + 2].v.ob[1] = persistentMasksVtx[i + 3].v.ob[1] =
                    persistentMasksVtx[i + 0].v.ob[1] - MASK_GRID_SELECTED_QUAD_HEIGHT;

                persistentMasksVtx[i + 0].v.ob[2] = persistentMasksVtx[i + 1].v.ob[2] =
                    persistentMasksVtx[i + 2].v.ob[2] = persistentMasksVtx[i + 3].v.ob[2] = 0;

                persistentMasksVtx[i + 0].v.flag = persistentMasksVtx[i + 1].v.flag = persistentMasksVtx[i + 2].v.flag =
                    persistentMasksVtx[i + 3].v.flag = 0;

                persistentMasksVtx[i + 0].v.tc[0] = persistentMasksVtx[i + 0].v.tc[1] =
                    persistentMasksVtx[i + 1].v.tc[1] = persistentMasksVtx[i + 2].v.tc[0] = 0;

                persistentMasksVtx[i + 1].v.tc[0] = persistentMasksVtx[i + 2].v.tc[1] =
                    persistentMasksVtx[i + 3].v.tc[0] = persistentMasksVtx[i + 3].v.tc[1] =
                        MASK_GRID_SELECTED_QUAD_TEX_SIZE * (1 << 5);

                persistentMasksVtx[i + 0].v.cn[0] = persistentMasksVtx[i + 1].v.cn[0] =
                    persistentMasksVtx[i + 2].v.cn[0] = persistentMasksVtx[i + 3].v.cn[0] =
                        persistentMasksVtx[i + 0].v.cn[1] = persistentMasksVtx[i + 1].v.cn[1] =
                            persistentMasksVtx[i + 2].v.cn[1] = persistentMasksVtx[i + 3].v.cn[1] =
                                persistentMasksVtx[i + 0].v.cn[2] = persistentMasksVtx[i + 1].v.cn[2] =
                                    persistentMasksVtx[i + 2].v.cn[2] = persistentMasksVtx[i + 3].v.cn[2] = 255;

                persistentMasksVtx[i + 0].v.cn[3] = persistentMasksVtx[i + 1].v.cn[3] =
                    persistentMasksVtx[i + 2].v.cn[3] = persistentMasksVtx[i + 3].v.cn[3] = pauseCtx->alpha;

                if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, 100, 200, 255, 255);
                    gSPVertex(POLY_OPA_DISP++, (uintptr_t)persistentMasksVtx, 4, 0);
                    POLY_OPA_DISP = Gfx_DrawTexQuadIA8(POLY_OPA_DISP, (TexturePtr)gEquippedItemOutlineTex, 32, 32, 0);
                }
            }

            CLOSE_DISPS(gfxCtx);
        });
}

void RegisterPersistentMasks() {
    UpdatePersistentMasksState();

    // Easiest way to handle things like loading into a different file, debug warping, etc.
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>(
        [](s8 sceneId, s8 spawnNum) { UpdatePersistentMasksState(); });

    // Speed the player up when the bunny hood state is active
    REGISTER_VB_SHOULD(VB_CONSIDER_BUNNY_HOOD_EQUIPPED, {
        // But don't speed up if the player is non-human and controller input is being overriden for cutscenes/minigames
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0) &&
            (GET_PLAYER_FORM == PLAYER_FORM_HUMAN || gPlayState->actorCtx.unk268 == 0)) {
            *should = true;
        }
    });

    // Overrides allowing them to equip a mask while transformed
    REGISTER_VB_SHOULD(VB_USE_ITEM_CONSIDER_LINK_HUMAN, {
        PlayerItemAction* itemAction = va_arg(args, PlayerItemAction*);
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) &&
            *itemAction == PLAYER_IA_MASK_BUNNY) {
            *should = true;
        }
    });

    // When they do equip the mask, prevent it and instead set our state
    REGISTER_VB_SHOULD(VB_USE_ITEM_EQUIP_MASK, {
        PlayerMask* maskId = va_arg(args, PlayerMask*);
        Player* player = GET_PLAYER(gPlayState);

        if (*maskId == PLAYER_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0)) {
            *should = false;

            CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0));
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                func_8082E1F0(player, NA_SE_PL_CHANGE_ARMS);
            } else {
                func_8082E1F0(player, NA_SE_PL_TAKE_OUT_SHIELD);
            }
        }
    });

    // Prevent the "equipped" white border from being drawn so ours shows instead (ours was drawn before it, so it's
    // underneath)
    REGISTER_VB_SHOULD(VB_DRAW_ITEM_EQUIPPED_OUTLINE, {
        ItemId* itemId = va_arg(args, ItemId*);
        if (*itemId == ITEM_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
            *should = false;
        }
    });

    // Typically when you are in a transformation all masks are dimmed on the C-Buttons
    REGISTER_VB_SHOULD(VB_ITEM_BE_RESTRICTED, {
        ItemId* itemId = va_arg(args, ItemId*);
        if (*itemId == ITEM_MASK_BUNNY && CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0)) {
            *should = false;
        }
    });

    // Override "A" press behavior on kaleido scope to toggle the mask state
    REGISTER_VB_SHOULD(VB_KALEIDO_DISPLAY_ITEM_TEXT, {
        ItemId itemId = (ItemId)*va_arg(args, u16*);
        if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.Enabled", 0) && itemId == ITEM_MASK_BUNNY) {
            *should = false;
            CVarSetInteger("gEnhancements.Masks.PersistentBunnyHood.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0));
            if (CVarGetInteger("gEnhancements.Masks.PersistentBunnyHood.State", 0)) {
                Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_DOWN);
            } else {
                Audio_PlaySfx(NA_SE_SY_CAMERA_ZOOM_UP);
            }
        } else if (CVarGetInteger("gEnhancements.Masks.PersistentGreatFairyMask.Enabled", 0) &&
                   itemId == ITEM_MASK_GREAT_FAIRY) {
            *should = false;
            CVarSetInteger("gEnhancements.Masks.PersistentGreatFairyMask.State",
                           !CVarGetInteger("gEnhancements.Masks.PersistentGreatFairyMask.State", 0));
        }
    });
}
