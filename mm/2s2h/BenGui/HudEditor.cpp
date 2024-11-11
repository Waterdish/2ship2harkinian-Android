
#include "HudEditor.h"
#include "macros.h"

extern "C" int16_t OTRGetRectDimensionFromLeftEdge(float v);
extern "C" int16_t OTRGetRectDimensionFromRightEdge(float v);

HudEditorElementID hudEditorActiveElement = HUD_EDITOR_ELEMENT_NONE;

HudEditorElement hudEditorElements[HUD_EDITOR_ELEMENT_MAX] = {
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_B, "B Button", "B", 167, 17, 100, 255, 120, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_C_LEFT, "C-Left Button", "CLeft", 227, 18, 255, 240, 0, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_C_DOWN, "C-Down Button", "CDown", 249, 34, 255, 240, 0, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_C_RIGHT, "C-Right Button", "CRight", 271, 18, 255, 240, 0, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_A, "A Button", "A", 191, 18, 100, 200, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_C_UP, "C-Up Button", "CUp", 254, 16, 255, 240, 0, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_D_PAD, "D-Pad", "DPad", 271, 55, 255, 255, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_MINIMAP, "Minimap", "Minimap", 295, 220, 0, 255, 255, 160),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_START, "Start Button", "Start", 136, 17, 255, 130, 60, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_CARROT, "Horse Carrots", "Carrots", 160, 64, 236, 92, 41, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_CLOCK, "Three Day Clock", "Clock", 160, 206, 255, 255, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_HEARTS, "Hearts", "Hearts", 30, 26, 255, 70, 50, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_MAGIC_METER, "Magic", "Magic", 18, 34, 0, 200, 0, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_TIMERS, "Timers", "Timers", 26, 46, 255, 255, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_TIMERS_MOON_CRASH, "Timer - Skull Kid", "SkullKidTimer", 115, 200, 255, 255,
                       255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_MINIGAME_COUNTER, "Minigames", "Minigames", 20, 67, 255, 255, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_RUPEE_COUNTER, "Rupees", "Rupees", 26, 206, 200, 255, 100, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_KEY_COUNTER, "Keys", "Keys", 26, 190, 255, 255, 255, 255),
    HUD_EDITOR_ELEMENT(HUD_EDITOR_ELEMENT_SKULLTULA_COUNTER, "Skulltulas", "Skulltulas", 26, 190, 255, 255, 255, 255),
};

extern "C" bool HudEditor_ShouldOverrideDraw() {
    return hudEditorActiveElement != HUD_EDITOR_ELEMENT_NONE &&
           CVarGetInteger(hudEditorElements[hudEditorActiveElement].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) !=
               HUD_EDITOR_ELEMENT_MODE_VANILLA;
}

extern "C" void HudEditor_SetActiveElement(HudEditorElementID id) {
    hudEditorActiveElement = id;
}

extern "C" bool HudEditor_IsActiveElementHidden() {
    return hudEditorActiveElement != HUD_EDITOR_ELEMENT_NONE
               ? CVarGetInteger(hudEditorElements[hudEditorActiveElement].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) ==
                     HUD_EDITOR_ELEMENT_MODE_HIDDEN
               : false;
}

extern "C" f32 HudEditor_GetActiveElementScale() {
    return hudEditorActiveElement != HUD_EDITOR_ELEMENT_NONE
               ? CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f)
               : 1.0f;
}

extern "C" void HudEditor_ModifyRectPosValuesFromBase(s16 baseX, s16 baseY, s16* rectLeft, s16* rectTop) {
    s16 offsetFromBaseX = *rectLeft - baseX;
    s16 offsetFromBaseY = *rectTop - baseY;
    *rectLeft = baseX + (offsetFromBaseX * CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f));
    *rectTop = baseY + (offsetFromBaseY * CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f));
}

