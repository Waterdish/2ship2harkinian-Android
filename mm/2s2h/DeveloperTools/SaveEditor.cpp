#include "SaveEditor.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/BenGui/Notification.h"

#include "interface/icon_item_dungeon_static/icon_item_dungeon_static.h"
#include "archives/icon_item_24_static/icon_item_24_static_yar.h"

extern "C" {
#include <z64.h>
#include <z64save.h>
#include <macros.h>
#include <variables.h>
#include <functions.h>
#include "overlays/actors/ovl_En_Test4/z_en_test4.h"

extern PlayState* gPlayState;
extern SaveContext gSaveContext;
extern TexturePtr gItemIcons[131];
extern u8 gItemSlots[77];
void Interface_LoadItemIconImpl(PlayState* play, u8 btn);
void Interface_NewDay(PlayState* play, s32 day);
extern RegEditor* gRegEditor;
extern ActorOverlay gActorOverlayTable[ACTOR_ID_MAX];
extern s16 gPlayerFormObjectIds[PLAYER_FORM_MAX];
void Player_PlaySfx(Player* player, u16 sfxId);
void PlayerCall_Init(Actor* thisx, PlayState* play);
void PlayerCall_Update(Actor* thisx, PlayState* play);
void PlayerCall_Draw(Actor* thisx, PlayState* play);
void TransitionFade_SetColor(void* thisx, u32 color);

void func_80A42198(EnTest4* thisx);
void func_80A425E4(EnTest4* thisx, PlayState* play);
}

bool safeMode = true;
constexpr float INV_GRID_WIDTH = 46.0f;
constexpr float INV_GRID_HEIGHT = 58.0f;
constexpr float INV_GRID_ICON_SIZE = 40.0f;
constexpr float INV_GRID_PADDING = 10.0f;
constexpr float INV_GRID_TOP_MARGIN = 20.0f;
constexpr uint16_t HEART_COUNT_MIN = 3;
constexpr uint16_t HEART_COUNT_MAX = 20;
constexpr int16_t S16_ZERO = 0;
constexpr int8_t S8_ZERO = 0;
constexpr u8 U8_ZERO = 0;
constexpr u8 REG_PAGES_MAX = REG_PAGES;
constexpr u8 REG_GROUPS_MAX = REG_GROUPS - 1;
const char* MAGIC_LEVEL_NAMES[3] = { "No Magic", "Single Magic", "Double Magic" };
constexpr int8_t MAGIC_LEVEL_MAX = 2;
const char* WALLET_LEVEL_NAMES[3] = { "Child Wallet", "Adult Wallet", "Giant Wallet" };
constexpr u8 WALLET_LEVEL_MAX = 2;
ImVec4 colorTint;
const char* songTooltip;
const char* curForm;
ImVec4 formColor;
uint32_t formObject;

InventorySlot selectedInventorySlot = SLOT_NONE;
std::vector<ItemId> safeItemsForInventorySlot[SLOT_MASK_FIERCE_DEITY + 1] = {};

void initSafeItemsForInventorySlot() {
    for (int i = 0; i < sizeof(gItemSlots); i++) {
        InventorySlot slot = static_cast<InventorySlot>(gItemSlots[i]);
        switch (slot) {
            case SLOT_BOTTLE_1:
                if (i != ITEM_LONGSHOT) { // No longshot in bottles
                    safeItemsForInventorySlot[SLOT_BOTTLE_1].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_2].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_3].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_4].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_5].push_back(static_cast<ItemId>(i));
                    safeItemsForInventorySlot[SLOT_BOTTLE_6].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_BOW:
                if (i == ITEM_BOW) { // No elemental bows here
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_TRADE_KEY_MAMA:
                if (i != ITEM_SLINGSHOT) { // No slingshot in trade items
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            case SLOT_TRADE_DEED:
                if (i != ITEM_OCARINA_FAIRY) { // No fairy ocarina in trade items
                    safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                }
                break;
            default:
                safeItemsForInventorySlot[slot].push_back(static_cast<ItemId>(i));
                break;
        }
    }
}

char z2ASCII(int code) {
    int ret;
    if (code < 10) { // Digits
        ret = code + 0x30;
    } else if (code >= 10 && code < 36) { // Uppercase letters
        ret = code + 0x37;
    } else if (code >= 36 && code < 62) { // Lowercase letters
        ret = code + 0x3D;
    } else if (code == 62) { // Space
        ret = code - 0x1E;
    } else if (code == 63 || code == 64) { // _ and .
        ret = code - 0x12;
    } else {
        ret = code;
    }
    return char(ret);
}

char ASCII2z(int code) {
    int ret;
    if (code >= 0x30 && code < 0x3A) { // Digits
        ret = code - 0x30;
    } else if (code >= 0x41 && code < 0x5B) { // Uppercase letters
        ret = code - 0x37;
    } else if (code >= 0x61 && code < 0x7B) { // Lowercase letters
        ret = code - 0x3D;
    } else if (code == 0x20 || code == 0x0) { // Space
        ret = 62;
    } else if (code == 0x5F || code == 0x2E) { // _ and .
        ret = code + 0x12;
    } else {
        ret = code;
    }
    return char(ret);
}

char* getPlayerName(char* playerNameBuf) {
    for (int i = 0; i < 8; i++) {
        playerNameBuf[i] = z2ASCII(gSaveContext.save.saveInfo.playerData.playerName[i]);
    }
    playerNameBuf[8] = '\0';
    return playerNameBuf;
}

int setPlayerName(ImGuiInputTextCallbackData* data) {
    for (int i = 0; i < 8; i++) {
        gSaveContext.save.saveInfo.playerData.playerName[i] = ASCII2z(data->Buf[i]);
    }
    return 0;
};

EnTest4* FindEnTest4Actor() {
    if (gPlayState == NULL) {
        return NULL;
    }

    Actor* enTest4Search = gPlayState->actorCtx.actorLists[ACTORCAT_SWITCH].first;

    while (enTest4Search != NULL) {
        if (enTest4Search->id == ACTOR_EN_TEST4) {
            return (EnTest4*)enTest4Search;
        }
        enTest4Search = enTest4Search->next;
    }

    return NULL;
}

void UpdateGameTime(u16 gameTime) {
    bool newTimeIsNight = (gameTime > CLOCK_TIME(18, 0)) || (gameTime < CLOCK_TIME(6, 0));
    bool prevTimeIsNight = (gSaveContext.save.time > CLOCK_TIME(18, 0)) || (gSaveContext.save.time < CLOCK_TIME(6, 0));

    gSaveContext.save.time = gameTime;

    if (gPlayState == NULL) {
        return;
    }

    // Clear weather from day 2
    gWeatherMode = WEATHER_MODE_CLEAR;
    gPlayState->envCtx.lightningState = LIGHTNING_LAST;

    // When transitioning over night boundaries, stop the sequences and ask to replay, then respawn actors
    if (newTimeIsNight != prevTimeIsNight) {
        // AMBIENCE_ID_13 is used to persist a scenes sequence through night, so we shouldn't
        // change anything if thats active
        if (gPlayState->sequenceCtx.ambienceId != AMBIENCE_ID_13) {
            SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_AMBIENCE, 0);
            SEQCMD_STOP_SEQUENCE(SEQ_PLAYER_BGM_MAIN, 240);
            gSaveContext.seqId = (u8)NA_BGM_DISABLED;
            gSaveContext.ambienceId = AMBIENCE_ID_DISABLED;
            Environment_PlaySceneSequence(gPlayState);
        }

        // Kills/Spawns half-day actors
        gPlayState->numSetupActors = -gPlayState->numSetupActors;
    }

    EnTest4* enTest4 = FindEnTest4Actor();

    // Update EnTest4 actor to be in sync with the new time
    // This ensures that day transitions are not triggered with the change
    if (enTest4 != NULL) {
        enTest4->unk_146 = gameTime;
        enTest4->lastBellTime = gameTime;
        enTest4->csIdIndex = newTimeIsNight ? 0 : 1;

        // Sets the nextBellTime based on the new current time
        if (CURRENT_DAY == 3) {
            func_80A42198(enTest4);
        } else {
            func_80A425E4(enTest4, gPlayState);
        }

        // Unset any screen scaling from the above funcs
        gSaveContext.screenScale = 1000.0f;
        gSaveContext.screenScaleFlag = false;
    }
}

