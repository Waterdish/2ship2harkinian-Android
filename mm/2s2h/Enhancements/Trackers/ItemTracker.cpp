#include "ItemTracker.h"
#include "Context.h"
#include "config/Config.h"
#include "assets/archives/icon_item_static/icon_item_static_yar.h"
#include "assets/archives/icon_item_24_static/icon_item_24_static_yar.h"

#define CFG_TRACKER_ITEM(var) ("ItemTracker." var)

using namespace Ship;

ItemTrackerWindow::~ItemTrackerWindow() {
    auto config = Context::GetInstance()->GetConsoleVariables();

    config->SetFloat(CFG_TRACKER_ITEM("BgColorR"), mBgColor.x);
    config->SetFloat(CFG_TRACKER_ITEM("BgColorG"), mBgColor.y);
    config->SetFloat(CFG_TRACKER_ITEM("BgColorB"), mBgColor.z);
    config->SetFloat(CFG_TRACKER_ITEM("BgColorA"), mBgColor.w);
    config->SetFloat(CFG_TRACKER_ITEM("IconSize"), mIconSize);
    config->SetFloat(CFG_TRACKER_ITEM("IconSpacing"), mIconSpacing);
    config->SetFloat(CFG_TRACKER_ITEM("TextSize"), mTextSize);
    config->SetFloat(CFG_TRACKER_ITEM("TextOffset"), mTextOffset);
    config->SetInteger(CFG_TRACKER_ITEM("WindowType"), (int8_t)mWindowType);
    config->SetInteger(CFG_TRACKER_ITEM("IsDraggable"), mIsDraggable);
    config->SetInteger(CFG_TRACKER_ITEM("OnlyDrawPaused"), mOnlyDrawPaused);
    config->SetInteger(CFG_TRACKER_ITEM("DrawCurrentAmmo"), mCapacityModes[ItemTrackerCapacityMode::DrawCurrent]);
    config->SetInteger(CFG_TRACKER_ITEM("DrawMaxAmmo"), mCapacityModes[ItemTrackerCapacityMode::DrawCurCapacity]);
    config->SetInteger(CFG_TRACKER_ITEM("DrawMaxCapacity"), mCapacityModes[ItemTrackerCapacityMode::DrawMaxCapacity]);
    config->SetInteger(CFG_TRACKER_ITEM("InventoryDrawMode"), (int8_t)mItemDrawModes[SECTION_INVENTORY]);
    config->SetInteger(CFG_TRACKER_ITEM("MasksDrawMode"), (int8_t)mItemDrawModes[SECTION_MASKS]);
    config->SetInteger(CFG_TRACKER_ITEM("SongsDrawMode"), (int8_t)mItemDrawModes[SECTION_SONGS]);
    config->SetInteger(CFG_TRACKER_ITEM("DungeonDrawMode"), (int8_t)mItemDrawModes[SECTION_DUNGEON]);

    config->Save();
}

