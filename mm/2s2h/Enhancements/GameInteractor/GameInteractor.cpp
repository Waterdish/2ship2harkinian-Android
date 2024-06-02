#include "GameInteractor.h"
#include "spdlog/spdlog.h"

extern "C" {
#include "z64actor.h"
}

#include <libultraship/bridge.h>

void GameInteractor_ExecuteOnGameStateMainFinish() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateMainFinish>();
}

void GameInteractor_ExecuteOnGameStateDrawFinish() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateDrawFinish>();
}

void GameInteractor_ExecuteOnGameStateUpdate() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnGameStateUpdate>();
}

void GameInteractor_ExecuteOnSaveInit(s16 fileNum) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSaveInit>(fileNum);
}

void GameInteractor_ExecuteBeforeEndOfCycleSave() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::BeforeEndOfCycleSave>();
}

void GameInteractor_ExecuteAfterEndOfCycleSave() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::AfterEndOfCycleSave>();
}

void GameInteractor_ExecuteBeforeMoonCrashSaveReset() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::BeforeMoonCrashSaveReset>();
}

void GameInteractor_ExecuteOnSceneInit(s16 sceneId, s8 spawnNum) {
    SPDLOG_DEBUG("OnSceneInit: sceneId: {}, spawnNum: {}", sceneId, spawnNum);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneInit>(sceneId, spawnNum);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnSceneInit>(sceneId, sceneId, spawnNum);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneInit>(sceneId, spawnNum);
}

void GameInteractor_ExecuteOnRoomInit(s16 sceneId, s8 roomNum) {
    SPDLOG_DEBUG("OnRoomInit: sceneId: {}, roomNum: {}", sceneId, roomNum);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnRoomInit>(sceneId, roomNum);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnRoomInit>(sceneId, sceneId, roomNum);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnRoomInit>(sceneId, roomNum);
}

void GameInteractor_ExecuteOnPlayDestroy() {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnPlayDestroy>();
}