void DrawTempleClears() {
    bool cleared;
    bool open;
    bool inverted = false;
    bool inStoneTower = false;

    // Woodfall
    cleared = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE);
    if (UIWidgets::Checkbox("Woodfall cleared", &cleared)) {
        if (cleared) {
            SET_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_CLEARED_WOODFALL_TEMPLE);
        }
    }

    ImGui::SameLine();

    if (cleared) {
        open = true;
        SET_WEEKEVENTREG(WEEKEVENTREG_20_01);
    } else {
        open = CHECK_WEEKEVENTREG(WEEKEVENTREG_20_01);
    }
    if (UIWidgets::Checkbox("Woodfall Open", &open, { .disabled = cleared })) {
        if (open) {
            SET_WEEKEVENTREG(WEEKEVENTREG_20_01);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_20_01);
        }
    }

    // Snowhead
    cleared = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE);
    if (UIWidgets::Checkbox("Snowhead cleared", &cleared)) {
        if (cleared) {
            SET_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_CLEARED_SNOWHEAD_TEMPLE);
        }
    }

    ImGui::SameLine();

    if (cleared) {
        open = true;
        SET_WEEKEVENTREG(WEEKEVENTREG_30_01);
    } else {
        open = CHECK_WEEKEVENTREG(WEEKEVENTREG_30_01);
    }
    if (UIWidgets::Checkbox("Snowhead Open", &open, { .disabled = cleared })) {
        if (open) {
            SET_WEEKEVENTREG(WEEKEVENTREG_30_01);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_30_01);
        }
    }

    // Great Bay
    cleared = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE);
    if (UIWidgets::Checkbox("Great Bay cleared", &cleared)) {
        if (cleared) {
            SET_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_CLEARED_GREAT_BAY_TEMPLE);
        }
    }

    ImGui::SameLine();

    if (cleared) {
        open = true;
        // 53_20 is if the turtle is raised
        // 93_08 determines if a long or short cutscene should play
        SET_WEEKEVENTREG(WEEKEVENTREG_53_20);
        SET_WEEKEVENTREG(WEEKEVENTREG_93_08);
    } else {
        open = CHECK_WEEKEVENTREG(WEEKEVENTREG_53_20);
    }
    if (UIWidgets::Checkbox("Great Bay Open", &open, { .disabled = cleared })) {
        if (open) {
            SET_WEEKEVENTREG(WEEKEVENTREG_53_20);
            SET_WEEKEVENTREG(WEEKEVENTREG_93_08);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_53_20);
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_93_08);
        }
    }

    // Stone Tower
    cleared = CHECK_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);
    if (UIWidgets::Checkbox("Stone Tower cleared", &cleared)) {
        if (cleared) {
            SET_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_CLEARED_STONE_TOWER_TEMPLE);
        }
    }

    if (gPlayState != NULL) {
        inStoneTower = Play_GetOriginalSceneId(gPlayState->sceneId) == SCENE_F40;
        inverted = Flags_GetSwitch(gPlayState, 20);
    }

    ImGui::SameLine();

    if (UIWidgets::Checkbox("Stone Tower Inverted", &inverted,
                            { .disabled = !inStoneTower, .disabledTooltip = "Can only invert while in Stone Tower" })) {
        if (inverted) {
            Flags_SetSwitch(gPlayState, 20);
        } else {
            Flags_UnsetSwitch(gPlayState, 20);
        }
    }
}

void DrawGeneralTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("generalTab", ImVec2(0, 0), true);
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - 4.0f);

    ImGui::BeginGroup();
    ImGui::Text("Player Name");
    ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    static char playerNameBuf[9];
    ImGui::InputText("##playerNameInput", getPlayerName(playerNameBuf), 9,
                     ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_AutoSelectAll, setPlayerName);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::SetCursorPosY(0.0f);
    ImGui::BeginGroup();
    ImGui::Text("Time");
    UIWidgets::PushStyleSlider();
    static u16 minTime = 0;
    static u16 maxTime = 0xFFFF;
    // Get current time and format as digital string
    u16 curMinutes = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) % 60;
    u16 curHours = (s32)TIME_TO_MINUTES_F(gSaveContext.save.time) / 60;
    std::string minutes = (curMinutes < 10 ? "0" : "") + std::to_string(curMinutes);
    std::string hours = "";
    std::string ampm = "";
    // Handle 24 or 12 hour time
    if (CVarGetInteger("gEnhancements.Graphics.24HoursClock", 0)) {
        if (curHours < 10) {
            hours += "0";
        }
    } else {
        ampm = curHours >= 12 ? " pm" : " am";
        curHours = curHours % 12 ? curHours % 12 : 12;
    }
    hours += std::to_string(curHours);
    std::string timeString = hours + ":" + minutes + ampm + " (0x%x)";

    u16 gameTime = gSaveContext.save.time;
    if (ImGui::SliderScalar("##timeInput", ImGuiDataType_U16, &gameTime, &minTime, &maxTime, timeString.c_str())) {
        UpdateGameTime(gameTime);
    }
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Health", { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.doubleDefense = 1;
        gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
        gSaveContext.save.saveInfo.playerData.healthCapacity = gSaveContext.save.saveInfo.playerData.health = 20 * 16;
        gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetHealthButton",
                          { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.doubleDefense = 0;
        gSaveContext.save.saveInfo.inventory.defenseHearts = 0;
        gSaveContext.save.saveInfo.playerData.healthCapacity = gSaveContext.save.saveInfo.playerData.health = 3 * 16;
        gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
    }
    ImGui::SameLine();
    if (UIWidgets::Checkbox("DD", (bool*)&gSaveContext.save.saveInfo.playerData.doubleDefense,
                            { .color = UIWidgets::Colors::Red })) {
        if (gSaveContext.save.saveInfo.playerData.doubleDefense) {
            gSaveContext.save.saveInfo.inventory.defenseHearts = 20;
        } else {
            gSaveContext.save.saveInfo.inventory.defenseHearts = 0;
        }
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::DarkRed);
    int16_t heartCount = (int16_t)gSaveContext.save.saveInfo.playerData.healthCapacity / 16;
    if (ImGui::SliderScalar("##heartCountSlider", ImGuiDataType_U16, &heartCount, &HEART_COUNT_MIN, &HEART_COUNT_MAX,
                            "Max Hearts: %d")) {
        gSaveContext.save.saveInfo.playerData.healthCapacity = heartCount * 16;
        gSaveContext.save.saveInfo.playerData.health =
            MIN(gSaveContext.save.saveInfo.playerData.health, gSaveContext.save.saveInfo.playerData.healthCapacity);
    }
    ImGui::SliderScalar("##healthSlider", ImGuiDataType_S16, &gSaveContext.save.saveInfo.playerData.health, &S16_ZERO,
                        &gSaveContext.save.saveInfo.playerData.healthCapacity, "Health: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    // Time skip buttons
    ImGui::SameLine();
    ImGui::BeginGroup();
    static const std::array<std::pair<int8_t, const char*>, 4> timeSkipAmounts = {
        { { -60, "-1hr" }, { -15, "-15m" }, { 15, "+15m" }, { 60, "+1hr" } }
    };
    for (size_t i = 0; i < timeSkipAmounts.size(); i++) {
        const auto& skip = timeSkipAmounts.at(i);
        if (UIWidgets::Button(skip.second, { .color = UIWidgets::Colors::Indigo, .size = UIWidgets::Sizes::Inline })) {
            UpdateGameTime(gSaveContext.save.time + CLOCK_TIME(0, skip.first));
        }
        if (i < timeSkipAmounts.size() - 1) {
            ImGui::SameLine();
        }
    }
    // Day slider
    UIWidgets::PushStyleSlider();
    static s32 minDay = 0;
    static s32 maxDay = 4;
    if (ImGui::SliderScalar("##dayInput", ImGuiDataType_S32, &gSaveContext.save.day, &minDay, &maxDay, "Day: %d")) {
        gSaveContext.save.eventDayCount = CURRENT_DAY;

        if (gPlayState != nullptr) {
            Interface_NewDay(gPlayState, CURRENT_DAY);
            // Inverting setup actors forces half-day actors to kill/respawn for new day
            gPlayState->numSetupActors = -gPlayState->numSetupActors;
            // Load environment values for new day
            func_800FEAF4(&gPlayState->envCtx);
            // Clear weather from day 2
            gWeatherMode = WEATHER_MODE_CLEAR;
            gPlayState->envCtx.lightningState = LIGHTNING_LAST;
        }
    }
    // Time speed slider
    // Values are added to R_TIME_SPEED which is generally 3 in normal play state
    static s32 minSpeed = -3; // Time frozen
    static s32 maxSpeed = 7;
    std::string extraText = "";
    if (gSaveContext.save.timeSpeedOffset == 0) {
        extraText = " (default)";
    } else if (gSaveContext.save.timeSpeedOffset == -2) {
        extraText = " (inverted)";
    }
    std::string timeSpeedString = "Time Speed: " + std::to_string(gSaveContext.save.timeSpeedOffset + 3) + extraText;
    ImGui::SliderScalar("##timeSpeedInput", ImGuiDataType_S32, &gSaveContext.save.timeSpeedOffset, &minSpeed, &maxSpeed,
                        timeSpeedString.c_str());
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Magic", { .color = UIWidgets::Colors::DarkGreen, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magic = MAGIC_DOUBLE_METER;
        gSaveContext.save.saveInfo.playerData.magicLevel = 2;
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
        gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
        BUTTON_ITEM_EQUIP(PLAYER_FORM_DEKU, EQUIP_SLOT_B) = ITEM_DEKU_NUT;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetMagicButton",
                          { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magic = 0;
        gSaveContext.save.saveInfo.playerData.magicLevel = 0;
        gSaveContext.save.saveInfo.playerData.isMagicAcquired = false;
        gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
    }
    ImGui::SameLine();
    bool spinAttack = CHECK_WEEKEVENTREG(WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK);
    if (UIWidgets::Checkbox("Spin Lv2", &spinAttack, { .color = UIWidgets::Colors::DarkGreen })) {
        if (spinAttack) {
            SET_WEEKEVENTREG(WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK);
        } else {
            CLEAR_WEEKEVENTREG(WEEKEVENTREG_OBTAINED_GREAT_SPIN_ATTACK);
        }
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::DarkGreen);
    if (ImGui::SliderScalar("##magicLevelSlider", ImGuiDataType_S8, &gSaveContext.save.saveInfo.playerData.magicLevel,
                            &S8_ZERO, &MAGIC_LEVEL_MAX,
                            MAGIC_LEVEL_NAMES[gSaveContext.save.saveInfo.playerData.magicLevel])) {
        gSaveContext.magicCapacity = gSaveContext.save.saveInfo.playerData.magicLevel * MAGIC_NORMAL_METER;
        gSaveContext.save.saveInfo.playerData.magic =
            MIN(gSaveContext.save.saveInfo.playerData.magic, gSaveContext.magicCapacity);
        switch (gSaveContext.save.saveInfo.playerData.magicLevel) {
            case 0:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = false;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
                break;
            case 1:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = false;
                BUTTON_ITEM_EQUIP(PLAYER_FORM_DEKU, EQUIP_SLOT_B) = ITEM_DEKU_NUT;
                break;
            case 2:
                gSaveContext.save.saveInfo.playerData.isMagicAcquired = true;
                gSaveContext.save.saveInfo.playerData.isDoubleMagicAcquired = true;
                BUTTON_ITEM_EQUIP(PLAYER_FORM_DEKU, EQUIP_SLOT_B) = ITEM_DEKU_NUT;
                break;
        }
    }
    ImGui::SliderScalar("##magicSlider", ImGuiDataType_S8, &gSaveContext.save.saveInfo.playerData.magic, &S8_ZERO,
                        &gSaveContext.magicCapacity, "Magic: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    if (UIWidgets::Button("Max Rupees", { .color = UIWidgets::Colors::Green, .size = UIWidgets::Sizes::Inline })) {
        Inventory_ChangeUpgrade(UPG_WALLET, 2);
        gSaveContext.save.saveInfo.playerData.rupees = CUR_CAPACITY(UPG_WALLET);
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##resetRupeesButton",
                          { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
        gSaveContext.save.saveInfo.playerData.rupees = 0;
        Inventory_ChangeUpgrade(UPG_WALLET, 0);
    }
    UIWidgets::PushStyleSlider(UIWidgets::Colors::Green);
    u8 currentWalletLevel = CUR_UPG_VALUE(UPG_WALLET);
    if (ImGui::SliderScalar("##walletLevelSlider", ImGuiDataType_U8, &currentWalletLevel, &U8_ZERO, &WALLET_LEVEL_MAX,
                            WALLET_LEVEL_NAMES[currentWalletLevel])) {
        Inventory_ChangeUpgrade(UPG_WALLET, currentWalletLevel);
        gSaveContext.save.saveInfo.playerData.rupees =
            MIN(gSaveContext.save.saveInfo.playerData.rupees, CUR_CAPACITY(UPG_WALLET));
    }
    s16 walletCapacity = CUR_CAPACITY(UPG_WALLET);
    ImGui::SliderScalar("##rupeesSlider", ImGuiDataType_S16, &gSaveContext.save.saveInfo.playerData.rupees, &S16_ZERO,
                        &walletCapacity, "Rupees: %d");
    UIWidgets::PopStyleSlider();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    ImGui::Text("Bombers Code: %d %d %d %d %d", gSaveContext.save.saveInfo.bomberCode[0],
                gSaveContext.save.saveInfo.bomberCode[1], gSaveContext.save.saveInfo.bomberCode[2],
                gSaveContext.save.saveInfo.bomberCode[3], gSaveContext.save.saveInfo.bomberCode[4]);

    ImGui::Text("Banked Rupees: %d", HS_GET_BANK_RUPEES());
    int bankedRupees = HS_GET_BANK_RUPEES();
    UIWidgets::PushStyleSlider(UIWidgets::Colors::Green);
    if (ImGui::SliderInt("##setBank", &bankedRupees, 0, 5000, "Banked Rupees: %d")) {
        HS_SET_BANK_RUPEES(bankedRupees);
    }
    UIWidgets::Tooltip("To recieve the rewards, set the bank to 199, 999, or 4,999 then deposit a single rupee");
    UIWidgets::PopStyleSlider();

    DrawTempleClears();

    UIWidgets::Checkbox("Has Tatl", (bool*)&gSaveContext.save.hasTatl, { .color = UIWidgets::Colors::Gray });
    UIWidgets::Checkbox("Is Owl Save", (bool*)&gSaveContext.save.isOwlSave, { .color = UIWidgets::Colors::Gray });
    ImGui::EndGroup();

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

template <typename C, typename T> bool contains(C&& c, T e) {
    return std::find(std::begin(c), std::end(c), e) != std::end(c);
};

void DrawAmmoInput(InventorySlot slot) {
    int x = slot % 6;
    int y = ((int)floor(slot / 6) % 4);
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    ImGui::SetCursorPos(
        ImVec2(x * INV_GRID_WIDTH + INV_GRID_PADDING + 7.0f,
               y * INV_GRID_HEIGHT + INV_GRID_TOP_MARGIN + INV_GRID_PADDING + (INV_GRID_ICON_SIZE - 2.0f)));
    ImGui::PushItemWidth(24.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    if (ImGui::InputScalar("##ammoInput", ImGuiDataType_S8, &AMMO(currentItemId))) {
        // Crashes when < 0 and graphical issues when > 99
        AMMO(currentItemId) = MAX(0, MIN(AMMO(currentItemId), 99));
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
    ImGui::PopItemWidth();
}

void DrawEquipItemMenu(InventorySlot slot) {
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    if (ImGui::BeginMenu("Equip")) {
        if (ImGui::MenuItem("C-Left")) {
            SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_LEFT, currentItemId);
            SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_LEFT, slot);
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_LEFT);
        }
        if (ImGui::MenuItem("C-Down")) {
            SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_DOWN, currentItemId);
            SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_DOWN, slot);
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_DOWN);
        }
        if (ImGui::MenuItem("C-Right")) {
            SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_C_RIGHT, currentItemId);
            SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_RIGHT, slot);
            Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_C_RIGHT);
        }
        if (ImGui::MenuItem("D-Right")) {
            DPAD_SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_RIGHT, currentItemId);
            DPAD_SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_RIGHT, slot);
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_RIGHT);
        }
        if (ImGui::MenuItem("D-Left")) {
            DPAD_SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_LEFT, currentItemId);
            DPAD_SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_LEFT, slot);
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_LEFT);
        }
        if (ImGui::MenuItem("D-Down")) {
            DPAD_SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_DOWN, currentItemId);
            DPAD_SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_DOWN, slot);
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_DOWN);
        }
        if (ImGui::MenuItem("D-Up")) {
            DPAD_SET_CUR_FORM_BTN_ITEM(EQUIP_SLOT_D_UP, currentItemId);
            DPAD_SET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_UP, slot);
            Interface_Dpad_LoadItemIconImpl(gPlayState, EQUIP_SLOT_D_UP);
        }
        ImGui::EndMenu();
    }
}