void ItemTrackerWindow::LoadSettings() {
    auto config = Context::GetInstance()->GetConsoleVariables();

    mCapacityModes.fill(false);

    mBgColor.x = config->GetFloat(CFG_TRACKER_ITEM("BgColorR"), 0.0f);
    mBgColor.y = config->GetFloat(CFG_TRACKER_ITEM("BgColorG"), 0.0f);
    mBgColor.z = config->GetFloat(CFG_TRACKER_ITEM("BgColorB"), 0.0f);
    mBgColor.w = config->GetFloat(CFG_TRACKER_ITEM("BgColorA"), 0.0f);
    mIconSize = config->GetFloat(CFG_TRACKER_ITEM("IconSize"), 13.0f);
    mIconSpacing = config->GetFloat(CFG_TRACKER_ITEM("IconSpacing"), 12.0f);
    mTextSize = config->GetFloat(CFG_TRACKER_ITEM("TextSize"), 13.0f);
    mTextOffset = config->GetFloat(CFG_TRACKER_ITEM("TextOffset"), 0.0f);
    mWindowType =
        (TrackerWindowType)config->GetInteger(CFG_TRACKER_ITEM("WindowType"), (int8_t)TrackerWindowType::Floating);
    mIsDraggable = config->GetInteger(CFG_TRACKER_ITEM("IsDraggable"), false);
    mOnlyDrawPaused = config->GetInteger(CFG_TRACKER_ITEM("OnlyDrawPaused"), false);
    mCapacityModes[ItemTrackerCapacityMode::DrawCurrent] =
        config->GetInteger(CFG_TRACKER_ITEM("DrawCurrentAmmo"), false);
    mCapacityModes[ItemTrackerCapacityMode::DrawCurCapacity] =
        config->GetInteger(CFG_TRACKER_ITEM("DrawMaxAmmo"), false);
    mCapacityModes[ItemTrackerCapacityMode::DrawMaxCapacity] =
        config->GetInteger(CFG_TRACKER_ITEM("DrawMaxCapacity"), false);
    mItemDrawModes[SECTION_INVENTORY] = (ItemTrackerDisplayType)config->GetInteger(
        CFG_TRACKER_ITEM("InventoryDrawMode"), (int32_t)ItemTrackerDisplayType::Hidden);
    mItemDrawModes[SECTION_MASKS] = (ItemTrackerDisplayType)config->GetInteger(CFG_TRACKER_ITEM("MasksDrawMode"),
                                                                               (int32_t)ItemTrackerDisplayType::Hidden);
    mItemDrawModes[SECTION_SONGS] = (ItemTrackerDisplayType)config->GetInteger(CFG_TRACKER_ITEM("SongsDrawMode"),
                                                                               (int32_t)ItemTrackerDisplayType::Hidden);
    mItemDrawModes[SECTION_DUNGEON] = (ItemTrackerDisplayType)config->GetInteger(
        CFG_TRACKER_ITEM("DungeonDrawMode"), (int32_t)ItemTrackerDisplayType::Hidden);
}

void ItemTrackerWindow::BeginFloatingWindows(const char* name, ImGuiWindowFlags flags) {
    ImGuiWindowFlags windowFlags = flags;

    if (windowFlags == 0) {
        windowFlags |=
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize;
    }

    if (mWindowType == TrackerWindowType::Floating) {
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        windowFlags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar |
                       ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

        if (!mIsDraggable) {
            windowFlags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;
        }
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, mBgColor);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::Begin(name, nullptr, windowFlags);
}

void EndFloatingWindows() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::End();
}

static constexpr ImVec4 opaqueTex = { 1.0f, 1.0f, 1.0f, 1.0f };
static constexpr ImVec4 fadedTex = { 0.5f, 0.5f, 0.5f, 0.5f };

void DrawItem(char* tex, bool drawFaded, float itemSize) {
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(tex), ImVec2(itemSize, itemSize),
                 ImVec2(0, 0), ImVec2(1, 1), drawFaded ? fadedTex : opaqueTex);
}

static constexpr std::array<ImVec4, 5> songInfo = {
    ImVec4(0.588f, 1.0f, 0.392f, 1.0f), // QUEST_SONG_SONATA
    ImVec4(1.0f, 0.313f, 0.156f, 1.0f), // QUEST_SONG_LULLABY
    ImVec4(0.392f, 0.588f, 1.0f, 1.0f), // QUEST_SONG_BOSSA_NOVA
    ImVec4(1.0f, 0.627f, 0.0f, 1.0f),   // QUEST_SONG_ELEGY
    ImVec4(1.0f, 0.392f, 1.0f, 1.0f),   // QUEST_SONG_OATH
};

