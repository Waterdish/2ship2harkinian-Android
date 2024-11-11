#include <libultraship/bridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"

extern "C" {
#include "z64.h"
#include "src/overlays/actors/ovl_En_Kakasi/z_en_kakasi.h"
}

void RegisterSkipScarecrowDance() {
    // Skips to end of "passing time" dance with the scarecrow
    GameInteractor::Instance->RegisterGameHookForID<GameInteractor::OnActorInit>(
        ACTOR_EN_KAKASI, [](Actor* outerActor) {
            static uint32_t enKakasiUpdateHook = 0;
            static uint32_t enKakasiKillHook = 0;
            GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::OnActorUpdate>(enKakasiUpdateHook);
            GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enKakasiKillHook);
            enKakasiUpdateHook = 0;
            enKakasiKillHook = 0;

            enKakasiUpdateHook = GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::OnActorUpdate>(
                (uintptr_t)outerActor, [](Actor* actor) {
                    EnKakasi* enKakasi = (EnKakasi*)actor;
                    if (CVarGetInteger("gEnhancements.Cutscenes.SkipMiscInteractions", 0) &&
                        enKakasi->actionFunc == EnKakasi_DancingNightAway && enKakasi->unk190 == 0) {
                        enKakasi->unk190 = 13;
                        enKakasi->unk204 = 0;
                    }
                });
            enKakasiKillHook =
                GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneInit>([](s8 sceneId, s8 spawnNum) {
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnActorUpdate>(enKakasiUpdateHook);
                    GameInteractor::Instance->UnregisterGameHook<GameInteractor::OnSceneInit>(enKakasiKillHook);
                    enKakasiUpdateHook = 0;
                    enKakasiKillHook = 0;
                });
        });
}