void NextItemInSlot(InventorySlot slot) {
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);
    size_t currentItemIndex =
        find(safeItemsForInventorySlot[slot].begin(), safeItemsForInventorySlot[slot].end(), currentItemId) -
        safeItemsForInventorySlot[slot].begin();

    if (currentItemId == ITEM_NONE) {
        gSaveContext.save.saveInfo.inventory.items[slot] = safeItemsForInventorySlot[slot][0];

        if (gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOW) {
            Inventory_ChangeUpgrade(UPG_QUIVER, 1);
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        } else if (gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOMB ||
                   gSaveContext.save.saveInfo.inventory.items[slot] == ITEM_BOMBCHU) {
            if (CUR_UPG_VALUE(UPG_BOMB_BAG) == 0) {
                Inventory_ChangeUpgrade(UPG_BOMB_BAG, 1);
            }
            AMMO(gSaveContext.save.saveInfo.inventory.items[slot]) = CUR_CAPACITY(UPG_BOMB_BAG);
        }
    } else if (currentItemId == ITEM_BOW) {
        if (CUR_UPG_VALUE(UPG_QUIVER) < 3) {
            Inventory_ChangeUpgrade(UPG_QUIVER, CUR_UPG_VALUE(UPG_QUIVER) + 1);
            AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        } else {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[slot], slot);
            Inventory_ChangeUpgrade(UPG_QUIVER, 0);
            AMMO(ITEM_BOW) = 0;
        }
    } else if (currentItemId == ITEM_BOMB || currentItemId == ITEM_BOMBCHU) {
        if (CUR_UPG_VALUE(UPG_BOMB_BAG) < 3) {
            Inventory_ChangeUpgrade(UPG_BOMB_BAG, CUR_UPG_VALUE(UPG_BOMB_BAG) + 1);
            AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        } else {
            Inventory_DeleteItem(ITEM_BOMB, SLOT_BOMB);
            Inventory_DeleteItem(ITEM_BOMBCHU, SLOT_BOMBCHU);
            Inventory_ChangeUpgrade(UPG_BOMB_BAG, 0);
            AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = 0;
        }
    } else if (currentItemIndex < safeItemsForInventorySlot[slot].size() - 1) {
        Inventory_ReplaceItem(gPlayState, currentItemId, safeItemsForInventorySlot[slot][currentItemIndex + 1]);
    } else {
        Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[slot], slot);
    }
}

void DrawSlot(InventorySlot slot) {
    int x = slot % 6;
    int y = ((int)floor(slot / 6) % 4);
    ItemId currentItemId = static_cast<ItemId>(gSaveContext.save.saveInfo.inventory.items[slot]);

    ImGui::PushID(slot);

    if (currentItemId != ITEM_NONE &&
        currentItemId <= ITEM_BOW_LIGHT &&            // gItemSlots only has entries till 77 (ITEM_BOW_LIGHT)
        gItemSlots[currentItemId] <= SLOT_BOTTLE_6 && // There is only ammo data for the first page
        ((safeMode && gAmmoItems[gItemSlots[currentItemId]] != ITEM_NONE) || !safeMode)) {
        DrawAmmoInput(slot);
    }

    ImGui::SetCursorPos(
        ImVec2(x * INV_GRID_WIDTH + INV_GRID_PADDING, y * INV_GRID_HEIGHT + INV_GRID_TOP_MARGIN + INV_GRID_PADDING));

    // isEquipped border
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
    if (GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_LEFT) == slot || GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_DOWN) == slot ||
        GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_C_RIGHT) == slot ||
        (CVarGetInteger("gEnhancements.Dpad.DpadEquips", 0) &&
         (DPAD_GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_RIGHT) == slot ||
          DPAD_GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_LEFT) == slot ||
          DPAD_GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_DOWN) == slot ||
          DPAD_GET_CUR_FORM_BTN_SLOT(EQUIP_SLOT_D_UP) == slot))) {
        ImGui::PushStyleColor(ImGuiCol_Border, UIWidgets::Colors::White);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    }
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);

    ImTextureID textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gItemIcons[safeItemsForInventorySlot[slot][0]]);

    if (currentItemId != ITEM_NONE) {
        textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[currentItemId]);
    }

    if (ImGui::ImageButton(textureId, ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), 0,
                           ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, currentItemId == ITEM_NONE ? 0.4f : 1.0f))) {
        if (safeMode && safeItemsForInventorySlot[slot].size() < 2) {
            NextItemInSlot(slot);
        } else {
            selectedInventorySlot = slot;
            ImGui::OpenPopup("InventorySlotPopup");
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);

    if (ImGui::BeginPopup("InventorySlotPopup")) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
        if (ImGui::Button("##invNonePicker", ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE))) {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[selectedInventorySlot],
                                 selectedInventorySlot);
            ImGui::CloseCurrentPopup();
        }
        UIWidgets::Tooltip("None");

        size_t availableItems = safeMode ? safeItemsForInventorySlot[selectedInventorySlot].size() : ITEM_FISHING_ROD;
        for (int32_t pickerIndex = 0; pickerIndex < availableItems; pickerIndex++) {
            if (((pickerIndex + 1) % 8) != 0) {
                ImGui::SameLine();
            }
            ItemId id = safeMode ? safeItemsForInventorySlot[selectedInventorySlot][pickerIndex]
                                 : static_cast<ItemId>(pickerIndex);
            if (ImGui::ImageButton(
                    Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName((const char*)gItemIcons[id]),
                    ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), 0)) {
                gSaveContext.save.saveInfo.inventory.items[selectedInventorySlot] = id;
                ImGui::CloseCurrentPopup();
            }
            // TODO: Add names for items
            // UIWidgets::SetLastItemHoverText(SohUtils::GetItemName(id));
        }
        ImGui::PopStyleColor(3);
        ImGui::EndPopup();
    }

    ImGui::PopID();

    if (currentItemId != ITEM_NONE && ImGui::BeginPopupContextItem()) {
        DrawEquipItemMenu(slot);
        ImGui::EndPopup();
    }
}

void DrawItemsAndMasksTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    // Group left side in a child for a scroll bar
    ImGui::BeginChild("leftSide##items", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX);
    ImGui::BeginGroup();
    ImGui::BeginChild("itemsBox",
                      ImVec2(INV_GRID_WIDTH * 6 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 4 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Items");
    for (uint32_t i = SLOT_OCARINA; i <= SLOT_BOTTLE_6; i++) {
        InventorySlot slot = static_cast<InventorySlot>(i);

        DrawSlot(slot);
    }
    ImGui::EndChild();
    ImGui::BeginChild("masksBox",
                      ImVec2(INV_GRID_WIDTH * 6 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 4 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Masks");
    for (uint32_t i = SLOT_MASK_POSTMAN; i <= SLOT_MASK_FIERCE_DEITY; i++) {
        InventorySlot slot = static_cast<InventorySlot>(i);

        DrawSlot(slot);
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("equipsBox", ImVec2(0, 0), true);
    if (UIWidgets::Button("Give All##items")) {
        for (int32_t i = 0; i <= SLOT_MASK_FIERCE_DEITY; i++) {
            if (i >= SLOT_BOTTLE_1 && i <= SLOT_BOTTLE_6) {
                gSaveContext.save.saveInfo.inventory.items[i] = ITEM_BOTTLE;
            } else {
                gSaveContext.save.saveInfo.inventory.items[i] = safeItemsForInventorySlot[i].back();
            }
        }
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 3);
        Inventory_ChangeUpgrade(UPG_QUIVER, 3);
        AMMO(ITEM_BOW) = CUR_CAPACITY(UPG_QUIVER);
        AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = CUR_CAPACITY(UPG_BOMB_BAG);
        AMMO(ITEM_DEKU_STICK) = CUR_CAPACITY(UPG_DEKU_STICKS);
        AMMO(ITEM_DEKU_NUT) = CUR_CAPACITY(UPG_DEKU_NUTS);
        AMMO(ITEM_MAGIC_BEANS) = 20;
        AMMO(ITEM_POWDER_KEG) = 1;
    }
    if (UIWidgets::Button("Reset##items")) {
        for (int32_t i = 0; i <= SLOT_MASK_FIERCE_DEITY; i++) {
            Inventory_DeleteItem(gSaveContext.save.saveInfo.inventory.items[i], i);
        }
        AMMO(ITEM_BOW) = AMMO(ITEM_BOMB) = AMMO(ITEM_BOMBCHU) = AMMO(ITEM_DEKU_STICK) = AMMO(ITEM_DEKU_NUT) =
            AMMO(ITEM_MAGIC_BEANS) = AMMO(ITEM_POWDER_KEG) = 0;
        Inventory_ChangeUpgrade(UPG_BOMB_BAG, 0);
        Inventory_ChangeUpgrade(UPG_QUIVER, 0);
    }
    UIWidgets::Checkbox("Safe Mode", &safeMode);

    // Expose inputs to edit raw number values of equips
    // ImGui::Text("Equips");
    // ImGui::Text("C-Buttons");
    // ImGui::Text("D-Pad");
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void SongInfo(int32_t itemID) {
    switch (itemID) {
        case QUEST_SONG_SONATA:
            colorTint = ImVec4(0.588f, 1.0f, 0.392f, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Sonata of Awakening";
            break;
        case QUEST_SONG_LULLABY:
            colorTint = ImVec4(1.0f, 0.313f, 0.156f,
                               (CHECK_QUEST_ITEM(itemID) || CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO)) ? 1.0f : 0.4f);
            songTooltip = (!CHECK_QUEST_ITEM(itemID) && CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO))
                              ? "Goron Lullaby Intro"
                              : "Goron Lullaby";
            break;
        case QUEST_SONG_BOSSA_NOVA:
            colorTint = ImVec4(0.392f, 0.588f, 1.0f, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "New Wave Bossa Nova";
            break;
        case QUEST_SONG_ELEGY:
            colorTint = ImVec4(1.0f, 0.627f, 0.0f, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Elegy of Emptiness";
            break;
        case QUEST_SONG_OATH:
            colorTint = ImVec4(1.0f, 0.392f, 1.0f, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Oath to Order";
            break;
        case QUEST_SONG_TIME:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Song of Time";
            break;
        case QUEST_SONG_HEALING:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Song of Healing";
            break;
        case QUEST_SONG_EPONA:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Eponas Song";
            break;
        case QUEST_SONG_SOARING:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Song of Soaring";
            break;
        case QUEST_SONG_STORMS:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Song of Storms";
            break;
        case QUEST_SONG_SARIA:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Saria's Song (Unused?)";
            break;
        case QUEST_SONG_SUN:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = "Sun's Song (Not Obtainable)";
            break;
        default:
            colorTint = ImVec4(1, 1, 1, CHECK_QUEST_ITEM(itemID) ? 1.0f : 0.4f);
            songTooltip = " ";
            break;
    }
}

std::map<QuestItem, ItemId> questToItemMap = {
    { QUEST_REMAINS_ODOLWA, ITEM_REMAINS_ODOLWA }, { QUEST_REMAINS_GOHT, ITEM_REMAINS_GOHT },
    { QUEST_REMAINS_GYORG, ITEM_REMAINS_GYORG },   { QUEST_REMAINS_TWINMOLD, ITEM_REMAINS_TWINMOLD },
    { QUEST_SONG_SONATA, ITEM_SONG_SONATA },       { QUEST_SONG_LULLABY, ITEM_SONG_LULLABY },
    { QUEST_SONG_BOSSA_NOVA, ITEM_SONG_NOVA },     { QUEST_SONG_ELEGY, ITEM_SONG_ELEGY },
    { QUEST_SONG_OATH, ITEM_SONG_OATH },           { QUEST_SONG_SARIA, ITEM_SONG_SARIA },
    { QUEST_SONG_TIME, ITEM_SONG_TIME },           { QUEST_SONG_HEALING, ITEM_SONG_HEALING },
    { QUEST_SONG_EPONA, ITEM_SONG_EPONA },         { QUEST_SONG_SOARING, ITEM_SONG_SOARING },
    { QUEST_SONG_STORMS, ITEM_SONG_STORMS },       { QUEST_SONG_SUN, ITEM_SONG_SUN }
};

void NextQuestInSlot(QuestItem slot) {
    if (!gPlayState) {
        return;
    }
    Player* player = GET_PLAYER(gPlayState);
    if (slot == QUEST_SONG_LULLABY || slot == QUEST_SONG_LULLABY_INTRO) {
        if (CHECK_QUEST_ITEM(QUEST_SONG_LULLABY)) {
            REMOVE_QUEST_ITEM(QUEST_SONG_LULLABY);
            REMOVE_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO);
        } else if (CHECK_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO)) {
            SET_QUEST_ITEM(QUEST_SONG_LULLABY);
        } else {
            SET_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO);
        }
    } else if (slot == QUEST_SWORD) {
        uint32_t currentSword = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD);
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) >= EQUIP_VALUE_SWORD_GILDED) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
        } else {
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, currentSword + 1);
        }
        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) =
            ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_RAZOR) {
            gSaveContext.save.saveInfo.playerData.swordHealth = 100;
        }
        Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
    } else if (slot == QUEST_SHIELD) {
        uint32_t currentShield = GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD);
        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == 1) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_MIRROR);
        } else {
            SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_HERO);
        }
        Player_SetEquipmentData(gPlayState, player);
    } else {
        if (CHECK_QUEST_ITEM(slot)) {
            REMOVE_QUEST_ITEM(slot);
        } else {
            SET_QUEST_ITEM(slot);
        }
    }
}