void ItemTrackerWindow::DrawNote(size_t songIndex, bool drawFaded) {
    ImVec4 color;
    // Scale the note to 24*36 from 16*24 because all other items assume 36*36.
    constexpr float noteTo36scale = 36.0f / 24.0f;

    const float iconScale = mIconSize / 36.0f;

    // Scale the note icon with the rest of the items.
    const ImVec2 scaledNoteSize(noteTo36scale * 16.0f * iconScale, noteTo36scale * 24.0f * iconScale);

    if (songIndex >= 5) {
        color = songInfo[songIndex - 5];
    } else {
        color = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    if (drawFaded) {
        color.w = .5f;
    }
    ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName(gItemIconSongNoteTex),
                 scaledNoteSize, ImVec2(0, 0), ImVec2(1, 1), color);
}

extern "C" {
#include "z64save.h"
#include "macros.h"
#include "z64.h"
extern void* gItemIcons[131];
extern uint8_t gItemSlots[77];
extern SaveContext gSaveContext;
extern u32 gBitFlags[32];
extern PlayState* gPlayState;
extern u16 gUpgradeCapacities[][4];
extern u32 gUpgradeMasks[];
extern u8 gUpgradeShifts[];
}

// The textures in gItemIcons aren't in the same order as the subscreen
static constexpr std::array<const char*, 24> sMaskTextures = {
    gItemIconPostmansHatTex,    gItemIconAllNightMaskTex,     gItemIconBlastMaskTex,    gItemIconStoneMaskTex,
    gItemIconGreatFairyMaskTex, gItemIconDekuMaskTex,         gItemIconKeatonMaskTex,   gItemIconBremenMaskTex,
    gItemIconBunnyHoodTex,      gItemIconDonGeroMaskTex,      gItemIconMaskOfScentsTex, gItemIconGoronMaskTex,
    gItemIconRomaniMaskTex,     gItemIconCircusLeaderMaskTex, gItemIconKafeisMaskTex,   gItemIconCouplesMaskTex,
    gItemIconMaskOfTruthTex,    gItemIconZoraMaskTex,         gItemIconKamaroMaskTex,   gItemIconGibdoMaskTex,
    gItemIconGaroMaskTex,       gItemIconCaptainsHatTex,      gItemIconGiantsMaskTex,   gItemIconFierceDeityMaskTex,
};

// The songs in the enum are in the wrong order and has the unused songs
static constexpr std::array<uint8_t, 10> sSongBits = {
    QUEST_SONG_TIME,   QUEST_SONG_HEALING, QUEST_SONG_EPONA,      QUEST_SONG_SOARING, QUEST_SONG_STORMS,
    QUEST_SONG_SONATA, QUEST_SONG_LULLABY, QUEST_SONG_BOSSA_NOVA, QUEST_SONG_ELEGY,   QUEST_SONG_OATH,
};

bool ItemTrackerWindow::HasAmmoCount(int itemId) {
    switch (itemId) {
        case ITEM_BOW:
        case ITEM_BOMB:
        case ITEM_BOMBCHU:
        case ITEM_DEKU_STICK:
        case ITEM_DEKU_NUT:
            return true;
        default:
            return false;
    }
}

ItemTrackerWindow::AmmoInfo ItemTrackerWindow::GetAmmoInfo(int itemId) {
    AmmoInfo info;
    switch (itemId) {
        case ITEM_BOW:
            info = { .cur = (uint8_t)AMMO(ITEM_BOW), .curCap = (uint8_t)CUR_CAPACITY(UPG_QUIVER), .maxCap = 50 };
            break;
        case ITEM_BOMB:
            info = { .cur = (uint8_t)AMMO(ITEM_BOMB), .curCap = (uint8_t)CUR_CAPACITY(UPG_BOMB_BAG), .maxCap = 40 };
            break;
        case ITEM_BOMBCHU:
            info = { .cur = (uint8_t)AMMO(ITEM_BOMBCHU), .curCap = (uint8_t)CUR_CAPACITY(UPG_BOMB_BAG), .maxCap = 40 };
            break;
        case ITEM_DEKU_STICK:
            info = { .cur = (uint8_t)AMMO(ITEM_DEKU_STICK),
                     .curCap = (uint8_t)CUR_CAPACITY(UPG_DEKU_STICKS),
                     .maxCap = 30 };
            break;
        case ITEM_DEKU_NUT:
            info = { .cur = (uint8_t)AMMO(ITEM_DEKU_NUT),
                     .curCap = (uint8_t)CUR_CAPACITY(UPG_DEKU_NUTS),
                     .maxCap = 40 };
            break;
        default:
            info = { 0 };
    }
    return info;
}

