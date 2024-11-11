#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "src/overlays/actors/ovl_En_Mk/z_en_mk.h"
}

const uint32_t MAX_EGGS = 7;

void RegisterZoraEggCount() {
    // marine researcher, his actor update call is more consistent than the eggs
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(ACTOR_EN_MK, [](Actor* outerActor) {
        static uint32_t enMkUpdateHook = 0;
        static uint32_t enMkKillHook = 0;
        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(enMkUpdateHook);
        GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enMkKillHook);
        enMkUpdateHook = 0;
        enMkKillHook = 0;

        enMkUpdateHook = GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::OnActorUpdate>(
            (uintptr_t)outerActor, [](Actor* actor) {
                // complete quest if you have enough eggs
                if (gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14 != MAX_EGGS &&
                    CVarGetInteger("gEnhancements.Songs.ZoraEggCount", MAX_EGGS) <=
                        gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14) {
                    gSaveContext.save.saveInfo.permanentSceneFlags[SCENE_LABO].unk_14 = MAX_EGGS;
                }
            });
        enMkKillHook =
            GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(enMkUpdateHook);
                GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enMkKillHook);
                enMkUpdateHook = 0;
                enMkKillHook = 0;
            });
    });
}