void HudEditor_ModifyRectPosValuesFloat(f32* rectLeft, f32* rectTop) {
    f32 offsetFromBaseX = *rectLeft - hudEditorElements[hudEditorActiveElement].defaultX;
    f32 offsetFromBaseY = *rectTop - hudEditorElements[hudEditorActiveElement].defaultY;
    *rectLeft = CVarGetInteger(hudEditorElements[hudEditorActiveElement].xCvar,
                               hudEditorElements[hudEditorActiveElement].defaultX) +
                (offsetFromBaseX * CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f));
    *rectTop = CVarGetInteger(hudEditorElements[hudEditorActiveElement].yCvar,
                              hudEditorElements[hudEditorActiveElement].defaultY) +
               (offsetFromBaseY * CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f));

    if (CVarGetInteger(hudEditorElements[hudEditorActiveElement].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) ==
        HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT) {
        *rectLeft = OTRGetRectDimensionFromLeftEdge(*rectLeft);
    } else if (CVarGetInteger(hudEditorElements[hudEditorActiveElement].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) ==
               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT) {
        *rectLeft = OTRGetRectDimensionFromRightEdge(*rectLeft);
    }
}

extern "C" void HudEditor_ModifyRectPosValues(s16* rectLeft, s16* rectTop) {
    f32 newLeft = *rectLeft;
    f32 newTop = *rectTop;

    HudEditor_ModifyRectPosValuesFloat(&newLeft, &newTop);

    *rectLeft = (s16)newLeft;
    *rectTop = (s16)newTop;
}

extern "C" void HudEditor_ModifyRectSizeValues(s16* rectWidth, s16* rectHeight) {
    *rectWidth *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *rectHeight *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
}

extern "C" void HudEditor_ModifyTextureStepValues(s16* dsdx, s16* dtdy) {
    *dsdx /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *dtdy /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
}

// Modify matrix values based on the identity matrix (0,0) centered on the screen
extern "C" void HudEditor_ModifyMatrixValues(f32* transX, f32* transY) {
    *transX = ((f32)SCREEN_WIDTH / 2) + *transX;
    *transY = ((f32)SCREEN_HEIGHT / 2) - *transY;

    HudEditor_ModifyRectPosValuesFloat(transX, transY);

    *transX = *transX - ((f32)SCREEN_WIDTH / 2);
    *transY = ((f32)SCREEN_HEIGHT / 2) - *transY;
}

extern "C" void HudEditor_ModifyKaleidoEquipAnimValues(s16* ulx, s16* uly, s16* shrinkRate) {
    // Kaleido values are a multiple of 10 on the identity matrix
    // Normalize them before passing to the modify matrix
    f32 transX = *ulx / 10;
    f32 transY = *uly / 10;

    HudEditor_ModifyMatrixValues(&transX, &transY);

    *ulx = transX * 10;
    *uly = transY * 10;

    float scale = CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    // 320 is the vanilla start size, and 280 is the vanilla end size (or 160 for dpad)
    // So we apply the scale to 280 and subtract to get the shrink rate
    int16_t endAnimSize = hudEditorActiveElement == HUD_EDITOR_ELEMENT_D_PAD ? 160 : 280;
    *shrinkRate = 320 - (s16)(endAnimSize * scale);
}

extern "C" void HudEditor_ModifyDrawValuesFromBase(s16 baseX, s16 baseY, s16* rectLeft, s16* rectTop, s16* rectWidth,
                                                   s16* rectHeight, s16* dsdx, s16* dtdy) {
    HudEditor_ModifyRectPosValuesFromBase(baseX, baseY, rectLeft, rectTop);

    *rectWidth *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *rectHeight *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *dsdx /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *dtdy /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
}

extern "C" void HudEditor_ModifyDrawValues(s16* rectLeft, s16* rectTop, s16* rectWidth, s16* rectHeight, s16* dsdx,
                                           s16* dtdy) {
    HudEditor_ModifyRectPosValues(rectLeft, rectTop);

    *rectWidth *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *rectHeight *= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *dsdx /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
    *dtdy /= CVarGetFloat(hudEditorElements[hudEditorActiveElement].scaleCvar, 1.0f);
}

const char* modeNames[] = {
    "Vanilla (4:3)", "Hidden", "Movable (Align Center)", "Movable (Align Left)", "Movable (Align Right)",
};

const char* presetNames[] = {
    "Vanilla (4:3)",
    "Hidden",
    "Widescreen",
};

namespace HudEditor {
enum Presets {
    VANILLA,
    HIDDEN,
    WIDESCREEN,
};
};

