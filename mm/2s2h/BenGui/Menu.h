#pragma once

#include <libultraship/libultraship.h>
#include "UIWidgets.hpp"

namespace BenGui {
class BenMenu : public Ship::GuiWindow {
  public:
    using Ship::GuiWindow::GuiWindow;

    BenMenu(const std::string& consoleVariable, const std::string& name);

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
    void Draw() override;

  protected:
    ImVec2 mOriginalSize;
    std::string mName;
    uint32_t mWindowFlags;

  private:
    bool allowPopout = true; // PortNote: should be set to false on small screen ports
    bool popped;
    ImVec2 poppedSize;
    ImVec2 poppedPos;
    float windowHeight;
    float windowWidth;
};
} // namespace BenGui