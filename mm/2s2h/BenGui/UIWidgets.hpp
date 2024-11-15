#ifndef UIWidgets_hpp
#define UIWidgets_hpp

#include <string>
#include <vector>
#include <span>
#include <stdint.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <libultraship/libultraship.h>
#include <unordered_map>
#include "2s2h/ShipUtils.h"

namespace UIWidgets {

    using SectionFunc = void(*)();

    struct TextFilters {
        static int FilterNumbers(ImGuiInputTextCallbackData* data) {
            if (data->EventChar < 256 && strchr("1234567890", (char)data->EventChar)) {
                return 0;
            }
            return 1;
        }

        static int FilterAlphaNum(ImGuiInputTextCallbackData* data) {
            const char* alphanum = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYZ0123456789";
            if (data->EventChar < 256 && strchr(alphanum, (char)data->EventChar)) {
                return 0;
            }
            return 1;
        }
    };

    std::string WrappedText(const char* text, unsigned int charactersPerLine = 60);
    std::string WrappedText(const std::string& text, unsigned int charactersPerLine = 60);
    void Tooltip(const char* text);

    namespace Colors {
        const ImVec4 White = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        const ImVec4 Gray = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
        const ImVec4 DarkGray = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
        const ImVec4 Indigo = ImVec4(0.24f, 0.31f, 0.71f, 1.0f);
        const ImVec4 Red = ImVec4(0.5f, 0.0f, 0.0f, 1.0f);
        const ImVec4 DarkRed = ImVec4(0.3f, 0.0f, 0.0f, 1.0f);
        const ImVec4 LightGreen = ImVec4(0.0f, 0.7f, 0.0f, 1.0f);
        const ImVec4 Green = ImVec4(0.0f, 0.5f, 0.0f, 1.0f);
        const ImVec4 DarkGreen = ImVec4(0.0f, 0.3f, 0.0f, 1.0f);
        const ImVec4 Yellow = ImVec4(1.0f, 0.627f, 0.0f, 1.0f);
    };

    namespace Sizes {
        const ImVec2 Inline = ImVec2(0.0f, 0.0f);
        const ImVec2 Fill = ImVec2(-1.0f, 0.0f);
    }

    enum LabelPosition {
        Near,
        Far,
        Above,
        None,
        Within,
    };

    enum ComponentAlignment {
        Left,
        Right,
    };

    void PushStyleMenu(const ImVec4& color = Colors::Indigo);
    void PopStyleMenu();
    bool BeginMenu(const char* label, const ImVec4& color = Colors::Indigo);

    void PushStyleMenuItem(const ImVec4& color = Colors::Indigo);
    void PopStyleMenuItem();
    bool MenuItem(const char* label, const char* shortcut = NULL, const ImVec4& color = Colors::Indigo);

    struct ButtonOptions {
        const ImVec4 color = Colors::Gray;
        const ImVec2 size = Sizes::Fill;
        const char* tooltip = "";
        bool disabled = false;
        const char* disabledTooltip = "";
    };

    void PushStyleButton(const ImVec4& color = Colors::Gray);
    void PopStyleButton();
    bool Button(const char* label, const ButtonOptions& options = {});
    bool WindowButton(const char* label, const char* cvarName, std::shared_ptr<Ship::GuiWindow> windowPtr, const ButtonOptions& options = {});

    struct CheckboxOptions {
        const ImVec4 color = Colors::Indigo;
        const char* tooltip = "";
        bool disabled = false;
        const char* disabledTooltip = "";
        bool defaultValue = false; // Only applicable to CVarCheckbox
        ComponentAlignment alignment = ComponentAlignment::Left;
        LabelPosition labelPosition = LabelPosition::Near;
    };

    void PushStyleCheckbox(const ImVec4& color = Colors::Indigo);
    void PopStyleCheckbox();
    void RenderText(ImVec2 pos, const char* text, const char* text_end, bool hide_text_after_hash);
    bool Checkbox(const char* label, bool* v, const CheckboxOptions& options = {});
    bool CVarCheckbox(const char* label, const char* cvarName, const CheckboxOptions& options = {});

    struct ComboboxOptions {
        const ImVec4 color = Colors::Indigo;
        const char* tooltip = "";
        bool disabled = false;
        const char* disabledTooltip = "";
        uint32_t defaultIndex = 0; // Only applicable to CVarCombobox
        ComponentAlignment alignment = ComponentAlignment::Left;
        LabelPosition labelPosition = LabelPosition::Above;
        ImGuiComboFlags flags = 0;
    };

