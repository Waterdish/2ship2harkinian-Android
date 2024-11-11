#include "Menu.h"
#include "BenPort.h"
#include "BenGui.hpp"
#include "UIWidgets.hpp"
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "window/gui/GuiMenuBar.h"
#include "window/gui/GuiElement.h"
#include "DeveloperTools/SaveEditor.h"
#include "DeveloperTools/ActorViewer.h"
#include "DeveloperTools/CollisionViewer.h"
#include "DeveloperTools/EventLog.h"
#include "HudEditor.h"

#include "SearchableMenuItems.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
}
std::vector<ImVec2> windowTypeSizes = { {} };

namespace BenGui {

extern std::shared_ptr<HudEditorWindow> mHudEditorWindow;
extern std::shared_ptr<Ship::GuiWindow> mStatsWindow;
extern std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
extern std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
extern std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
extern std::shared_ptr<EventLogWindow> mEventLogWindow;
extern std::shared_ptr<BenInputEditorWindow> mBenInputEditorWindow;

extern std::shared_ptr<std::vector<Ship::WindowBackend>> availableWindowBackends;
extern std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
extern Ship::WindowBackend configWindowBackend;
extern void UpdateWindowBackendObjects();

// BENTODO: Not implemented yet
// UIWidgets::CVarCheckbox("Widescreen Actor Culling",
//                         "gEnhancements.Graphics.ActorCullingAccountsForWidescreen",
//                         { .tooltip = "Adjusts the culling planes to account for widescreen resolutions. "
//                                      "This may have unintended side effects." });

// if (gPlayState != NULL) {
//     ImGui::Separator();
//     SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_ENABLE);
//     if (gPlayState->frameAdvCtx.enabled) {
//         SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_SINGLE);
//         SearchMenuGetItem(MENU_ITEM_FRAME_ADVANCE_HOLD);
//         if (ImGui::IsItemActive()) {
//             CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
//         }
//     }
// }
// ImGui::PushStyleColor(ImGuiCol_Button, menuTheme[menuThemeIndex]);
// RenderWarpPointSection();
// ImGui::PopStyleColor(1);
//}

BenMenu::BenMenu(const std::string& consoleVariable, const std::string& name) : GuiWindow(consoleVariable, name) {
}

void BenMenu::InitElement() {
    popped = CVarGetInteger("gSettings.Menu.Popout", 0);
    poppedSize.x = CVarGetInteger("gSettings.Menu.PoppedWidth", 1280);
    poppedSize.y = CVarGetInteger("gSettings.Menu.PoppedHeight", 800);
    poppedPos.x = CVarGetInteger("gSettings.Menu.PoppedPos.x", 0);
    poppedPos.y = CVarGetInteger("gSettings.Menu.PoppedPos.y", 0);
    AddSettings();
    AddEnhancements();
    AddDevTools();

    menuEntries = { { "Settings", settingsSidebar, "gSettings.Menu.SettingsSidebarIndex" },
                    { "Enhancements", enhancementsSidebar, "gSettings.Menu.EnhancementsSidebarIndex" },
                    { "Developer Tools", devToolsSidebar, "gSettings.Menu.DevToolsSidebarIndex" } };

    UpdateWindowBackendObjects();
}

void BenMenu::UpdateElement() {
}

bool ModernMenuSidebarEntry(std::string label) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 pos = window->DC.CursorPos;
    const ImGuiID sidebarId = window->GetID(std::string(label + "##Sidebar").c_str());
    ImVec2 labelSize = ImGui::CalcTextSize(label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    pos.y += style.FramePadding.y;
    pos.x = window->WorkRect.GetCenter().x - labelSize.x / 2;
    ImRect bb = { pos - style.FramePadding, pos + labelSize + style.FramePadding };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, sidebarId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, sidebarId, &hovered, &held);
    if (pressed) {
        ImGui::MarkItemEdited(sidebarId);
    }
    window->DrawList->AddRectFilled(pos - style.FramePadding, pos + labelSize + style.FramePadding,
                                    ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive
                                                       : hovered         ? ImGuiCol_ButtonHovered
                                                                         : ImGuiCol_Button),
                                    3.0f);
    UIWidgets::RenderText(pos, label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    return pressed;
}