void ItemTrackerWindow::DrawAmmoCount(int itemId, const ImVec2& iconPos) {
    // Zeroing 16 bytes is a little more optimized than 10
    char ammoStr[16] = { 0 };
    char curStr[4] = { 0 };
    char curCapStr[4] = { 0 };
    char maxCapStr[4] = { 0 };
    AmmoInfo info;

    if (!HasAmmoCount(itemId)) {
        return;
    }

    info = GetAmmoInfo(itemId);

    if (mCapacityModes[ItemTrackerCapacityMode::DrawCurrent]) {
        snprintf(curStr, std::size(curStr), "%d", info.cur);
        if (mCapacityModes[ItemTrackerCapacityMode::DrawCurCapacity] ||
            mCapacityModes[ItemTrackerCapacityMode::DrawMaxCapacity]) {
            strncat(curStr, "/", 1);
        }
    }

    if (mCapacityModes[ItemTrackerCapacityMode::DrawCurCapacity]) {
        snprintf(curCapStr, std::size(curStr), "%d", info.curCap);
        if (mCapacityModes[ItemTrackerCapacityMode::DrawMaxCapacity]) {
            strncat(curCapStr, "/", 1);
        }
    }

    if (mCapacityModes[ItemTrackerCapacityMode::DrawMaxCapacity]) {
        snprintf(maxCapStr, std::size(curStr), "%d", info.maxCap);
    }

    ImGui::SetWindowFontScale(mTextSize / 13.0f);
    ImVec2 iconPos2 = ImGui::GetCursorScreenPos();

    snprintf(ammoStr, std::size(ammoStr), "%s%s%s", curStr, curCapStr, maxCapStr);

    float x = iconPos2.x + (mIconSize / 2.0f) - (ImGui::CalcTextSize(ammoStr).x / 2.0f);
    // Normalize the offset based on the icon being 36x36 to account for larger icons.
    ImGui::SetCursorScreenPos({ x, iconPos2.y - (mTextOffset / 36.0f) * mIconSize });

    ImGui::Text("%s", ammoStr);
}

int ItemTrackerWindow::DrawItems(int columns, int prevDrawnColumns) {
    int topPadding = 0;
    size_t i = 0;
    // Draw items minus bottles
    for (; i < 24 - BOTTLE_MAX; i++) {
        bool drawFaded = false;
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        char* texToDraw;
        const ImVec2 pos = { (column * (mIconSize + mIconSpacing) + 8.0f),
                             (row * (mIconSize + mIconSpacing)) + 8.0f + topPadding };

        ImGui::SetCursorPos(pos);

        if (gSaveContext.save.saveInfo.inventory.items[i] == ITEM_NONE) {
            if (column == 5) {
                // BENTODO these slots can hold multiple items. Something fun to do would be to draw the multitile
                // design here instead of nothing DrawMultiItem(nullptr, false, row, column);
                continue;
            }
            texToDraw = (char*)gItemIcons[i];
            drawFaded = true;
        } else {
            texToDraw = (char*)gItemIcons[gSaveContext.save.saveInfo.inventory.items[i]];
        }
        ImGui::BeginGroup();
        DrawItem(texToDraw, drawFaded, mIconSize);
        DrawAmmoCount((int)i, pos);
        ImGui::EndGroup();
    }

    // DrawBottles
    for (; i < 24; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (mIconSize + mIconSpacing) + 8.0f),
                                   (row * (mIconSize + mIconSpacing)) + 8.0f + topPadding));

        if (gSaveContext.save.saveInfo.inventory.items[i] == ITEM_NONE) {
            DrawItem(const_cast<char*>(gItemIconEmptyBottleTex), true, mIconSize);
        } else {
            DrawItem((char*)gItemIcons[gSaveContext.save.saveInfo.inventory.items[i]], false, mIconSize);
        }
    }
    return 4;
}

