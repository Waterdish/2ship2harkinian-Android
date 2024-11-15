#include "window/gui/Gui.h"
#include "window/gui/GuiWindow.h"
#include <array>

typedef enum class TrackerWindowType : uint8_t {
    Floating,
    Window,
} TrackerWindowType;

typedef enum class ItemTrackerDisplayType : int8_t {
    Hidden,
    MainWindow,
    Separate,
} ItemTrackerDisplayType;

typedef enum {
    SECTION_INVENTORY,
    SECTION_MASKS,
    SECTION_SONGS,
    SECTION_DUNGEON,
    SECTION_MAX,
} ItemTrackerSection;

typedef enum {
    DrawCurrent,
    DrawCurCapacity,
    DrawMaxCapacity,
    DrawCapacityMax,
} ItemTrackerCapacityMode;

class ItemTrackerWindow : public Ship::GuiWindow {
    typedef struct AmmoInfo {
        uint8_t cur;
        uint8_t curCap;
        uint8_t maxCap;
    } AmmoInfo;

  public:
    using GuiWindow::GuiWindow;
    ~ItemTrackerWindow();
    void Draw() override;

    ImVec4* GetBgColorPtr();
    float* GetIconSizePtr();
    float* GetIconSpacingPtr();
    float* GetTextSizePtr();
    float* GetTextOffsetPtr();
    TrackerWindowType* GetWindowTypePtr();
    bool* GetIsDraggablePtr();
    bool* GetOnlyShowPausedPtr();
    ItemTrackerDisplayType* GetDrawModePtr(ItemTrackerSection type);
    bool* GetCapacityModePtr(ItemTrackerCapacityMode mode);
    void LoadSettings();

  protected:
    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override{};

  private:
    void BeginFloatingWindows(const char* name, ImGuiWindowFlags flags = 0);
    void DrawItemsInRows(int columns = 6);
    int DrawItems(int columns, int startAt);
    int DrawMasks(int columns, int startAt);
    int DrawSongs(int columns, int startAt);
    void DrawNote(size_t songIndex, bool drawFaded);
    int DrawDungeonItemsVert(int columns, int startAt);
    void DrawAmmoCount(int itemId, const ImVec2& iconPos);
    bool HasAmmoCount(int itemId);
    AmmoInfo GetAmmoInfo(int itemId);

    ImVec4 mBgColor;
    float mIconSize = 36.0f;
    float mIconSpacing = 12.0f;
    float mTextSize = 13.0f;
    float mTextOffset = 0.0f;
    TrackerWindowType mWindowType = TrackerWindowType::Floating;
    bool mIsDraggable = false;
    bool mOnlyDrawPaused = false;
    std::array<bool, DrawCapacityMax> mCapacityModes;
    std::array<ItemTrackerDisplayType, SECTION_MAX> mItemDrawModes;
};