void DrawQuestSlot(QuestItem slot) {
    int x = slot % 6;
    int y = ((int)floor(slot / 6) % 4);
    ImGui::PushID(slot);

    ImGui::SetCursorPos(
        ImVec2(x * INV_GRID_WIDTH + INV_GRID_PADDING, y * INV_GRID_HEIGHT + INV_GRID_TOP_MARGIN + INV_GRID_PADDING));

    ImTextureID textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gItemIcons[questToItemMap[slot]]);
    if (ImGui::ImageButton(std::to_string(slot).c_str(), textureId, ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE),
                           ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                           ImVec4(1, 1, 1, CHECK_QUEST_ITEM(slot) ? 1.0f : 0.4f))) {
        NextQuestInSlot(slot);
    }

    ImGui::PopID();
}

void DrawSong(QuestItem slot) {
    SongInfo(slot);
    if (ImGui::ImageButton(std::to_string(slot).c_str(),
                           Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
                               (const char*)gItemIcons[questToItemMap[(QuestItem)slot]]),
                           ImVec2(INV_GRID_ICON_SIZE / 1.5f, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                           ImVec4(0, 0, 0, 0), colorTint)) {
        NextQuestInSlot(slot);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(songTooltip);
        ImGui::EndTooltip();
    }
    if (slot != QUEST_SONG_SUN) {
        ImGui::SameLine();
    }
}