    void PushStyleCombobox(const ImVec4& color = Colors::Indigo);
    void PopStyleCombobox();

    template <typename T>
    bool Combobox(const char* label, T* value, const std::unordered_map<T, const char*>& comboMap, const ComboboxOptions& options = {}) {
        bool dirty = false;
        float startX = ImGui::GetCursorPosX();
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        ImGui::PushID(label);
        ImGui::BeginGroup();
        ImGui::BeginDisabled(options.disabled);
        PushStyleCombobox(options.color);
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text("%s", label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x - ImGui::GetStyle().ItemSpacing.x * 2);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                ImGui::SetNextItemWidth(ImGui::CalcTextSize(comboMap.at(*value)).x + ImGui::GetStyle().FramePadding.x * 4 + ImGui::GetStyle().ItemSpacing.x);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text("%s", label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine(ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x * 2);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                float width = ImGui::CalcTextSize(comboMap.at(*value)).x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - width);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            }
        }
        if (ImGui::BeginCombo(invisibleLabel, comboMap.at(*value), options.flags)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            for (const auto& pair : comboMap) {
                if (strlen(pair.second) > 1) {
                    if (ImGui::Selectable(pair.second, pair.first == *value)) {
                        *value = pair.first;
                        dirty = true;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndCombo();
        }
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine();
                ImGui::Text("%s", label);
            } else if (options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text("%s", label);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Near || options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(startX);
                ImGui::Text("%s", label);
            }
        }
        PopStyleCombobox();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.disabledTooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip).c_str());
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.tooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip).c_str());
        }
        ImGui::PopID();
        return dirty;
    }

    template <typename T = size_t>
    bool Combobox(const char* label, T* value, const std::vector<const char*>& comboVector, const ComboboxOptions& options = {}) {
        bool dirty = false;
        float startX = ImGui::GetCursorPosX();
        size_t currentValueIndex = static_cast<size_t>(*value);
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        ImGui::PushID(label);
        ImGui::BeginGroup();
        ImGui::BeginDisabled(options.disabled);
        PushStyleCombobox(options.color);
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text(label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x - ImGui::GetStyle().ItemSpacing.x * 2);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                ImGui::SetNextItemWidth(ImGui::CalcTextSize(comboVector.at(currentValueIndex)).x + ImGui::GetStyle().FramePadding.x * 4 + ImGui::GetStyle().ItemSpacing.x);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine(ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x * 2);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                float width = ImGui::CalcTextSize(comboVector.at(currentValueIndex)).x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - width);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            }
        }
        if (ImGui::BeginCombo(invisibleLabel, comboVector.at(currentValueIndex), options.flags)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            for (size_t i = 0; i < comboVector.size(); ++i) {
                auto newValue = static_cast<T>(i);
                if (strlen(comboVector.at(i)) > 1) {
                    if (ImGui::Selectable(comboVector.at(i), newValue == *value)) {
                        *value = newValue;
                        dirty = true;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndCombo();
        }
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine();
                ImGui::Text(label);
            } else if (options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text(label);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Near || options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(startX);
                ImGui::Text(label);
            }
        }
        PopStyleCombobox();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.disabledTooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip).c_str());
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.tooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip).c_str());
        }
        ImGui::PopID();
        return dirty;
    }

    template <typename T = size_t, size_t N>
    bool Combobox(const char* label, T* value, const char* (&comboArray)[N], const ComboboxOptions& options = {}) {
        bool dirty = false;
        float startX = ImGui::GetCursorPosX();
        size_t currentValueIndex = static_cast<size_t>(*value);
        if (currentValueIndex >= N) {
            currentValueIndex = 0;
        }
        std::string invisibleLabelStr = "##" + std::string(label);
        const char* invisibleLabel = invisibleLabelStr.c_str();
        ImGui::PushID(label);
        ImGui::BeginGroup();
        ImGui::BeginDisabled(options.disabled);
        PushStyleCombobox(options.color);
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::Text("%s", label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x - ImGui::GetStyle().ItemSpacing.x * 2);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                ImGui::SetNextItemWidth(ImGui::CalcTextSize(comboArray[currentValueIndex]).x + ImGui::GetStyle().FramePadding.x * 4 + ImGui::GetStyle().ItemSpacing.x);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Above) {
                ImGui::NewLine();
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text("%s", label);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine(ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemSpacing.x * 2);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            } else if (options.labelPosition == LabelPosition::Far || options.labelPosition == LabelPosition::None) {
                float width = ImGui::CalcTextSize(comboArray[currentValueIndex]).x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - width);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            }
        }
        if (ImGui::BeginCombo(invisibleLabel, comboArray[currentValueIndex], options.flags)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
            for (size_t i = 0; i < N; ++i) {
                auto newValue = static_cast<T>(i);
                if (strlen(comboArray[i]) > 1) {
                    if (ImGui::Selectable(comboArray[i], newValue == *value)) {
                        *value = newValue;
                        dirty = true;
                    }
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndCombo();
        }
        if (options.alignment == ComponentAlignment::Left) {
            if (options.labelPosition == LabelPosition::Near) {
                ImGui::SameLine();
                ImGui::Text("%s", label);
            } else if (options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(label).x);
                ImGui::Text("%s", label);
            }
        } else if (options.alignment == ComponentAlignment::Right) {
            if (options.labelPosition == LabelPosition::Near || options.labelPosition == LabelPosition::Far) {
                ImGui::SameLine(startX);
                ImGui::Text("%s", label);
            }
        }
        PopStyleCombobox();
        ImGui::EndDisabled();
        ImGui::EndGroup();
        if (options.disabled && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.disabledTooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.disabledTooltip).c_str());
        } else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !Ship_IsCStringEmpty(options.tooltip)) {
            ImGui::SetTooltip("%s", WrappedText(options.tooltip).c_str());
        }
        ImGui::PopID();
        return dirty;
    }

    template <typename T = int32_t>
    bool CVarCombobox(const char* label, const char* cvarName, const std::unordered_map<T, const char*>& comboMap, const ComboboxOptions& options = {}) {
        bool dirty = false;
        int32_t value = CVarGetInteger(cvarName, options.defaultIndex);
        if (Combobox<T>(label, &value, comboMap, options)) {
            CVarSetInteger(cvarName, value);
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }

    template <typename T = int32_t>
    bool CVarCombobox(const char* label, const char* cvarName, const std::vector<const char*>& comboVector, const ComboboxOptions& options = {}) {
        bool dirty = false;
        int32_t value = CVarGetInteger(cvarName, options.defaultIndex);
        if (Combobox<T>(label, &value, comboVector, options)) {
            CVarSetInteger(cvarName, value);
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }

    template <typename T = int32_t, size_t N>
    bool CVarCombobox(const char* label, const char* cvarName, const char* (&comboArray)[N], const ComboboxOptions& options = {}) {
        bool dirty = false;
        int32_t value = CVarGetInteger(cvarName, options.defaultIndex);
        if (Combobox<T>(label, &value, comboArray, options)) {
            CVarSetInteger(cvarName, value);
            Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
            dirty = true;
        }
        return dirty;
    }

    struct IntSliderOptions {
        const ImVec4 color = Colors::Gray;
        const char* tooltip = "";
        bool disabled = false;
        const char* disabledTooltip = "";
        bool showButtons = true;
        ImGuiSliderFlags flags = 0;
        const char* format = "%d";
        const uint32_t step = 1;
        ComponentAlignment alignment = ComponentAlignment::Left;
        LabelPosition labelPosition = LabelPosition::Above;
    };

    struct FloatSliderOptions {
        const ImVec4 color = Colors::Gray;
        const char* tooltip = "";
        bool disabled = false;
        const char* disabledTooltip = "";
        bool showButtons = true;
        ImGuiSliderFlags flags = 0;
        const char* format = "%f";
        const float step = 0.01f;
        bool isPercentage = false; // Multiplies visual value by 100
        ComponentAlignment alignment = ComponentAlignment::Left;
        LabelPosition labelPosition = LabelPosition::Above;
    };

    void PushStyleSlider(const ImVec4& color = Colors::Indigo);
    void PopStyleSlider();
    bool SliderInt(const char* label, int32_t* value, int32_t min, int32_t max, const IntSliderOptions& options = {});
    bool CVarSliderInt(const char* label, const char* cvarName, int32_t min, int32_t max, const int32_t defaultValue, const IntSliderOptions& options = {});
    bool SliderFloat(const char* label, float* value, float min, float max, const FloatSliderOptions& options = {});
    bool CVarSliderFloat(const char* label, const char* cvarName, float min, float max, const float defaultValue, const FloatSliderOptions& options = {});
    bool CVarColorPicker(const char* label, const char* cvarName, Color_RGBA8 defaultColor);
    void DrawFlagArray32(const std::string& name, uint32_t& flags);
    void DrawFlagArray16(const std::string& name, uint16_t& flags);
    void DrawFlagArray8(const std::string& name, uint8_t& flags);
}

#endif /* UIWidgets_hpp */