int ItemTrackerWindow::DrawMasks(int columns, int prevDrawnColumns) {
    int topPadding = 0;
    // Masks
    for (size_t i = 0; i < 24; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (mIconSize + mIconSpacing) + 8.0f),
                                   (row * (mIconSize + mIconSpacing)) + 8.0f + topPadding));

        DrawItem(const_cast<char*>(sMaskTextures[i]), gSaveContext.save.saveInfo.inventory.items[i + 24] == ITEM_NONE,
                 mIconSize);
    }
    return 4;
}

static int RoundDown(int orig, int nearest) {
    int res = orig % nearest;
    return orig - res;
}

int ItemTrackerWindow::DrawSongs(int columns, int prevDrawnColumns) {
    int topPadding = 0;

    for (size_t i = 0; i < sSongBits.size(); i++) {
        int row = prevDrawnColumns + (i / 5);
        int column = i % 5;
        ImGui::SetCursorPos(ImVec2((column * (mIconSize + mIconSpacing) + 8.0f),
                                   (row * (mIconSize + mIconSpacing)) + 8.0f + topPadding));

        DrawNote(i, !CHECK_QUEST_ITEM(sSongBits[i]));
    }
    return 2;
}

int ItemTrackerWindow::DrawDungeonItemsVert(int columns, int prevDrawnColumns) {
    int topPadding = 0;

    // The icon size is based on 36x36.
    float iconScale = mIconSize / 36.0f;
    // Yes this does nothing. But for other icon sizes it will.
    float squareIconSize = iconScale * 36.0f;
    float rectIconSize = iconScale * 36.0f;

    for (size_t i = 0; i < 4; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (squareIconSize + mIconSpacing) + 8.0f),
                                   (row * (squareIconSize + mIconSpacing)) + 8.0f + topPadding));
        DrawItem(static_cast<char*>(gItemIcons[i + ITEM_REMAINS_ODOLWA]), !CHECK_QUEST_ITEM(i), squareIconSize);
    }

    prevDrawnColumns++;

    for (size_t i = 0; i < 4; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (rectIconSize + mIconSpacing) + 8.0f),
                                   (row * (rectIconSize + mIconSpacing)) + 8.0f + topPadding));
        DrawItem(const_cast<char*>(gQuestIconBossKeyTex), !CHECK_DUNGEON_ITEM(DUNGEON_BOSS_KEY, i), rectIconSize);
    }

    prevDrawnColumns++;

    for (size_t i = 0; i < 4; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (rectIconSize + mIconSpacing) + 8.0f),
                                   (row * (rectIconSize + mIconSpacing)) + 8.0f + topPadding));
        DrawItem(const_cast<char*>(gQuestIconCompassTex), !CHECK_DUNGEON_ITEM(DUNGEON_COMPASS, i), rectIconSize);
    }

    prevDrawnColumns++;

    for (size_t i = 0; i < 4; i++) {
        int row = prevDrawnColumns + (i / columns);
        int column = i % columns;
        ImGui::SetCursorPos(ImVec2((column * (rectIconSize + mIconSpacing) + 8.0f),
                                   (row * (rectIconSize + mIconSpacing)) + 8.0f + topPadding));
        DrawItem(const_cast<char*>(gQuestIconDungeonMapTex), !CHECK_DUNGEON_ITEM(DUNGEON_MAP, i), rectIconSize);
    }

    return 4;
}