bool ModernMenuHeaderEntry(std::string label) {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 pos = window->DC.CursorPos;
    const ImGuiID headerId = window->GetID(std::string(label + "##Header").c_str());
    ImVec2 labelSize = ImGui::CalcTextSize(label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    ImRect bb = { pos, pos + labelSize + style.FramePadding * 2 };
    ImGui::ItemSize(bb, style.FramePadding.y);
    ImGui::ItemAdd(bb, headerId);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, headerId, &hovered, &held);
    window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                    ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive
                                                       : hovered         ? ImGuiCol_ButtonHovered
                                                                         : ImGuiCol_Button),
                                    3.0f);
    pos += style.FramePadding;
    UIWidgets::RenderText(pos, label.c_str(), ImGui::FindRenderedTextEnd(label.c_str()), true);
    return pressed;
}

void BenMenu::Draw() {
    if (!IsVisible()) {
        return;
    }
    DrawElement();
    // Sync up the IsVisible flag if it was changed by ImGui
    SyncVisibilityConsoleVariable();
}

void BenMenu::DrawElement() {
    for (auto& [reason, info] : disabledMap) {
        info.active = info.evaluation(info);
    }
    menuThemeIndex = static_cast<ColorOption>(CVarGetInteger("gSettings.MenuTheme", 3));

    windowHeight = ImGui::GetMainViewport()->WorkSize.y;
    windowWidth = ImGui::GetMainViewport()->WorkSize.x;
    auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    bool popout = CVarGetInteger("gSettings.Menu.Popout", 0) && allowPopout;
    if (popout) {
        windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking;
    }
    if (popout != popped) {
        if (popout) {
            windowHeight = poppedSize.y;
            windowWidth = poppedSize.x;
            ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) },
                                     ImGuiCond_Always);
            ImGui::SetNextWindowPos(poppedPos, ImGuiCond_Always);
        } else if (popped) {
            CVarSetFloat("gSettings.Menu.PoppedWidth", poppedSize.x);
            CVarSetFloat("gSettings.Menu.PoppedHeight", poppedSize.y);
            CVarSave();
        }
    }
    popped = popout;
    auto windowCond = ImGuiCond_Always;
    if (!popout) {
        ImGui::SetNextWindowSize({ static_cast<float>(windowWidth), static_cast<float>(windowHeight) }, windowCond);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), windowCond, { 0.5f, 0.5f });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    }
    if (!ImGui::Begin("Main Menu", NULL, windowFlags | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        if (!popout) {
            ImGui::PopStyleVar();
        }
        ImGui::End();
        return;
    }
    if (popped != popout) {
        if (!popout) {
            ImGui::PopStyleVar();
        }
        CVarSetInteger("gSettings.Menu.Popout", popped);
        CVarSetFloat("gSettings.Menu.PoppedWidth", poppedSize.x);
        CVarSetFloat("gSettings.Menu.PoppedHeight", poppedSize.y);
        CVarSetFloat("gSettings.Menu.PoppedPos.x", poppedSize.x);
        CVarSetFloat("gSettings.Menu.PoppedPos.y", poppedSize.y);
        CVarSave();
        ImGui::End();
        return;
    }
    ImGui::PushFont(OTRGlobals::Instance->fontStandardLargest);
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStyle& style = ImGui::GetStyle();
    windowHeight = window->WorkRect.GetHeight();
    windowWidth = window->WorkRect.GetWidth();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    auto sectionCount = menuEntries.size();
    const char* headerCvar = "gSettings.Menu.SelectedHeader";
    uint8_t headerIndex = CVarGetInteger(headerCvar, 0);
    ImVec2 pos = window->DC.CursorPos;
    float centerX = pos.x + windowWidth / 2 - (style.ItemSpacing.x * (sectionCount + 1));
    std::vector<ImVec2> headerSizes;
    float headerWidth = 200.0f + style.ItemSpacing.x;
    for (int i = 0; i < sectionCount; i++) {
        ImVec2 size = ImGui::CalcTextSize(menuEntries.at(i).label.c_str());
        headerSizes.push_back(size);
        headerWidth += size.x + style.FramePadding.x * 2;
        if (i + 1 < sectionCount) {
            headerWidth += style.ItemSpacing.x;
        }
    }
    ImVec2 menuSize = { std::fminf(1280, windowWidth), std::fminf(800, windowHeight) };
    pos += window->WorkRect.GetSize() / 2 - menuSize / 2;
    ImGui::SetNextWindowPos(pos);
    ImGui::BeginChild("Menu Block", menuSize,
                      ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    std::vector<SidebarEntry> sidebar;
    float headerHeight = headerSizes.at(0).y + style.FramePadding.y * 2;
    ImVec2 buttonSize = ImGui::CalcTextSize(ICON_FA_TIMES_CIRCLE) + style.FramePadding * 2;
    bool scrollbar = false;
    if (headerWidth > menuSize.x - buttonSize.x * 3 - style.ItemSpacing.x * 3) {
        headerHeight += style.ScrollbarSize;
        scrollbar = true;
    }
    if (UIWidgets::Button(ICON_FA_TIMES_CIRCLE, { .size = UIWidgets::Sizes::Inline, .tooltip = "Close Menu (Esc)" })) {
        ToggleVisibility();
    }
    ImGui::SameLine();
    ImGui::SetNextWindowSizeConstraints({ 0, headerHeight }, { headerWidth, headerHeight });
    ImVec2 headerSelSize = { menuSize.x - buttonSize.x * 3 - style.ItemSpacing.x * 3, headerHeight };
    if (scrollbar) {
        headerSelSize.y += style.ScrollbarSize;
    }
    bool autoFocus = CVarGetInteger("gSettings.SearchAutofocus", 0);
    bool headerSearch = !CVarGetInteger("gSettings.SidebarSearch", 0);
    ImGui::BeginChild("Header Selection", headerSelSize,
                      ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < sectionCount; i++) {
        auto entry = menuEntries.at(i);
        uint8_t nextIndex = i;
        UIWidgets::PushStyleButton(menuTheme[menuThemeIndex]);
        if (headerIndex != i) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
        }
        if (ModernMenuHeaderEntry(entry.label)) {
            if (autoFocus) {
                menuSearch.Clear();
            }
            CVarSetInteger(headerCvar, i);
            CVarSave();
            nextIndex = i;
        }
        if (headerIndex != i) {
            ImGui::PopStyleColor();
        }
        UIWidgets::PopStyleButton();
        if (headerIndex == i) {
            sidebar = entry.sidebarEntries;
        }
        if (i + 1 < sectionCount) {
            ImGui::SameLine();
        }
        if (nextIndex != i) {
            headerIndex = nextIndex;
        }
    }
    std::string menuSearchText = "";
    if (headerSearch) {
        ImGui::SameLine();
        if (autoFocus && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() &&
            !ImGui::IsMouseClicked(0)) {
            ImGui::SetKeyboardFocusHere(0);
        }
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0, 0, 0, 0 });
        menuSearch.Draw("##search", 200.0f);
        menuSearchText = menuSearch.InputBuf;
        menuSearchText.erase(std::remove(menuSearchText.begin(), menuSearchText.end(), ' '), menuSearchText.end());
        if (menuSearchText.length() < 1) {
            ImGui::SameLine(headerWidth - 200.0f + style.ItemSpacing.x);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "Search...");
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    ImGui::SameLine(menuSize.x - (buttonSize.x * 2) - style.ItemSpacing.x);
    if (UIWidgets::Button(ICON_FA_UNDO, { .color = UIWidgets::Colors::Red,
                                          .size = UIWidgets::Sizes::Inline,
                                          .tooltip = "Reset"
#ifdef __APPLE__
                                                     " (Command-R)"
#elif !defined(__SWITCH__) && !defined(__WIIU__)
                                                     " (Ctrl+R)"
#else
                                                     ""
#endif
                                        })) {
        std::reinterpret_pointer_cast<Ship::ConsoleWindow>(
            Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
            ->Dispatch("reset");
    }
    ImGui::SameLine();
    if (UIWidgets::Button(
            ICON_FA_POWER_OFF,
            { .color = UIWidgets::Colors::Red, .size = UIWidgets::Sizes::Inline, .tooltip = "Quit 2S2H" })) {
        if (!popped) {
            ToggleVisibility();
        }
        Ship::Context::GetInstance()->GetWindow()->Close();
    }
    ImGui::PopStyleVar();

    pos.y += headerHeight + style.ItemSpacing.y;
    pos.x = centerX - menuSize.x / 2 + (style.ItemSpacing.x * (sectionCount + 1));
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ menuSize.x, 4 }, ImGui::GetColorU32({ 255, 255, 255, 255 }),
                                    true, style.WindowRounding);
    pos.y += style.ItemSpacing.y;
    float sectionHeight = menuSize.y - headerHeight - 4 - style.ItemSpacing.y * 2;
    float columnHeight = sectionHeight - style.ItemSpacing.y * 4;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing * 2);
    float sidebarWidth = 200 - style.ItemSpacing.x;

    const char* sidebarCvar = menuEntries.at(headerIndex).sidebarCvar;

    uint8_t sectionIndex = CVarGetInteger(sidebarCvar, 0);
    if (sectionIndex > sidebar.size() - 1)
        sectionIndex = sidebar.size() - 1;
    if (sectionIndex < 0)
        sectionIndex = 0;
    float sectionCenterX = pos.x + (sidebarWidth / 2);
    float topY = pos.y;
    ImGui::SetNextWindowSizeConstraints({ sidebarWidth, 0 }, { sidebarWidth, columnHeight });
    ImGui::BeginChild((menuEntries.at(headerIndex).label + " Section").c_str(), { sidebarWidth, columnHeight * 3 },
                      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, ImGuiWindowFlags_NoTitleBar);
    for (size_t i = 0; i < sidebar.size(); i++) {
        auto sidebarEntry = sidebar.at(i);
        uint8_t nextIndex = i;
        UIWidgets::PushStyleButton(menuTheme[menuThemeIndex]);
        if (sectionIndex != i) {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });
        }
        if (ModernMenuSidebarEntry(sidebarEntry.label)) {
            if (autoFocus) {
                menuSearch.Clear();
            }
            CVarSetInteger(sidebarCvar, i);
            CVarSave();
            nextIndex = i;
        }
        if (sectionIndex != i) {
            ImGui::PopStyleColor();
        }
        UIWidgets::PopStyleButton();
        if (nextIndex != i) {
            sectionIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::PushFont(OTRGlobals::Instance->fontMonoLarger);
    pos = ImVec2{ sectionCenterX + (sidebarWidth / 2), topY } + style.ItemSpacing * 2;
    window->DrawList->AddRectFilled(pos, pos + ImVec2{ 4, sectionHeight - style.FramePadding.y * 2 },
                                    ImGui::GetColorU32({ 255, 255, 255, 255 }), true, style.WindowRounding);
    pos.x += 4 + style.ItemSpacing.x;
    ImGui::SetNextWindowPos(pos + style.ItemSpacing);
    float sectionWidth = menuSize.x - sidebarWidth - 4 - style.ItemSpacing.x * 4;
    std::string sectionMenuId = sidebar.at(sectionIndex).label + " Settings";
    int columns = sidebar.at(sectionIndex).columnCount;
    size_t columnFuncs = sidebar.at(sectionIndex).columnWidgets.size();
    if (windowWidth < 800) {
        columns = 1;
    }
    float columnWidth = (sectionWidth - style.ItemSpacing.x * columns) / columns;
    bool useColumns = columns > 1;
    if (!useColumns || (headerSearch && menuSearchText.length() > 0)) {
        ImGui::SameLine();
        ImGui::SetNextWindowSizeConstraints({ sectionWidth, 0 }, { sectionWidth, columnHeight });
        ImGui::BeginChild(sectionMenuId.c_str(), { sectionWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY,
                          ImGuiWindowFlags_NoTitleBar);
    }
    if (headerSearch && menuSearchText.length() > 0) {
        ImGui::BeginChild("Search Results");
        int searchCount = 0;
        for (auto& [menuLabel, menuSidebar, cvar] : menuEntries) {
            for (auto& sidebar : menuSidebar) {
                for (auto& widgets : sidebar.columnWidgets) {
                    int column = 1;
                    for (auto& info : widgets) {
                        if (info.widgetType == WIDGET_SEPARATOR || info.widgetType == WIDGET_SEPARATOR_TEXT ||
                            info.isHidden) {
                            continue;
                        }
                        std::string widgetStr = std::string(info.widgetName) + std::string(info.widgetTooltip);
                        std::transform(menuSearchText.begin(), menuSearchText.end(), menuSearchText.begin(), ::tolower);
                        std::transform(widgetStr.begin(), widgetStr.end(), widgetStr.begin(), ::tolower);
                        widgetStr.erase(std::remove(widgetStr.begin(), widgetStr.end(), ' '), widgetStr.end());
                        if (widgetStr.find(menuSearchText) != std::string::npos) {
                            SearchMenuGetItem(info);
                            searchCount++;
                            ImGui::PushStyleColor(ImGuiCol_Text, UIWidgets::Colors::Gray);
                            std::string origin = fmt::format("  ({} -> {}, Clmn {})", menuLabel, sidebar.label, column);
                            ImGui::Text("%s", origin.c_str());
                            ImGui::PopStyleColor();
                        }
                    }
                    column++;
                }
            }
        }

        if (searchCount == 0) {
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("No results found").x) / 2);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "No results found");
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Clear Search").x) / 2 - 10.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
            if (UIWidgets::Button("Clear Search", { .size = UIWidgets::Sizes::Inline })) {
                menuSearch.Clear();
            }
        }

        ImGui::EndChild();
    } else {
        for (int i = 0; i < columnFuncs; i++) {
            std::string sectionId = fmt::format("{} Column {}", sectionMenuId, i);
            if (useColumns) {
                ImGui::SetNextWindowSizeConstraints({ columnWidth, 0 }, { columnWidth, columnHeight });
                ImGui::BeginChild(sectionId.c_str(), { columnWidth, windowHeight * 4 }, ImGuiChildFlags_AutoResizeY,
                                  ImGuiWindowFlags_NoTitleBar);
            }
            for (auto& entry : sidebar.at(sectionIndex).columnWidgets.at(i)) {
                SearchMenuGetItem(entry);
            }
            if (useColumns) {
                ImGui::EndChild();
            }
            if (i < columns - 1) {
                ImGui::SameLine();
            }
        }
    }
    if (!useColumns || menuSearchText.length() > 0) {
        ImGui::EndChild();
    }
    ImGui::PopFont();
    ImGui::PopFont();

    if (!popout) {
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    if (popout) {
        poppedSize = ImGui::GetWindowSize();
        poppedPos = ImGui::GetWindowPos();
    }
    ImGui::End();
}
} // namespace BenGui
