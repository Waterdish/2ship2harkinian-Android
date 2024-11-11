#ifndef ENHANCEMENTS_H
#define ENHANCEMENTS_H

#include "Camera/Camera.h"
#include "Cheats/Cheats.h"
#include "Cutscenes/Cutscenes.h"
#include "Cycle/Cycle.h"
#include "Dialogue/Dialogue.h"
#include "Equipment/Equipment.h"
#include "Fixes/Fixes.h"
#include "Graphics/Graphics.h"
#include "Masks/Masks.h"
#include "Minigames/Minigames.h"
#include "Modes/Modes.h"
#include "Player/Player.h"
#include "Restorations/Restorations.h"
#include "Saving/SavingEnhancements.h"
#include "Songs/Songs.h"
#include "DifficultyOptions/DifficultyOptions.h"

enum AlwaysWinDoggyRaceOptions {
    ALWAYS_WIN_DOGGY_RACE_OFF,
    ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH,
    ALWAYS_WIN_DOGGY_RACE_ALWAYS,
};

enum TimeStopOptions {
    TIME_STOP_OFF,
    TIME_STOP_TEMPLES,
    TIME_STOP_TEMPLES_DUNGEONS,
};

enum ClockTypeOptions {
    CLOCK_TYPE_ORIGINAL,
    CLOCK_TYPE_3DS,
    CLOCK_TYPE_TEXT_BASED,
};

enum CremiaRewardsOptions {
    CREMIA_REWARD_RANDOM,
    CREMIA_REWARD_ALWAYS_HUG,
    CREMIA_REWARD_ALWAYS_RUPEE,
};

#ifdef __cplusplus
extern "C" {
#endif

void InitEnhancements();

#ifdef __cplusplus
}
#endif

#endif // ENHANCEMENTS_H