void ItemTrackerWindow::DrawItemsInRows(int columns) {
    float iconSize = mIconSize;
    float iconSpacing = mIconSpacing;
    int topPadding = 0;
    int mainWindowPos = 0;
    int advancedBy = 0;

    if (gPlayState == nullptr) {
        ImGui::Text("Item tracker not available");
        return;
    }
    if (mOnlyDrawPaused && gPlayState->pauseCtx.state == 0) {
        return;
    }
    if (mItemDrawModes[SECTION_INVENTORY] != ItemTrackerDisplayType::Hidden) {
        if (mItemDrawModes[SECTION_INVENTORY] == ItemTrackerDisplayType::Separate) {
            BeginFloatingWindows("Items");
        }
        advancedBy = DrawItems(6, mainWindowPos);
        if (mItemDrawModes[SECTION_INVENTORY] == ItemTrackerDisplayType::Separate) {
            EndFloatingWindows();
        } else {
            mainWindowPos += advancedBy;
        }
    }

    if (mItemDrawModes[SECTION_MASKS] != ItemTrackerDisplayType::Hidden) {
        int drawPos = mainWindowPos;
        if (mItemDrawModes[SECTION_MASKS] == ItemTrackerDisplayType::Separate) {
            drawPos = 0;
            BeginFloatingWindows("Masks");
        }
        advancedBy = DrawMasks(6, drawPos);
        if (mItemDrawModes[SECTION_MASKS] == ItemTrackerDisplayType::Separate) {
            EndFloatingWindows();
        } else {
            mainWindowPos += advancedBy;
        }
    }

    if (mItemDrawModes[SECTION_SONGS] != ItemTrackerDisplayType::Hidden) {
        int drawPos = mainWindowPos;
        if (mItemDrawModes[SECTION_SONGS] == ItemTrackerDisplayType::Separate) {
            drawPos = 0;
            BeginFloatingWindows("Songs");
        }
        advancedBy = DrawSongs(5, drawPos);
        if (mItemDrawModes[SECTION_SONGS] == ItemTrackerDisplayType::Separate) {
            EndFloatingWindows();
        } else {
            mainWindowPos += advancedBy;
        }
    }

    if (mItemDrawModes[SECTION_DUNGEON] != ItemTrackerDisplayType::Hidden) {
        int drawPos = mainWindowPos;
        if (mItemDrawModes[SECTION_DUNGEON] == ItemTrackerDisplayType::Separate) {
            drawPos = 0;
            BeginFloatingWindows("Dungeon Items");
        }
        advancedBy = DrawDungeonItemsVert(6, drawPos);
        if (mItemDrawModes[SECTION_DUNGEON] == ItemTrackerDisplayType::Separate) {
            EndFloatingWindows();
        } else {
            mainWindowPos += advancedBy;
        }
    }
}

ImVec4* ItemTrackerWindow::GetBgColorPtr() {
    return &mBgColor;
}

float* ItemTrackerWindow::GetIconSizePtr() {
    return &mIconSize;
}

float* ItemTrackerWindow::GetIconSpacingPtr() {
    return &mIconSpacing;
}

float* ItemTrackerWindow::GetTextSizePtr() {
    return &mTextSize;
}

float* ItemTrackerWindow::GetTextOffsetPtr() {
    return &mTextOffset;
}

TrackerWindowType* ItemTrackerWindow::GetWindowTypePtr() {
    return &mWindowType;
}

bool* ItemTrackerWindow::GetIsDraggablePtr() {
    return &mIsDraggable;
}

bool* ItemTrackerWindow::GetOnlyShowPausedPtr() {
    return &mOnlyDrawPaused;
}

ItemTrackerDisplayType* ItemTrackerWindow::GetDrawModePtr(ItemTrackerSection type) {
    return &mItemDrawModes[type];
}

bool* ItemTrackerWindow::GetCapacityModePtr(ItemTrackerCapacityMode mode) {
    return &mCapacityModes[mode];
}

void ItemTrackerWindow::Draw() {
    if (!IsVisible()) {
        return;
    }
    BeginFloatingWindows("Inventory Items Tracker");
    DrawItemsInRows();
    EndFloatingWindows();
}
void ItemTrackerWindow::InitElement() {
    LoadSettings();
}

void ItemTrackerWindow::DrawElement() {
}