bool GameInteractor_ShouldActorInit(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorInit>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorInit>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorInit>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorInit>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorInit(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorInit>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorInit>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorInit>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorInit>(actor);
}

bool GameInteractor_ShouldActorUpdate(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorUpdate>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorUpdate>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorUpdate>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorUpdate>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorUpdate(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorUpdate>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorUpdate>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorUpdate>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorUpdate>(actor);
}

bool GameInteractor_ShouldActorDraw(Actor* actor) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldActorDraw>(actor, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldActorDraw>(actor->id, actor, &result);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldActorDraw>((uintptr_t)actor, actor, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldActorDraw>(actor, &result);
    return result;
}

void GameInteractor_ExecuteOnActorDraw(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorDraw>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorDraw>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorDraw>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorDraw>(actor);
}

void GameInteractor_ExecuteOnActorKill(Actor* actor) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnActorKill>(actor);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnActorKill>(actor->id, actor);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnActorKill>((uintptr_t)actor, actor);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnActorKill>(actor);
}

void GameInteractor_ExecuteOnSceneFlagSet(s16 sceneId, FlagType flagType, u32 flag) {
    SPDLOG_DEBUG("OnSceneFlagSet: sceneId: {}, flagType: {}, flag: {}", sceneId, (u32)flagType, flag);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneFlagSet>(sceneId, flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneFlagSet>(sceneId, flagType, flag);
}

void GameInteractor_ExecuteOnSceneFlagUnset(s16 sceneId, FlagType flagType, u32 flag) {
    SPDLOG_DEBUG("OnSceneFlagUnset: sceneId: {}, flagType: {}, flag: {}", sceneId, (u32)flagType, flag);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnSceneFlagUnset>(sceneId, flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnSceneFlagUnset>(sceneId, flagType, flag);
}

void GameInteractor_ExecuteOnFlagSet(FlagType flagType, u32 flag) {
    // This flag in particular is very spammy, so we'll suppress it
    if (!(flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_92_80)) {
        SPDLOG_DEBUG("OnFlagSet: flagType: {}, flag: {}", (u32)flagType, flag);
    }
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnFlagSet>(flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnFlagSet>(flagType, flag);
}

void GameInteractor_ExecuteOnFlagUnset(FlagType flagType, u32 flag) {
    // This flag in particular is very spammy, so we'll suppress it
    if (!(flagType == FLAG_WEEK_EVENT_REG && flag == WEEKEVENTREG_92_80)) {
        SPDLOG_DEBUG("OnFlagUnset: flagType: {}, flag: {}", (u32)flagType, flag);
    }
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnFlagUnset>(flagType, flag);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnFlagUnset>(flagType, flag);
}

void GameInteractor_ExecuteOnCameraChangeModeFlags(Camera* camera) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnCameraChangeModeFlags>(camera);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnCameraChangeModeFlags>(camera->uid, camera);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnCameraChangeModeFlags>((uintptr_t)camera, camera);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnCameraChangeModeFlags>(camera);
}

void GameInteractor_ExecuteAfterCameraUpdate(Camera* camera) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::AfterCameraUpdate>(camera);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::AfterCameraUpdate>(camera->uid, camera);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::AfterCameraUpdate>((uintptr_t)camera, camera);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::AfterCameraUpdate>(camera);
}

void GameInteractor_ExecuteOnCameraChangeSettingsFlags(Camera* camera) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnCameraChangeSettingsFlags>(camera);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnCameraChangeSettingsFlags>(camera->uid, camera);
    GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::OnCameraChangeSettingsFlags>((uintptr_t)camera,
                                                                                              camera);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnCameraChangeSettingsFlags>(camera);
}

void GameInteractor_ExecuteOnPassPlayerInputs(Input* input) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnPassPlayerInputs>(input);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnPassPlayerInputs>(input);
}

void GameInteractor_ExecuteOnOpenText(u16 textId) {
    SPDLOG_DEBUG("OnOpenText: textId: {}", textId);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnOpenText>(textId);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnOpenText>(textId, textId);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnOpenText>(textId);
}

bool GameInteractor_ShouldItemGive(u8 item) {
    bool result = true;
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldItemGive>(item, &result);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldItemGive>(item, item, &result);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldItemGive>(item, &result);
    return result;
}

void GameInteractor_ExecuteOnItemGive(u8 item) {
    SPDLOG_DEBUG("OnItemGive: item: {}", item);
    GameInteractor::Instance->ExecuteHooks<GameInteractor::OnItemGive>(item);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::OnItemGive>(item, item);
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::OnItemGive>(item);
}

bool GameInteractor_Should(GIVanillaBehavior flag, bool result, void* opt) {
    GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldVanillaBehavior>(flag, &result, opt);
    GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldVanillaBehavior>(flag, flag, &result, opt);
    if (opt != nullptr) {
        GameInteractor::Instance->ExecuteHooksForPtr<GameInteractor::ShouldVanillaBehavior>((uintptr_t)opt, flag,
                                                                                            &result, opt);
    }
    GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldVanillaBehavior>(flag, &result, opt);
    return result;
}

// Returns 1 or -1 based on a number of factors like CVars or other game states.
int GameInteractor_InvertControl(GIInvertType type) {
    int result = 1;

    switch (type) {
        case GI_INVERT_CAMERA_RIGHT_STICK_X:
            if (CVarGetInteger("gEnhancements.Camera.RightStick.InvertXAxis", 0)) {
                result *= -1;
            }
            break;
        case GI_INVERT_CAMERA_RIGHT_STICK_Y:
            if (CVarGetInteger("gEnhancements.Camera.RightStick.InvertYAxis", 1)) {
                result *= -1;
            }
            break;
    }

    /*
    // Invert all X axis inputs if the Mirrored World mode is enabled
    if (CVarGetInteger("gModes.MirroredWorld.State", 0)) {
        switch (type) {
            case GI_INVERT_CAMERA_RIGHT_STICK_X:
                result *= -1;
                break;
        }
    }
    */

    /*
    if (CrowdControl::State::InvertedInputs) {
        result *= -1;
    }
    */

    return result;
}

uint32_t GameInteractor_Dpad(GIDpadType type, uint32_t buttonCombo) {
    uint32_t result = 0;

    switch (type) {
        case GI_DPAD_OCARINA:
            if (CVarGetInteger("gEnhancements.Playback.DpadOcarina", 0)) {
                result = buttonCombo;
            }
            break;
        case GI_DPAD_EQUIP:
            if (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0)) {
                result = buttonCombo;
            }
            break;
    }

    return result;
}