void DrawQuestStatusTab() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("questTab", ImVec2(0, 0), true);

    if (UIWidgets::Button("Give All##items", { .color = UIWidgets::Colors::Green, .size = UIWidgets::Sizes::Inline })) {
        for (int32_t i = QUEST_REMAINS_ODOLWA; i <= QUEST_BOMBERS_NOTEBOOK; i++) {
            if (i != QUEST_SHIELD && i != QUEST_SWORD) {
                SET_QUEST_ITEM(i);
            }
        }
        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_MIRROR);
        if (GET_PLAYER_FORM != PLAYER_FORM_FIERCE_DEITY) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_GILDED);
            if (gPlayState) {
                Player_SetEquipmentData(gPlayState, GET_PLAYER(gPlayState));
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) =
                    ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
                Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
            }
        }
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Reset##items", { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
        for (int32_t i = QUEST_REMAINS_ODOLWA; i <= QUEST_BOMBERS_NOTEBOOK; i++) {
            if (i != QUEST_SHIELD && i != QUEST_SWORD) {
                REMOVE_QUEST_ITEM(i);
            }
        }
        REMOVE_QUEST_ITEM(QUEST_SONG_LULLABY_INTRO);
        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, EQUIP_VALUE_SHIELD_HERO);
        if (GET_PLAYER_FORM != PLAYER_FORM_FIERCE_DEITY) {
            SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, EQUIP_VALUE_SWORD_KOKIRI);
            if (gPlayState) {
                Player_SetEquipmentData(gPlayState, GET_PLAYER(gPlayState));
                BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) =
                    ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
                Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
            }
        }
    }

    ImGui::BeginChild("remainsBox",
                      ImVec2(INV_GRID_WIDTH * 4 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 1 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Boss Remains");
    for (int32_t i = QUEST_REMAINS_ODOLWA; i <= QUEST_REMAINS_TWINMOLD; i++) {
        QuestItem slot = static_cast<QuestItem>(i);

        DrawQuestSlot(slot);
    }
    ImGui::EndChild();
    ImGui::BeginChild("songBox",
                      ImVec2((INV_GRID_WIDTH / 1.1f) * 6 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 2.15f + INV_GRID_PADDING * 1 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Songs");
    for (int32_t i = QUEST_SONG_TIME; i <= QUEST_SONG_SUN; i++) {
        DrawSong((QuestItem)i);
    }
    for (int32_t i = QUEST_SONG_SONATA; i <= QUEST_SONG_SARIA; i++) {
        DrawSong((QuestItem)i);
    }
    ImGui::EndChild();
    ImGui::BeginChild("equipBox",
                      ImVec2(INV_GRID_WIDTH * 2.2 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 1 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Equipment");
    if (GET_PLAYER_FORM == PLAYER_FORM_FIERCE_DEITY) {
        ImTextureID swordTextureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_SWORD_DEITY]);
        ImGui::ImageButton(std::to_string(ITEM_SWORD_DEITY).c_str(), swordTextureId,
                           ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                           ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
    } else {
        ImTextureID swordTextureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)
                gItemIcons[ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI]);
        if (ImGui::ImageButton(std::to_string(ITEM_SWORD_KOKIRI).c_str(), swordTextureId,
                               ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                               ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            NextQuestInSlot(QUEST_SWORD);
        }
    }
    ImGui::SameLine();
    if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SHIELD) == EQUIP_VALUE_SHIELD_HERO) {
        ImTextureID shieldTextureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_SHIELD_HERO]);
        if (ImGui::ImageButton(std::to_string(ITEM_SHIELD_HERO).c_str(), shieldTextureId,
                               ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                               ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            NextQuestInSlot(QUEST_SHIELD);
        }
    } else {
        ImTextureID shieldTextureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
            (const char*)gItemIcons[ITEM_SHIELD_MIRROR]);
        if (ImGui::ImageButton(std::to_string(ITEM_SHIELD_MIRROR).c_str(), shieldTextureId,
                               ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                               ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
            NextQuestInSlot(QUEST_SHIELD);
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("notebookBox",
                      ImVec2(INV_GRID_WIDTH * 1 + INV_GRID_PADDING * 2,
                             INV_GRID_HEIGHT * 1 + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                      ImGuiChildFlags_Border);
    ImGui::Text("Bombers");
    ImTextureID textureId = Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(
        (const char*)gItemIcons[ITEM_BOMBERS_NOTEBOOK]);
    if (ImGui::ImageButton(std::to_string(ITEM_BOMBERS_NOTEBOOK).c_str(), textureId,
                           ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1),
                           ImVec4(0, 0, 0, 0),
                           ImVec4(1, 1, 1, CHECK_QUEST_ITEM(QUEST_BOMBERS_NOTEBOOK) ? 1.0f : 0.4f))) {
        NextQuestInSlot(QUEST_BOMBERS_NOTEBOOK);
    }
    ImGui::EndChild();
    ImGui::BeginChild("heartshapedBox",
                      ImVec2(INV_GRID_WIDTH * 2 + INV_GRID_PADDING * 2, INV_GRID_HEIGHT + INV_GRID_PADDING),
                      ImGuiChildFlags_Border);
    ImGui::Text("Heart Pieces");
    int32_t pohCount = (gSaveContext.save.saveInfo.inventory.questItems & 0xF0000000) >> 28;
    UIWidgets::PushStyleCombobox(UIWidgets::Colors::Red);
    if (ImGui::BeginCombo("##PoHcount", std::to_string(pohCount).c_str())) {
        for (int32_t i = 0; i < 4; i++) {
            if (ImGui::Selectable(std::to_string(i).c_str(), pohCount == i)) {
                gSaveContext.save.saveInfo.inventory.questItems &= ~0xF0000000;
                gSaveContext.save.saveInfo.inventory.questItems |= (i << 28);
            }
        }
        ImGui::EndCombo();
    }
    UIWidgets::PopStyleCombobox();
    ImGui::EndChild();

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

void SetDungeonItems(uint32_t dungeonItem, uint32_t dungeonId) {
    if (CHECK_DUNGEON_ITEM(dungeonItem, dungeonId)) {
        REMOVE_DUNGEON_ITEM(dungeonItem, dungeonId);
    } else {
        SET_DUNGEON_ITEM(dungeonItem, dungeonId);
    }
}

const char* dungeonNames[4] = { "Woodfall Temple", "Snowhead Temple", "Great Bay Temple", "Stone Tower Temple" };
uint32_t smallKeyCounts[4] = { 1, 3, 1, 4 };
const char* fairyIcons[] = { gDungeonStrayFairyWoodfallIconTex, gDungeonStrayFairySnowheadIconTex,
                             gDungeonStrayFairyGreatBayIconTex, gDungeonStrayFairyStoneTowerIconTex };
void DrawDungeonItemTab() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("dungeonTab", ImVec2(0, 0), true);

    for (int i = DUNGEON_INDEX_WOODFALL_TEMPLE; i <= DUNGEON_INDEX_STONE_TOWER_TEMPLE; i++) {
        std::string stray_id = "Stray" + std::to_string(i);
        std::string map_id = "Map" + std::to_string(i);
        std::string comp_id = "Compass" + std::to_string(i);
        std::string sKey_id = "Small Key" + std::to_string(i);
        std::string bKey_id = "Boss Key" + std::to_string(i);
        int dungeonId = i;
        ImGui::BeginChild(std::to_string(i).c_str(),
                          ImVec2(INV_GRID_WIDTH * 6 + INV_GRID_PADDING * 2, INV_GRID_HEIGHT * 1.5 + INV_GRID_PADDING),
                          ImGuiChildFlags_Border);
        ImGui::Text(dungeonNames[i]);
        if (ImGui::ImageButton(
                stray_id.c_str(),
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(fairyIcons[dungeonId]),
                ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                ImVec4(1, 1, 1, gSaveContext.save.saveInfo.inventory.strayFairies[dungeonId] ? 1.0f : 0.4f))) {
            ImGui::OpenPopup("strayFairies");
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(
                map_id.c_str(),
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconDungeonMapTex),
                ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                ImVec4(1, 1, 1, CHECK_DUNGEON_ITEM(DUNGEON_MAP, i) ? 1.0f : 0.4f))) {
            SetDungeonItems(DUNGEON_MAP, i);
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(
                comp_id.c_str(),
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconCompassTex),
                ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                ImVec4(1, 1, 1, CHECK_DUNGEON_ITEM(DUNGEON_COMPASS, i) ? 1.0f : 0.4f))) {
            SetDungeonItems(DUNGEON_COMPASS, i);
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(
                sKey_id.c_str(),
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconSmallKeyTex),
                ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                ImVec4(1, 1, 1, DUNGEON_KEY_COUNT(i) + 1 ? 1.0f : 0.4f))) {
            ImGui::OpenPopup("smallKeys");
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(
                bKey_id.c_str(),
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gQuestIconBossKeyTex),
                ImVec2(INV_GRID_ICON_SIZE, INV_GRID_ICON_SIZE), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
                ImVec4(1, 1, 1, CHECK_DUNGEON_ITEM(DUNGEON_BOSS_KEY, i) ? 1.0f : 0.4f))) {
            SetDungeonItems(DUNGEON_BOSS_KEY, i);
        }
        if (ImGui::BeginPopup("strayFairies")) {
            s32 minStray = 0;
            s32 maxStray = 15;
            int currentStrays = gSaveContext.save.saveInfo.inventory.strayFairies[dungeonId];
            if (ImGui::SliderScalar("##strayCount", ImGuiDataType_S32, &currentStrays, &minStray, &maxStray,
                                    "Strays: %d")) {
                gSaveContext.save.saveInfo.inventory.strayFairies[dungeonId] = currentStrays;
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("smallKeys")) {
            s32 minKey = -1;
            s32 maxKey = smallKeyCounts[dungeonId];
            int currentKeys = gSaveContext.save.saveInfo.inventory.dungeonKeys[dungeonId];
            if (ImGui::SliderScalar("##sKeyCount", ImGuiDataType_S32, &currentKeys, &minKey, &maxKey,
                                    "Small Keys: %d")) {
                gSaveContext.save.saveInfo.inventory.dungeonKeys[dungeonId] = currentKeys;
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

void GetPlayerForm(uint32_t form) {
    switch (form) {
        case PLAYER_FORM_FIERCE_DEITY:
            curForm = "Fierce Deity";
            formColor = UIWidgets::Colors::Red;
            formObject = OBJECT_LINK_BOY;
            break;
        case PLAYER_FORM_GORON:
            curForm = "Goron";
            formColor = UIWidgets::Colors::Yellow;
            formObject = OBJECT_LINK_GORON;
            break;
        case PLAYER_FORM_ZORA:
            curForm = "Zora";
            formColor = UIWidgets::Colors::Indigo;
            formObject = OBJECT_LINK_ZORA;
            break;
        case PLAYER_FORM_DEKU:
            curForm = "Deku";
            formColor = UIWidgets::Colors::DarkGreen;
            formObject = OBJECT_LINK_NUTS;
            break;
        case PLAYER_FORM_HUMAN:
            curForm = "Human";
            formColor = UIWidgets::Colors::LightGreen;
            formObject = OBJECT_LINK_CHILD;
            break;
        default:
            break;
    }
}

void DrawPlayerTab() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("playerTab", ImVec2(0, 0), true);

    if (gPlayState) {
        Player* player = GET_PLAYER(gPlayState);
        ImGui::BeginChild("playerLocation",
                          ImVec2(INV_GRID_WIDTH * 8 + INV_GRID_PADDING * 2,
                                 INV_GRID_HEIGHT * 1.75f + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                          ImGuiChildFlags_Border);

        GetPlayerForm(GET_PLAYER_FORM);
        ImGui::Text("%s Link", curForm);
        ImGui::Text("Position:");
        UIWidgets::PushStyleCombobox(formColor);
        ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
        ImGui::InputScalar("X Pos", ImGuiDataType_Float, &player->actor.world.pos.x);
        ImGui::SameLine();
        ImGui::InputScalar("Y Pos", ImGuiDataType_Float, &player->actor.world.pos.y);
        ImGui::SameLine();
        ImGui::InputScalar("Z Pos", ImGuiDataType_Float, &player->actor.world.pos.z);
        ImGui::Text("Rotation:");
        ImGui::InputScalar("X Rot", ImGuiDataType_S16, &player->actor.world.rot.x);
        ImGui::SameLine();
        ImGui::InputScalar("Y Rot", ImGuiDataType_S16, &player->actor.world.rot.y);
        ImGui::SameLine();
        ImGui::InputScalar("Z Rot", ImGuiDataType_S16, &player->actor.world.rot.z);

        ImGui::PopItemWidth();
        UIWidgets::PopStyleCombobox();
        ImGui::EndChild();

        ImGui::BeginChild("playerSpeed",
                          ImVec2(INV_GRID_WIDTH * 5 + INV_GRID_PADDING * 2,
                                 INV_GRID_HEIGHT * 2.75f + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                          ImGuiChildFlags_Border);
        ImGui::Text("Link's Speed");
        UIWidgets::PushStyleCombobox(formColor);
        ImGui::PushItemWidth(ImGui::GetFontSize() * 6);
        ImGui::InputScalar("Linear Velocity", ImGuiDataType_Float, &player->linearVelocity);
        ImGui::InputScalar("Y Velocity", ImGuiDataType_Float, &player->actor.velocity.y);
        ImGui::InputScalar("Ledge Height", ImGuiDataType_Float, &player->yDistToLedge);
        ImGui::InputScalar("Invincibility Timer", ImGuiDataType_S16, &player->invincibilityTimer);
        ImGui::InputScalar("Gravity", ImGuiDataType_Float, &player->actor.gravity);

        ImGui::PopItemWidth();
        UIWidgets::PopStyleCombobox();
        ImGui::EndChild();

        ImGui::BeginChild("playerForm",
                          ImVec2(INV_GRID_WIDTH * 8 + INV_GRID_PADDING * 2,
                                 INV_GRID_HEIGHT * 2.2f + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                          ImGuiChildFlags_Border);
        ImGui::Text("Change Link's Current Form");
        for (int i = PLAYER_FORM_FIERCE_DEITY; i <= PLAYER_FORM_HUMAN; i++) {
            GetPlayerForm(i);
            UIWidgets::PushStyleButton(formColor);
            if (ImGui::Button(curForm)) {
                s16 objectId = formObject;
                if (i != PLAYER_FORM_HUMAN) {
                    gSaveContext.save.equippedMask = PLAYER_MASK_FIERCE_DEITY + i;
                }
                gActorOverlayTable[ACTOR_PLAYER].initInfo->objectId = objectId;
                func_8012F73C(&gPlayState->objectCtx, player->actor.objectSlot, objectId);
                player->actor.objectSlot = Object_GetSlot(&gPlayState->objectCtx, GAMEPLAY_KEEP);
                gSaveContext.save.playerForm = i;
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
            }
            UIWidgets::PopStyleButton();
            if (i != PLAYER_FORM_HUMAN) {
                ImGui::SameLine();
            }
        }

        ImGui::Separator();
        ImGui::Text("Link's Current Equipment");
        if (GET_PLAYER_FORM == PLAYER_FORM_HUMAN) {
            static const char* weaponCombo[] = { "Kokiri Sword", "Razor Sword", "Gilded Sword" };
            static const char* shieldCombo[] = { "Hero's Shield", "Mirror Shield" };
            static int currentWeapon = 0;
            static int currentShield = 0;

            UIWidgets::PushStyleCombobox(UIWidgets::Colors::LightGreen);
            ImGui::PushItemWidth(ImGui::GetFontSize() * 15);
            if (ImGui::BeginCombo("Equipped Sword", weaponCombo[currentWeapon])) {
                for (int i = EQUIP_VALUE_SWORD_KOKIRI; i <= EQUIP_VALUE_SWORD_GILDED; i++) {
                    const bool isSelected = (currentWeapon == i);
                    if (ImGui::Selectable(weaponCombo[i - 1], isSelected)) {
                        currentWeapon = i - 1;
                        SET_EQUIP_VALUE(EQUIP_TYPE_SWORD, i);
                        if (GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) == EQUIP_VALUE_SWORD_RAZOR) {
                            gSaveContext.save.saveInfo.playerData.swordHealth = 100;
                        }
                        BUTTON_ITEM_EQUIP(0, EQUIP_SLOT_B) =
                            ITEM_SWORD_KOKIRI + GET_CUR_EQUIP_VALUE(EQUIP_TYPE_SWORD) - EQUIP_VALUE_SWORD_KOKIRI;
                        Interface_LoadItemIconImpl(gPlayState, EQUIP_SLOT_B);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (ImGui::BeginCombo("Equipped Shield", shieldCombo[currentShield])) {
                for (int i = EQUIP_VALUE_SHIELD_HERO; i <= EQUIP_VALUE_SHIELD_MIRROR; i++) {
                    const bool isSelected = (currentShield == i);
                    if (ImGui::Selectable(shieldCombo[i - 1], isSelected)) {
                        currentShield = i - 1;
                        SET_EQUIP_VALUE(EQUIP_TYPE_SHIELD, i);
                        Player_SetEquipmentData(gPlayState, player);
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();
            UIWidgets::PopStyleCombobox();

        } else {
            ImGui::Separator();
            ImGui::Text("              No Equipment in this Form");
            ImGui::Separator();
        }
        ImGui::EndChild();

        ImGui::BeginChild("playerStates",
                          ImVec2(INV_GRID_WIDTH * 8 + INV_GRID_PADDING * 2,
                                 INV_GRID_HEIGHT * 0.75f + INV_GRID_PADDING * 2 + INV_GRID_TOP_MARGIN),
                          ImGuiChildFlags_Border);
        ImGui::Text("Player States");
        uint32_t states[4] = { player->stateFlags1, player->stateFlags2, player->stateFlags3,
                               player->meleeWeaponState };

        if (ImGui::BeginTable("stateTable", 4)) {
            GetPlayerForm(GET_PLAYER_FORM);
            ImGui::PushStyleVar(ImGuiTableColumnFlags_WidthFixed, 15.0f);
            ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, formColor);
            ImGui::TableSetupColumn("State 1");
            ImGui::TableSetupColumn("State 2");
            ImGui::TableSetupColumn("State 3");
            ImGui::TableSetupColumn("Sword State");
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();

            for (int i = 0; i <= 3; i++) {
                ImGui::Text(std::to_string(states[i]).c_str());
                ImGui::TableNextColumn();
            }

            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(1);
}

const char* regGroupNames[] = {
    "REG  (0)",  "SREG (1)",  "OREG (2)",  "PREG (3)",  "QREG (4)",  "MREG (5)",  "YREG (6)",  "DREG (7)",
    "UREG (8)",  "IREG (9)",  "ZREG (10)", "CREG (11)", "NREG (12)", "KREG (13)", "XREG (14)", "cREG (15)",
    "sREG (16)", "iREG (17)", "WREG (18)", "AREG (19)", "VREG (20)", "HREG (21)", "GREG (22)", "mREG (23)",
    "nREG (24)", "BREG (25)", "dREG (26)", "kREG (27)", "bREG (28)",
};

// 2S2H Added columns to scene table: entranceSceneId, betterMapSelectIndex, humanName
#define DEFINE_SCENE(_name, enumValue, _textId, _drawConfig, _restrictionFlags, _persistentCycleFlags, \
                     _entranceSceneId, _betterMapSelectIndex, humanName)                               \
    { enumValue, humanName },
#define DEFINE_SCENE_UNSET(_enumValue)

std::unordered_map<s16, const char*> sceneList = {
#include "tables/scene_table.h"
};

#undef DEFINE_SCENE
#undef DEFINE_SCENE_UNSET

void DrawRegEditorTab() {
    UIWidgets::PushStyleSlider();
    ImGui::SliderScalar("Reg Group", ImGuiDataType_U8, &gRegEditor->regGroup, &S8_ZERO, &REG_GROUPS_MAX,
                        regGroupNames[gRegEditor->regGroup]);
    ImGui::SliderScalar("Reg Page", ImGuiDataType_U8, &gRegEditor->regPage, &S8_ZERO, &REG_PAGES_MAX, "%d");
    UIWidgets::PopStyleSlider();

    ImGui::BeginChild("regSliders", ImVec2(0, 0), ImGuiChildFlags_Border);

    for (int i = 0; i < REG_PER_PAGE; i++) {
        ImGui::PushID(i);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f - 4.0f);
        ImGui::BeginGroup();
        ImGui::Text("%02X (%d)", i + gRegEditor->regPage * REG_PER_PAGE, i + gRegEditor->regPage * REG_PER_PAGE);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, UIWidgets::Colors::Gray);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
        ImGui::InputScalar(
            "##regInput", ImGuiDataType_S16,
            &gRegEditor->data[i + gRegEditor->regPage * REG_PER_PAGE + gRegEditor->regGroup * REG_PER_PAGE * REG_PAGES],
            NULL, NULL, "%d");
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(1);
        ImGui::EndGroup();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

const char* flagEditorSections[] = {
    "currentSceneFlags", "weekEventReg",        "eventInf",        "scenesVisible",
    "owlActivation",     "permanentSceneFlags", "cycleSceneFlags",
};

void DrawFlagsTab() {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::BeginChild("flagsBox", ImVec2(0, 0), true);
    static int selectedFlagSection = 0;
    UIWidgets::Combobox("Flag Type", &selectedFlagSection, flagEditorSections,
                        {
                            .alignment = UIWidgets::ComponentAlignment::Left,
                            .labelPosition = UIWidgets::LabelPosition::None,
                        });

    static int16_t selectedScene = 0;
    if (selectedFlagSection == 5 || selectedFlagSection == 6) {
        ImGui::SameLine();
        UIWidgets::Combobox("Scene", &selectedScene, sceneList,
                            {
                                .alignment = UIWidgets::ComponentAlignment::Left,
                                .labelPosition = UIWidgets::LabelPosition::None,
                            });
        if (gPlayState != NULL) {
            ImGui::SameLine();
            if (UIWidgets::Button("Current", {
                                                 .color = UIWidgets::Colors::Gray,
                                                 .size = UIWidgets::Sizes::Inline,
                                             })) {
                selectedScene = gPlayState->sceneId;
            }
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));
    switch (selectedFlagSection) {
        case 0: // currentSceneFlags
            if (gPlayState == NULL) {
                ImGui::Text("Play state is NULL, cannot display scene flags");
                break;
            }

            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switches[0]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switches0",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[0] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switches0",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[0] = 0;
            }
            UIWidgets::DrawFlagArray32("##switches0", gPlayState->actorCtx.sceneFlags.switches[0]);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switches[1]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switches1",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[1] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switches1",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[1] = 0;
            }
            UIWidgets::DrawFlagArray32("##switches1", gPlayState->actorCtx.sceneFlags.switches[1]);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switches[2]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switches2",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[2] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switches2",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[2] = 0;
            }
            UIWidgets::DrawFlagArray32("##switches2", gPlayState->actorCtx.sceneFlags.switches[2]);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switches[3]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switches3",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[3] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switches3",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.switches[3] = 0;
            }
            UIWidgets::DrawFlagArray32("##switches3", gPlayState->actorCtx.sceneFlags.switches[3]);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("chest");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##chest",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.chest = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##chest",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.chest = 0;
            }
            UIWidgets::DrawFlagArray32("##chest", gPlayState->actorCtx.sceneFlags.chest);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("clearedRoom");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##clearedRoom",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.clearedRoom = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##clearedRoom",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.clearedRoom = 0;
            }
            UIWidgets::DrawFlagArray32("##clearedRoom", gPlayState->actorCtx.sceneFlags.clearedRoom);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("clearedRoomTemp");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##clearedRoomTemp",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.clearedRoomTemp = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##clearedRoomTemp",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.clearedRoomTemp = 0;
            }
            UIWidgets::DrawFlagArray32("##clearedRoomTemp", gPlayState->actorCtx.sceneFlags.clearedRoomTemp);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible[0]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible0",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[0] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible0",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[0] = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible0", gPlayState->actorCtx.sceneFlags.collectible[0]);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible[1]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible1",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[1] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible1",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[1] = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible1", gPlayState->actorCtx.sceneFlags.collectible[1]);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible[2]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible2",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[2] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible2",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[2] = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible2", gPlayState->actorCtx.sceneFlags.collectible[2]);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible[3]");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible3",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[3] = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible3",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gPlayState->actorCtx.sceneFlags.collectible[3] = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible3", gPlayState->actorCtx.sceneFlags.collectible[3]);
            ImGui::EndGroup();
            break;
        case 1: // weekEventReg
            for (int i = 0; i < 100; i++) {
                ImGui::PushID(i);
                ImGui::Text("%02d", i);
                ImGui::SameLine();
                UIWidgets::DrawFlagArray8("##", gSaveContext.save.saveInfo.weekEventReg[i]);
                ImGui::PopID();
            }
            break;
        case 2: // eventInf
            for (int i = 0; i < 8; i++) {
                ImGui::PushID(i);
                ImGui::Text("%02d", i);
                ImGui::SameLine();
                UIWidgets::DrawFlagArray8("##", gSaveContext.eventInf[i]);
                ImGui::PopID();
            }
            break;
        case 3: // scenesVisible
            if (UIWidgets::Button("All##scenesVisible",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                for (int i = 0; i < 7; i++) {
                    gSaveContext.save.saveInfo.scenesVisible[i] = UINT32_MAX;
                }
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##scenesVisible",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                for (int i = 0; i < 7; i++) {
                    gSaveContext.save.saveInfo.scenesVisible[i] = 0;
                }
            }
            for (int i = 0; i < 7; i++) {
                ImGui::PushID(i);
                UIWidgets::DrawFlagArray32("##", gSaveContext.save.saveInfo.scenesVisible[i]);
                ImGui::PopID();
            }
            break;
        case 4: // owlActivation
            if (UIWidgets::Button("All##owlActivationFlags",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.playerData.owlActivationFlags = UINT16_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##owlActivationFlags",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.playerData.owlActivationFlags = 0;
            }
            UIWidgets::DrawFlagArray16("##owlActivationFlags",
                                       gSaveContext.save.saveInfo.playerData.owlActivationFlags);
            break;
        case 5: // permanentSceneFlags
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("chest");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##chest",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].chest = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##chest",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].chest = 0;
            }
            UIWidgets::DrawFlagArray32("##chest", gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].chest);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switch0");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switch0",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch0 = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switch0",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch0 = 0;
            }
            UIWidgets::DrawFlagArray32("##switch0",
                                       gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch0);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].collectible = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].collectible = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible",
                                       gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].collectible);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switch1");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switch1",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch1 = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switch1",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch1 = 0;
            }
            UIWidgets::DrawFlagArray32("##switch1",
                                       gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].switch1);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("clearedRoom");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##clearedRoom",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].clearedRoom = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##clearedRoom",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].clearedRoom = 0;
            }
            UIWidgets::DrawFlagArray32("##clearedRoom",
                                       gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].clearedRoom);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("unk_14");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##unk_14",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].unk_14 = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##unk_14",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].unk_14 = 0;
            }
            UIWidgets::DrawFlagArray32("##unk_14",
                                       gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].unk_14);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("rooms");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##rooms",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].rooms = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##rooms",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].rooms = 0;
            }
            UIWidgets::DrawFlagArray32("##rooms", gSaveContext.save.saveInfo.permanentSceneFlags[selectedScene].rooms);
            ImGui::EndGroup();
            break;
        case 6: // cycleSceneFlags
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("chest");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##chest",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].chest = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##chest",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].chest = 0;
            }
            UIWidgets::DrawFlagArray32("##chest", gSaveContext.cycleSceneFlags[selectedScene].chest);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switch0");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switch0",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].switch0 = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switch0",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].switch0 = 0;
            }
            UIWidgets::DrawFlagArray32("##switch0", gSaveContext.cycleSceneFlags[selectedScene].switch0);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("collectible");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##collectible",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].collectible = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##collectible",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].collectible = 0;
            }
            UIWidgets::DrawFlagArray32("##collectible", gSaveContext.cycleSceneFlags[selectedScene].collectible);
            ImGui::EndGroup();
            ImGui::SameLine(0, 10.0f);
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("switch1");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##switch1",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].switch1 = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##switch1",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].switch1 = 0;
            }
            UIWidgets::DrawFlagArray32("##switch1", gSaveContext.cycleSceneFlags[selectedScene].switch1);
            ImGui::EndGroup();
            ImGui::BeginGroup();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("clearedRoom");
            ImGui::SameLine(110);
            if (UIWidgets::Button("All##clearedRoom",
                                  { .color = UIWidgets::Colors::Gray, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].clearedRoom = UINT32_MAX;
            }
            ImGui::SameLine();
            if (UIWidgets::Button("Clear##clearedRoom",
                                  { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline })) {
                gSaveContext.cycleSceneFlags[selectedScene].clearedRoom = 0;
            }
            UIWidgets::DrawFlagArray32("##clearedRoom", gSaveContext.cycleSceneFlags[selectedScene].clearedRoom);
            ImGui::EndGroup();
            break;
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
}

void SaveEditorWindow::DrawElement() {
    if (ImGui::BeginTabBar("SaveContextTabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
        if (ImGui::BeginTabItem("General")) {
            DrawGeneralTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Items & Masks")) {
            DrawItemsAndMasksTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Quest Status")) {
            DrawQuestStatusTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Dungeon Items")) {
            DrawDungeonItemTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Reg Editor")) {
            DrawRegEditorTab();
            ImGui::EndTabItem();
        }

        // if (ImGui::BeginTabItem("Equipment")) {
        //     DrawEquipmentTab();
        //     ImGui::EndTabItem();
        // }

        if (ImGui::BeginTabItem("Flags")) {
            DrawFlagsTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Player")) {
            DrawPlayerTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void SaveEditorWindow::InitElement() {
    initSafeItemsForInventorySlot();
}
