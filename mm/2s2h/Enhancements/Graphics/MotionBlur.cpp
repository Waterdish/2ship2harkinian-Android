#include <libultraship/bridge.h>

#include "2s2h/BenGui/UIWidgets.hpp"

extern "C" {
#include "z64.h"
}

const char* motionBlurOptions[] = { "Dynamic (default)", "Always Off", "Always On" };

void MotionBlur_RenderMenuOptions() {
    ImGui::SeparatorText("Motion Blur");
    UIWidgets::CVarCombobox("Motion Blur Mode", "gEnhancements.Graphics.MotionBlur.Mode", motionBlurOptions,
                            {
                                .labelPosition = UIWidgets::LabelPosition::None,
                            });

    UIWidgets::CVarCheckbox(
        "Interpolate", "gEnhancements.Graphics.MotionBlur.Interpolate",
        { .tooltip =
              "Change motion blur capture to also happen on interpolated frames instead of only on game frames.\n"
              "This notably reduces the overall motion blur strength but smooths out the trails." });

    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 0) {
        UIWidgets::Checkbox("On/Off", (bool*)&R_MOTION_BLUR_ENABLED);
        if (R_MOTION_BLUR_ENABLED) {
            int32_t motionBlurStrength = R_MOTION_BLUR_ALPHA;
            if (UIWidgets::SliderInt("Strength", &motionBlurStrength, 0, 255)) {
                R_MOTION_BLUR_ALPHA = motionBlurStrength;
            }
        }
    }
    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 2) {
        UIWidgets::CVarSliderInt("Strength", "gEnhancements.Graphics.MotionBlur.Strength", 0, 255, 180);
    }
}

extern "C" void MotionBlur_Override(u8* status, s32* alpha) {
    if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 1) {
        *status = 0;
        *alpha = 0;
    } else if (CVarGetInteger("gEnhancements.Graphics.MotionBlur.Mode", 0) == 2) {
        if (*status == 0)
            *status = 2;
        *alpha = CVarGetInteger("gEnhancements.Graphics.MotionBlur.Strength", 180);
    }
}