void HudEditorWindow::DrawElement() {
    static HudEditor::Presets preset = HudEditor::Presets::VANILLA;
    if (UIWidgets::Combobox("Preset", &preset, presetNames)) {
        for (int i = HUD_EDITOR_ELEMENT_B; i < HUD_EDITOR_ELEMENT_MAX; i++) {
            CVarClear(hudEditorElements[i].xCvar);
            CVarClear(hudEditorElements[i].yCvar);
            CVarClear(hudEditorElements[i].scaleCvar);
            CVarClear(hudEditorElements[i].colorCvar);
            CVarClear(hudEditorElements[i].modeCvar);
        }

        switch (preset) {
            case HudEditor::Presets::HIDDEN: {
                for (int i = HUD_EDITOR_ELEMENT_B; i < HUD_EDITOR_ELEMENT_MAX; i++) {
                    CVarSetInteger(hudEditorElements[i].modeCvar, HUD_EDITOR_ELEMENT_MODE_HIDDEN);
                }
                break;
            }
            case HudEditor::Presets::WIDESCREEN: {
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_B].modeCvar, HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_C_LEFT].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_C_DOWN].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_C_RIGHT].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_A].modeCvar, HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_C_UP].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_D_PAD].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_MINIMAP].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_START].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_RIGHT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_CARROT].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_43);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_CLOCK].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_43);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_HEARTS].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_MAGIC_METER].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_TIMERS].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_TIMERS_MOON_CRASH].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_43);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_MINIGAME_COUNTER].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_RUPEE_COUNTER].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_KEY_COUNTER].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                CVarSetInteger(hudEditorElements[HUD_EDITOR_ELEMENT_SKULLTULA_COUNTER].modeCvar,
                               HUD_EDITOR_ELEMENT_MODE_MOVABLE_LEFT);
                break;
            }
        }
        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
    }

    for (int i = HUD_EDITOR_ELEMENT_B; i < HUD_EDITOR_ELEMENT_MAX; i++) {
        ImGui::PushID(hudEditorElements[i].name);
        ImGui::SeparatorText(hudEditorElements[i].name);
        float color[3] = { (float)hudEditorElements[i].defaultR / 255, (float)hudEditorElements[i].defaultG / 255,
                           (float)hudEditorElements[i].defaultB / 255 };
        // BENTODO: This color picker currently doesn't do anything other than serve as a visual indicator. Eventually
        // it will be used to set the color of the element.
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::PopItemFlag();
        ImGui::SameLine();
        if (UIWidgets::CVarCombobox("Mode", hudEditorElements[i].modeCvar, modeNames,
                                    { .labelPosition = UIWidgets::LabelPosition::None })) {
            CVarClear(hudEditorElements[i].xCvar);
            CVarClear(hudEditorElements[i].yCvar);
            CVarClear(hudEditorElements[i].scaleCvar);
        }
        if (CVarGetInteger(hudEditorElements[i].modeCvar, HUD_EDITOR_ELEMENT_MODE_VANILLA) >=
            HUD_EDITOR_ELEMENT_MODE_MOVABLE_43) {
            if (ImGui::BeginTable("##table", 3,
                                  ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody |
                                      ImGuiTableFlags_SizingStretchSame)) {
                ImGui::TableNextColumn();
                UIWidgets::CVarSliderInt("X", hudEditorElements[i].xCvar, -10, 330, hudEditorElements[i].defaultX,
                                         {
                                             .showButtons = false,
                                             .format = "X: %d",
                                             .labelPosition = UIWidgets::LabelPosition::None,
                                         });
                ImGui::TableNextColumn();
                UIWidgets::CVarSliderInt("Y", hudEditorElements[i].yCvar, -10, 250, hudEditorElements[i].defaultY,
                                         {
                                             .showButtons = false,
                                             .format = "Y: %d",
                                             .labelPosition = UIWidgets::LabelPosition::None,
                                         });
                ImGui::TableNextColumn();
                UIWidgets::CVarSliderFloat("Scale", hudEditorElements[i].scaleCvar, 0.25f, 4.0f, 1.0f,
                                           {
                                               .showButtons = false,
                                               .format = "Scale: %.2f",
                                               .labelPosition = UIWidgets::LabelPosition::None,
                                           });
                ImGui::EndTable();
            }
        }
        ImGui::PopID();
    }
}
