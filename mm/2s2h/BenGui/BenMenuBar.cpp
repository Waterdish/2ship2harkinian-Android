#include "BenMenuBar.h"
#include "BenPort.h"
#include <imgui.h>
#include "public/bridge/consolevariablebridge.h"
#include <libultraship/libultraship.h>
#include "graphic/Fast3D/gfx_rendering_api.h"
#include "UIWidgets.hpp"
#include <unordered_map>
#include <string>
#include "2s2h/Enhancements/Enhancements.h"
#include "2s2h/Enhancements/Graphics/MotionBlur.h"
#include "2s2h/Enhancements/Graphics/PlayAsKafei.h"
#include "2s2h/DeveloperTools/DeveloperTools.h"
#include "2s2h/DeveloperTools/WarpPoint.h"
#include "HudEditor.h"

extern "C" {
#include "z64.h"
#include "functions.h"
extern PlayState* gPlayState;
}

static const std::unordered_map<int32_t, const char*> textureFilteringMap = {
    { FILTER_THREE_POINT, "Three-Point" },
    { FILTER_LINEAR, "Linear" },
    { FILTER_NONE, "None" },
};

static const std::unordered_map<int32_t, const char*> debugSaveOptions = {
    { DEBUG_SAVE_INFO_COMPLETE, "100\% save" },
    { DEBUG_SAVE_INFO_VANILLA_DEBUG, "Vanilla debug save" },
    { DEBUG_SAVE_INFO_NONE, "Empty save" },
};

static const std::unordered_map<Ship::AudioBackend, const char*> audioBackendsMap = {
    { Ship::AudioBackend::WASAPI, "Windows Audio Session API" },
    { Ship::AudioBackend::SDL, "SDL" },
};

static std::unordered_map<Ship::WindowBackend, const char*> windowBackendsMap = {
    { Ship::WindowBackend::DX11, "DirectX" },
    { Ship::WindowBackend::SDL_OPENGL, "OpenGL" },
    { Ship::WindowBackend::SDL_METAL, "Metal" },
    { Ship::WindowBackend::GX2, "GX2" }
};

static const std::unordered_map<int32_t, const char*> clockTypeOptions = {
    { CLOCK_TYPE_ORIGINAL, "Original" },
    { CLOCK_TYPE_TEXT_BASED, "Text only" },
};

static const std::unordered_map<int32_t, const char*> alwaysWinDoggyraceOptions = {
    { ALWAYS_WIN_DOGGY_RACE_OFF, "Off" },
    { ALWAYS_WIN_DOGGY_RACE_MASKOFTRUTH, "When owning Mask of Truth" },
    { ALWAYS_WIN_DOGGY_RACE_ALWAYS, "Always" },
};

namespace BenGui {

void DrawMenuBarIcon() {
    static bool gameIconLoaded = false;
    if (!gameIconLoaded) {
        Ship::Context::GetInstance()->GetWindow()->GetGui()->LoadTextureFromRawImage("Game_Icon",
                                                                                     "textures/icons/g2ShipIcon.png");
        gameIconLoaded = true;
    }

    if (Ship::Context::GetInstance()->GetWindow()->GetGui()->HasTextureByName("Game_Icon")) {
#ifdef __SWITCH__
        ImVec2 iconSize = ImVec2(20.0f, 20.0f);
        float posScale = 1.0f;
#elif defined(__WIIU__)
        ImVec2 iconSize = ImVec2(16.0f * 2, 16.0f * 2);
        float posScale = 2.0f;
#elif defined(__ANDROID__)
        ImVec2 iconSize = ImVec2(16.0f * 2, 16.0f * 2);
        float posScale = 2.0f;
#else
        ImVec2 iconSize = ImVec2(16.0f, 16.0f);
        float posScale = 1.0f;
#endif
        ImGui::SetCursorPos(ImVec2(5, 5) * posScale);
        ImGui::Image(Ship::Context::GetInstance()->GetWindow()->GetGui()->GetTextureByName("Game_Icon"), iconSize);
        ImGui::SameLine();
        ImGui::SetCursorPos(ImVec2(25, 0) * posScale);
    }
}

void DrawBenMenu() {
    if (UIWidgets::BeginMenu("2Ship")) {
        if (UIWidgets::MenuItem("Hide Menu Bar",
#if !defined(__SWITCH__) && !defined(__WIIU__) && !defined(__ANDROID__)
                                "F1"
#else
                                "[-]"
#endif
                                )) {
            Ship::Context::GetInstance()->GetWindow()->GetGui()->GetMenuBar()->ToggleVisibility();
        }
#if !defined(__SWITCH__) && !defined(__WIIU__) && !defined(__ANDROID__)
        if (UIWidgets::MenuItem("Toggle Fullscreen", "F11")) {
            Ship::Context::GetInstance()->GetWindow()->ToggleFullscreen();
        }
#endif
        if (UIWidgets::MenuItem("Reset",
#ifdef __APPLE__
                                "Command-R"
#elif !defined(__SWITCH__) && !defined(__WIIU__) && !defined(__ANDROID__)
                                "Ctrl+R"
#else
                                ""
#endif
                                )) {
            std::reinterpret_pointer_cast<Ship::ConsoleWindow>(
                Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGuiWindow("Console"))
                ->Dispatch("reset");
        }
#if !defined(__SWITCH__) && !defined(__WIIU__) && !defined(__ANDROID__)
        if (UIWidgets::MenuItem("Open App Files Folder")) {
            std::string filesPath = Ship::Context::GetInstance()->GetAppDirectoryPath();
            SDL_OpenURL(std::string("file:///" + std::filesystem::absolute(filesPath).string()).c_str());
        }

        if (UIWidgets::MenuItem("Quit")) {
            Ship::Context::GetInstance()->GetWindow()->Close();
        }
#endif
        ImGui::EndMenu();
    }
}

extern std::shared_ptr<Ship::GuiWindow> mInputEditorWindow;

void DrawSettingsMenu() {
    if (UIWidgets::BeginMenu("Settings")) {
        if (UIWidgets::BeginMenu("Audio")) {
            UIWidgets::CVarSliderFloat("Master Volume: %.0f %%", "gSettings.Audio.MasterVolume", 0.0f, 1.0f, 1.0f,
                                       { .showButtons = false, .format = "", .isPercentage = true });

            if (UIWidgets::CVarSliderFloat("Main Music Volume: %.0f %%", "gSettings.Audio.MainMusicVolume", 0.0f, 1.0f,
                                           1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_MAIN, CVarGetFloat("gSettings.Audio.MainMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sub Music Volume: %.0f %%", "gSettings.Audio.SubMusicVolume", 0.0f, 1.0f,
                                           1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_BGM_SUB, CVarGetFloat("gSettings.Audio.SubMusicVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Sound Effects Volume: %.0f %%", "gSettings.Audio.SoundEffectsVolume", 0.0f,
                                           1.0f, 1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_SFX, CVarGetFloat("gSettings.Audio.SoundEffectsVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Fanfare Volume: %.0f %%", "gSettings.Audio.FanfareVolume", 0.0f, 1.0f, 1.0f,
                                           { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_FANFARE, CVarGetFloat("gSettings.Audio.FanfareVolume", 1.0f));
            }
            if (UIWidgets::CVarSliderFloat("Ambience Volume: %.0f %%", "gSettings.Audio.AmbienceVolume", 0.0f, 1.0f,
                                           1.0f, { .showButtons = false, .format = "", .isPercentage = true })) {
                AudioSeq_SetPortVolumeScale(SEQ_PLAYER_AMBIENCE, CVarGetFloat("gSettings.Audio.AmbienceVolume", 1.0f));
            }

            auto currentAudioBackend = Ship::Context::GetInstance()->GetAudio()->GetAudioBackend();
            if (UIWidgets::Combobox(
                    "Audio API", &currentAudioBackend, audioBackendsMap,
                    { .tooltip = "Sets the audio API used by the game. Requires a relaunch to take effect.",
                      .disabled = Ship::Context::GetInstance()->GetAudio()->GetAvailableAudioBackends()->size() <= 1,
                      .disabledTooltip = "Only one audio API is available on this platform." })) {
                Ship::Context::GetInstance()->GetAudio()->SetAudioBackend(currentAudioBackend);
            }

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Graphics")) {

#ifndef __APPLE__
            if (UIWidgets::CVarSliderFloat("Internal Resolution: %f %%", CVAR_INTERNAL_RESOLUTION, 0.5f, 2.0f, 1.0f)) {
                Ship::Context::GetInstance()->GetWindow()->SetResolutionMultiplier(
                    CVarGetFloat(CVAR_INTERNAL_RESOLUTION, 1));
            };
            UIWidgets::Tooltip(
                "Multiplies your output resolution by the value inputted, as a more intensive but effective "
                "form of anti-aliasing");
#endif
#ifndef __WIIU__
            if (UIWidgets::CVarSliderInt((CVarGetInteger(CVAR_MSAA_VALUE, 1) == 1) ? "Anti-aliasing (MSAA): Off"
                                                                                   : "Anti-aliasing (MSAA): %d",
                                         CVAR_MSAA_VALUE, 1, 8, 1)) {
                Ship::Context::GetInstance()->GetWindow()->SetMsaaLevel(CVarGetInteger(CVAR_MSAA_VALUE, 1));
            };
            UIWidgets::Tooltip(
                "Activates MSAA (multi-sample anti-aliasing) from 2x up to 8x, to smooth the edges of rendered "
                "geometry.\n"
                "Higher sample count will result in smoother edges on models, but may reduce performance.");
#endif

            { // FPS Slider
                constexpr unsigned int minFps = 20;
                static unsigned int maxFps;
                if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
                    maxFps = 360;
                } else {
                    maxFps = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                }
                unsigned int currentFps =
                    std::max(std::min(OTRGlobals::Instance->GetInterpolationFPS(), maxFps), minFps);
                bool matchingRefreshRate =
                    CVarGetInteger("gMatchRefreshRate", 0) &&
                    Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() != Ship::WindowBackend::DX11;
                UIWidgets::CVarSliderInt((currentFps == 20) ? "FPS: Original (20)" : "FPS: %d", "gInterpolationFPS",
                                         minFps, maxFps, 20, { .disabled = matchingRefreshRate });
                if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
                    UIWidgets::Tooltip(
                        "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                        "purely visual and does not impact game logic, execution of glitches etc.\n\n A higher target "
                        "FPS than your monitor's refresh rate will waste resources, and might give a worse result.");
                } else {
                    UIWidgets::Tooltip(
                        "Uses Matrix Interpolation to create extra frames, resulting in smoother graphics. This is "
                        "purely visual and does not impact game logic, execution of glitches etc.");
                }
            } // END FPS Slider

            if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
                // UIWidgets::Spacer(0);
                if (ImGui::Button("Match Refresh Rate")) {
                    int hz = Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();
                    if (hz >= 20 && hz <= 360) {
                        CVarSetInteger("gInterpolationFPS", hz);
                        Ship::Context::GetInstance()->GetWindow()->GetGui()->SaveConsoleVariablesOnNextTick();
                    }
                }
            } else {
                UIWidgets::CVarCheckbox("Match Refresh Rate", "gMatchRefreshRate");
            }
            UIWidgets::Tooltip("Matches interpolation value to the current game's window refresh rate");

            if (Ship::Context::GetInstance()->GetWindow()->GetWindowBackend() == Ship::WindowBackend::DX11) {
                UIWidgets::CVarSliderInt(CVarGetInteger("gExtraLatencyThreshold", 80) == 0 ? "Jitter fix: Off"
                                                                                           : "Jitter fix: >= %d FPS",
                                         "gExtraLatencyThreshold", 0, 360, 80);
                UIWidgets::Tooltip("When Interpolation FPS setting is at least this threshold, add one frame of input "
                                   "lag (e.g. 16.6 ms for 60 FPS) in order to avoid jitter. This setting allows the "
                                   "CPU to work on one frame while GPU works on the previous frame.\nThis setting "
                                   "should be used when your computer is too slow to do CPU + GPU work in time.");
            }

            // UIWidgets::PaddedSeparator(true, true, 3.0f, 3.0f);
            //  #endregion */

            Ship::WindowBackend runningWindowBackend = Ship::Context::GetInstance()->GetWindow()->GetWindowBackend();
            Ship::WindowBackend configWindowBackend;
            int32_t configWindowBackendId = Ship::Context::GetInstance()->GetConfig()->GetInt("Window.Backend.Id", -1);
            if (configWindowBackendId != -1 &&
                configWindowBackendId < static_cast<int>(Ship::WindowBackend::BACKEND_COUNT)) {
                configWindowBackend = static_cast<Ship::WindowBackend>(configWindowBackendId);
            } else {
                configWindowBackend = runningWindowBackend;
            }

            auto availableWindowBackends = Ship::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends();
            std::unordered_map<Ship::WindowBackend, const char*> availableWindowBackendsMap;
            for (auto& backend : *availableWindowBackends) {
                availableWindowBackendsMap[backend] = windowBackendsMap[backend];
            }

            if (UIWidgets::Combobox(
                    "Renderer API (Needs reload)", &configWindowBackend, availableWindowBackendsMap,
                    { .tooltip = "Sets the renderer API used by the game. Requires a relaunch to take effect.",
                      .disabled = Ship::Context::GetInstance()->GetWindow()->GetAvailableWindowBackends()->size() <= 1,
                      .disabledTooltip = "Only one renderer API is available on this platform." })) {
                Ship::Context::GetInstance()->GetConfig()->SetInt("Window.Backend.Id",
                                                                  static_cast<int32_t>(configWindowBackend));
                Ship::Context::GetInstance()->GetConfig()->SetString("Window.Backend.Name",
                                                                     windowBackendsMap.at(configWindowBackend));
                Ship::Context::GetInstance()->GetConfig()->Save();
            }

            if (Ship::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
                UIWidgets::CVarCheckbox("Enable Vsync", CVAR_VSYNC_ENABLED);
            }

            if (Ship::Context::GetInstance()->GetWindow()->SupportsWindowedFullscreen()) {
                UIWidgets::CVarCheckbox("Windowed fullscreen", CVAR_SDL_WINDOWED_FULLSCREEN);
            }

            if (Ship::Context::GetInstance()->GetWindow()->GetGui()->SupportsViewports()) {
                UIWidgets::CVarCheckbox(
                    "Allow multi-windows", CVAR_ENABLE_MULTI_VIEWPORTS,
                    { .tooltip = "Allows multiple windows to be opened at once. Requires a reload to take effect." });
            }

            UIWidgets::CVarCombobox("Texture Filter (Needs reload)", CVAR_TEXTURE_FILTER, textureFilteringMap);

            // Currently this only has "Overlays Text Font", it doesn't use our new UIWidgets so it stands out
            // Ship::Context::GetInstance()->GetWindow()->GetGui()->GetGameOverlay()->DrawSettings();

            ImGui::EndMenu();
        }
        // #region 2S2H [Todo] None of this works yet
        /*
        if (UIWidgets::BeginMenu("Controller")) { */
        if (mInputEditorWindow) {
            UIWidgets::WindowButton("Controller Mapping", "gWindows.InputEditor", mInputEditorWindow);
        }
        /*
        #ifndef __SWITCH__
            UIWidgets::CVarCheckbox("Menubar Controller Navigation", CVAR_IMGUI_CONTROLLER_NAV, {
                .tooltip = "Allows controller navigation of the SOH menu bar (Settings, Enhancements,...)\nCAUTION: "
                "This will disable game inputs while the menubar is visible.\n\nD-pad to move between "
                "items, A to select, and X to grab focus on the menu bar"
            });
        #endif
            UIWidgets::CVarCheckbox("Show Inputs", "gInputEnabled", {
                .tooltip = "Shows currently pressed inputs on the bottom right of the screen"
            });
            UIWidgets::CVarSliderFloat("Input Scale: %.1f", "gInputScale", 1.0f, 3.0f, 1.0f, {
                .tooltip = "Sets the on screen size of the displayed inputs from the Show Inputs setting",
                .showButtons = false,
                .format = ""
            });
            UIWidgets::CVarSliderInt("Simulated Input Lag: %d frames", CVAR_SIMULATED_INPUT_LAG, 0, 6, 0, {
                .tooltip = "Buffers your inputs to be executed a specified amount of frames later"
            });

            ImGui::EndMenu();
        }
        ImGui::EndMenu();
        */
        ImGui::EndMenu();
    }
}

extern std::shared_ptr<HudEditorWindow> mHudEditorWindow;

void DrawEnhancementsMenu() {
    if (UIWidgets::BeginMenu("Enhancements")) {

        if (UIWidgets::BeginMenu("Camera")) {
            ImGui::SeparatorText("Fixes");
            UIWidgets::CVarCheckbox(
                "Fix Targetting Camera Snap", "gEnhancements.Camera.FixTargettingCameraSnap",
                { .tooltip =
                      "Fixes the camera snap that occurs when you are moving and press the targetting button." });

            ImGui::SeparatorText("Free Look");
            if (UIWidgets::CVarCheckbox(
                    "Free Look", "gEnhancements.Camera.FreeLook.Enable",
                    { .tooltip = "Enables free look camera control\nNote: You must remap C buttons off of the right "
                                 "stick in the controller config menu, and map the camera stick to the right stick.",
                      .disabled = CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0) != 0 })) {
                RegisterCameraFreeLook();
            }

            if (CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0)) {
                UIWidgets::CVarCheckbox("Invert Camera X Axis", "gEnhancements.Camera.RightStick.InvertXAxis",
                                        { .tooltip = "Inverts the Camera X Axis" });
                UIWidgets::CVarCheckbox("Invert Camera Y Axis", "gEnhancements.Camera.RightStick.InvertYAxis",
                                        { .tooltip = "Inverts the Camera Y Axis", .defaultValue = true });
                UIWidgets::CVarSliderFloat("Third-Person Horizontal Sensitivity: %.0f",
                                           "gEnhancements.Camera.RightStick.CameraSensitivity.X", 0.01f, 5.0f, 1.0f);
                UIWidgets::CVarSliderFloat("Third-Person Vertical Sensitivity: %.0f",
                                           "gEnhancements.Camera.RightStick.CameraSensitivity.Y", 0.01f, 5.0f, 1.0f);

                UIWidgets::CVarSliderInt("Camera Distance: %d", "gEnhancements.Camera.FreeLook.MaxCameraDistance", 100,
                                         900, 185);
                UIWidgets::CVarSliderInt("Camera Transition Speed: %d", "gEnhancements.Camera.FreeLook.TransitionSpeed",
                                         1, 900, 25);
                UIWidgets::CVarSliderFloat("Max Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MaxPitch",
                                           -89.0f, 89.0f, 72.0f);
                UIWidgets::CVarSliderFloat("Min Camera Height Angle: %.0f°", "gEnhancements.Camera.FreeLook.MinPitch",
                                           -89.0f, 89.0f, -49.0f);
                f32 maxY = CVarGetFloat("gEnhancements.Camera.FreeLook.MaxPitch", 72.0f);
                f32 minY = CVarGetFloat("gEnhancements.Camera.FreeLook.MinPitch", -49.0f);
                CVarSetFloat("gEnhancements.Camera.FreeLook.MaxPitch", std::max(maxY, minY));
                CVarSetFloat("gEnhancements.Camera.FreeLook.MinPitch", std::min(maxY, minY));
            }

            ImGui::SeparatorText("'Debug' Camera");
            if (UIWidgets::CVarCheckbox(
                    "Debug Camera", "gEnhancements.Camera.DebugCam.Enable",
                    { .tooltip = "Enables free camera control.",
                      .disabled = CVarGetInteger("gEnhancements.Camera.FreeLook.Enable", 0) != 0 })) {
                RegisterDebugCam();
            }

            if (CVarGetInteger("gEnhancements.Camera.DebugCam.Enable", 0)) {
                UIWidgets::CVarCheckbox("Invert Camera X Axis", "gEnhancements.Camera.RightStick.InvertXAxis",
                                        { .tooltip = "Inverts the Camera X Axis" });
                UIWidgets::CVarCheckbox("Invert Camera Y Axis", "gEnhancements.Camera.RightStick.InvertYAxis",
                                        { .tooltip = "Inverts the Camera Y Axis", .defaultValue = true });
                UIWidgets::CVarSliderFloat("Third-Person Horizontal Sensitivity: %.0f",
                                           "gEnhancements.Camera.RightStick.CameraSensitivity.X", 0.01f, 5.0f, 1.0f);
                UIWidgets::CVarSliderFloat("Third-Person Vertical Sensitivity: %.0f",
                                           "gEnhancements.Camera.RightStick.CameraSensitivity.Y", 0.01f, 5.0f, 1.0f);

                UIWidgets::CVarCheckbox(
                    "Enable Roll (Six Degrees Of Freedom)", "gEnhancements.Camera.DebugCam.6DOF",
                    { .tooltip = "This allows for all six degrees of movement with the camera, NOTE: Yaw will work "
                                 "differently in "
                                 "this system, instead rotating around the focal point, rather than a polar axis." });
                UIWidgets::CVarSliderFloat("Camera Speed: %.0f", "gEnhancements.Camera.DebugCam.CameraSpeed", 0.1f,
                                           3.0f, 0.5f);
            }

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Cutscenes")) {
            UIWidgets::CVarCheckbox("Hide Title Cards", "gEnhancements.Cutscenes.HideTitleCards");
            UIWidgets::CVarCheckbox("Skip Entrance Cutscenes", "gEnhancements.Cutscenes.SkipEntranceCutscenes");
            UIWidgets::CVarCheckbox(
                "Skip to File Select", "gEnhancements.Cutscenes.SkipToFileSelect",
                { .tooltip = "Skip the opening title sequence and go straight to the file select menu after boot" });
            UIWidgets::CVarCheckbox(
                "Skip Intro Sequence", "gEnhancements.Cutscenes.SkipIntroSequence",
                {
                    .tooltip = "When starting a game you will be taken straight to South Clock Town as Deku Link.",
                });
            UIWidgets::CVarCheckbox(
                "Skip Story Cutscenes", "gEnhancements.Cutscenes.SkipStoryCutscenes",
                {
                    .tooltip =
                        "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time",
                });
            UIWidgets::CVarCheckbox(
                "Skip Misc Interactions", "gEnhancements.Cutscenes.SkipMiscInteractions",
                {
                    .tooltip =
                        "Disclaimer: This doesn't do much yet, we will be progressively adding more skips over time",
                });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Saving / Time Cycle")) {

            ImGui::SeparatorText("Saving");
            UIWidgets::CVarCheckbox("Persistent Owl Saves", "gEnhancements.Saving.PersistentOwlSaves",
                                    { .tooltip = "Continuing a save will not remove the owl save. Playing Song of "
                                                 "Time, allowing the moon to crash or finishing the "
                                                 "game will remove the owl save and become the new last save." });
            UIWidgets::CVarCheckbox(
                "Pause Menu Save", "gEnhancements.Saving.PauseSave",
                { .tooltip = "Re-introduce the pause menu save system. Pressing B in the pause menu will give you the "
                             "option to create a persistent Owl Save from your current location.\n\nWhen loading back "
                             "into the game, you will be placed either at the entrance of the dungeon you saved in, or "
                             "in South Clock Town." });
            if (UIWidgets::CVarCheckbox(
                    "Autosave", "gEnhancements.Saving.Autosave",
                    { .tooltip = "Automatically create a persistent Owl Save on the chosen interval.\n\nWhen loading "
                                 "back into the game, you will be placed either at the entrance of the dungeon you "
                                 "saved in, or in South Clock Town." })) {
                RegisterAutosave();
            }
            UIWidgets::CVarSliderInt("Autosave Interval (minutes): %d", "gEnhancements.Saving.AutosaveInterval", 1, 60,
                                     5, { .disabled = !CVarGetInteger("gEnhancements.Saving.Autosave", 0) });

            ImGui::SeparatorText("Time Cycle");
            UIWidgets::CVarCheckbox("Do not reset Bottle content", "gEnhancements.Cycle.DoNotResetBottleContent",
                                    { .tooltip = "Playing the Song Of Time will not reset the bottles' content." });
            UIWidgets::CVarCheckbox("Do not reset Consumables", "gEnhancements.Cycle.DoNotResetConsumables",
                                    { .tooltip = "Playing the Song Of Time will not reset the consumables." });
            UIWidgets::CVarCheckbox(
                "Do not reset Razor Sword", "gEnhancements.Cycle.DoNotResetRazorSword",
                { .tooltip = "Playing the Song Of Time will not reset the Sword back to Kokiri Sword." });
            UIWidgets::CVarCheckbox("Do not reset Rupees", "gEnhancements.Cycle.DoNotResetRupees",
                                    { .tooltip = "Playing the Song Of Time will not reset the your rupees." });

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 0, 255));
            ImGui::SeparatorText("Unstable");
            ImGui::PopStyleColor();
            UIWidgets::CVarCheckbox(
                "Disable Save Delay", "gEnhancements.Saving.DisableSaveDelay",
                { .tooltip = "Removes the arbitrary 2 second timer for saving from the original game. This is known to "
                             "cause issues when attempting the 0th Day Glitch" });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Dialogues")) {
            UIWidgets::CVarCheckbox(
                "Fast Text", "gEnhancements.Dialogue.FastText",
                { .tooltip = "Speeds up text rendering, and enables holding of B progress to next message" });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Dpad")) {
            UIWidgets::CVarCheckbox("Dpad Equips", "gEnhancements.Dpad.DpadEquips",
                                    { .tooltip = "Allows you to equip items to your d-pad" });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Fixes")) {
            UIWidgets::CVarCheckbox("Fix Ammo Count Color", "gFixes.FixAmmoCountEnvColor",
                                    { .tooltip = "Fixes a missing gDPSetEnvColor, which causes the ammo count to be "
                                                 "the wrong color prior to obtaining magic or other conditions." });

            UIWidgets::CVarCheckbox("Fix Hess and Weirdshot Crash", "gEnhancements.Fixes.HessCrash",
                                    { .tooltip = "Fixes a crash that can occur when performing a HESS or Weirdshot.",
                                      .defaultValue = true });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Graphics")) {
            ImGui::SeparatorText("Clock");
            UIWidgets::CVarCombobox("Clock Type", "gEnhancements.Graphics.ClockType", clockTypeOptions);
            UIWidgets::CVarCheckbox("24 Hours Clock", "gEnhancements.Graphics.24HoursClock");
            MotionBlur_RenderMenuOptions();
            ImGui::SeparatorText("Other");
            UIWidgets::CVarCheckbox("Authentic logo", "gEnhancements.Graphics.AuthenticLogo",
                                    { .tooltip = "Hide the game version and build details and display the authentic "
                                                 "model and texture on the boot logo start screen" });
            UIWidgets::CVarCheckbox("Bow Reticle", "gEnhancements.Graphics.BowReticle",
                                    { .tooltip = "Gives the bow a reticle when you draw an arrow" });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Masks")) {
            UIWidgets::CVarCheckbox("Fast Transformation", "gEnhancements.Masks.FastTransformation");
            UIWidgets::CVarCheckbox("Fierce Deity's Mask Anywhere", "gEnhancements.Masks.FierceDeitysAnywhere",
                                    { .tooltip = "Allow using Fierce Deity's mask outside of boss rooms." });
            UIWidgets::CVarCheckbox("No Blast Mask Cooldown", "gEnhancements.Masks.NoBlastMaskCooldown", {});

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Minigames")) {
            UIWidgets::CVarCombobox("Always Win Doggy Race", "gEnhancements.Minigames.AlwaysWinDoggyRace",
                                    alwaysWinDoggyraceOptions);

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Modes")) {
            UIWidgets::CVarCheckbox("Play As Kafei", "gModes.PlayAsKafei",
                                    { .tooltip = "Requires scene reload to take effect." });
            ImGui::EndMenu();
        }
        if (UIWidgets::BeginMenu("Player Movement")) {
            UIWidgets::CVarSliderInt("Climb speed", "gEnhancements.PlayerMovement.ClimbSpeed", 1, 5, 1,
                                     { .tooltip = "Increases the speed at which Link climbs vines and ladders." });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Restorations")) {
            UIWidgets::CVarCheckbox(
                "Constant Distance Backflips and Sidehops", "gEnhancements.Restorations.ConstantFlipsHops",
                { .tooltip = "Backflips and Sidehops travel a constant distance as they did in OOT." });
            UIWidgets::CVarCheckbox(
                "Power Crouch Stab", "gEnhancements.Restorations.PowerCrouchStab",
                { .tooltip =
                      "Crouch stabs will use the power of Link's previous melee attack, as is in MM JP 1.0 and OOT." });
            UIWidgets::CVarCheckbox("Side Rolls", "gEnhancements.Restorations.SideRoll",
                                    { .tooltip = "Restores side rolling from OOT." });
            UIWidgets::CVarCheckbox("Tatl ISG", "gEnhancements.Restorations.TatlISG",
                                    { .tooltip = "Restores Navi ISG from OOT, but now with Tatl." });

            ImGui::EndMenu();
        }

        if (UIWidgets::BeginMenu("Songs/Playback")) {
            UIWidgets::CVarCheckbox(
                "Enable Sun's Song", "gEnhancements.Songs.EnableSunsSong",
                { .tooltip = "Enables the partially implemented Sun's Song. RIGHT-DOWN-UP-RIGHT-DOWN-UP to play it. "
                             "This song will make time move very fast until either Link moves to a different scene, "
                             "or when the time switches to a new time period." });
            UIWidgets::CVarCheckbox("Dpad Ocarina", "gEnhancements.Playback.DpadOcarina",
                                    { .tooltip = "Enables using the Dpad for Ocarina playback." });
            UIWidgets::CVarCheckbox("Prevent Dropped Ocarina Inputs", "gEnhancements.Playback.NoDropOcarinaInput",
                                    { .tooltip = "Prevent dropping inputs when playing the ocarina quickly" });

            ImGui::EndMenu();
        }

        if (mHudEditorWindow) {
            UIWidgets::WindowButton("Hud Editor", "gWindows.HudEditor", mHudEditorWindow,
                                    { .tooltip = "Enables the Hud Editor window, allowing you to edit your hud" });
        }

        ImGui::EndMenu();
    }
}

void DrawCheatsMenu() {
    if (UIWidgets::BeginMenu("Cheats")) {
        UIWidgets::CVarCheckbox("Infinite Health", "gCheats.InfiniteHealth");
        UIWidgets::CVarCheckbox("Infinite Magic", "gCheats.InfiniteMagic");
        UIWidgets::CVarCheckbox("Infinite Rupees", "gCheats.InfiniteRupees");
        UIWidgets::CVarCheckbox("Infinite Consumables", "gCheats.InfiniteConsumables");
        UIWidgets::CVarCheckbox("Unbreakable Razor Sword", "gCheats.UnbreakableRazorSword");
        UIWidgets::CVarCheckbox("Unrestricted Items", "gCheats.UnrestrictedItems");
        if (UIWidgets::CVarCheckbox("Moon Jump on L", "gCheats.MoonJumpOnL",
                                    { .tooltip = "Holding L makes you float into the air" })) {
            RegisterMoonJumpOnL();
        }
        UIWidgets::CVarCheckbox("No Clip", "gCheats.NoClip");

        ImGui::EndMenu();
    }
}

extern std::shared_ptr<Ship::GuiWindow> mStatsWindow;
extern std::shared_ptr<Ship::GuiWindow> mConsoleWindow;
extern std::shared_ptr<Ship::GuiWindow> mGfxDebuggerWindow;
extern std::shared_ptr<SaveEditorWindow> mSaveEditorWindow;
extern std::shared_ptr<ActorViewerWindow> mActorViewerWindow;
extern std::shared_ptr<CollisionViewerWindow> mCollisionViewerWindow;
extern std::shared_ptr<EventLogWindow> mEventLogWindow;

const char* logLevels[] = {
    "trace", "debug", "info", "warn", "error", "critical", "off",
};

void DrawDeveloperToolsMenu() {
    if (UIWidgets::BeginMenu("Developer Tools", UIWidgets::Colors::Yellow)) {
        UIWidgets::CVarCheckbox("Debug Mode", "gDeveloperTools.DebugEnabled",
                                { .tooltip = "Enables Debug Mode, allowing you to select maps with L + R + Z." });

        if (CVarGetInteger("gDeveloperTools.DebugEnabled", 0)) {
            UIWidgets::CVarCheckbox(
                "Better Map Select", "gDeveloperTools.BetterMapSelect.Enabled",
                { .tooltip = "Overrides the original map select with a translated, more user-friendly version." });

            if (UIWidgets::CVarCombobox(
                    "Debug Save File Mode", "gDeveloperTools.DebugSaveFileMode", debugSaveOptions,
                    { .tooltip =
                          "Change the behavior of creating saves while debug mode is enabled:\n\n"
                          "- Empty Save: The default 3 heart save file in first cycle\n"
                          "- Vanilla Debug Save: Uses the title screen save info (8 hearts, all items and masks)\n"
                          "- 100\% Save: All items, equipment, mask, quast status and bombers notebook complete" })) {
                RegisterDebugSaveCreate();
            }
        }

        if (UIWidgets::CVarCheckbox("Prevent Actor Update", "gDeveloperTools.PreventActorUpdate")) {
            RegisterPreventActorUpdateHooks();
        }
        if (UIWidgets::CVarCheckbox("Prevent Actor Draw", "gDeveloperTools.PreventActorDraw")) {
            RegisterPreventActorDrawHooks();
        }
        if (UIWidgets::CVarCheckbox("Prevent Actor Init", "gDeveloperTools.PreventActorInit")) {
            RegisterPreventActorInitHooks();
        }
        if (UIWidgets::CVarCombobox("Log Level", "gDeveloperTools.LogLevel", logLevels,
                                    {
                                        .tooltip = "The log level determines which messages are printed to the "
                                                   "console. This does not affect the log file output",
                                        .defaultIndex = 1,
                                    })) {
            Ship::Context::GetInstance()->GetLogger()->set_level(
                (spdlog::level::level_enum)CVarGetInteger("gDeveloperTools.LogLevel", 1));
        }

        if (gPlayState != NULL) {
            ImGui::Separator();
            UIWidgets::Checkbox(
                "Frame Advance", (bool*)&gPlayState->frameAdvCtx.enabled,
                { .tooltip = "This allows you to advance through the game one frame at a time on command. "
                             "To advance a frame, hold Z and tap R on the second controller. Holding Z "
                             "and R will advance a frame every half second. You can also use the buttons below." });
            if (gPlayState->frameAdvCtx.enabled) {
                if (UIWidgets::Button("Advance 1", { .size = UIWidgets::Sizes::Inline })) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
                ImGui::SameLine();
                UIWidgets::Button("Advance (Hold)", { .size = UIWidgets::Sizes::Inline });
                if (ImGui::IsItemActive()) {
                    CVarSetInteger("gDeveloperTools.FrameAdvanceTick", 1);
                }
            }
        }
        RenderWarpPointSection();
        ImGui::Separator();
        if (mCollisionViewerWindow) {
            UIWidgets::WindowButton("Collision Viewer", "gWindows.CollisionViewer", mCollisionViewerWindow,
                                    { .tooltip = "Draws collision to the screen" });
        }
        if (mStatsWindow) {
            UIWidgets::WindowButton(
                "Stats", "gWindows.Stats", mStatsWindow,
                { .tooltip = "Shows the stats window, with your FPS and frametimes, and the OS you're playing on" });
        }
        if (mConsoleWindow) {
            UIWidgets::WindowButton(
                "Console", "gWindows.Console", mConsoleWindow,
                { .tooltip =
                      "Enables the console window, allowing you to input commands, type help for some examples" });
        }
        if (mGfxDebuggerWindow) {
            UIWidgets::WindowButton(
                "Gfx Debugger", "gWindows.GfxDebugger", mGfxDebuggerWindow,
                { .tooltip =
                      "Enables the Gfx Debugger window, allowing you to input commands, type help for some examples" });
        }
        if (mSaveEditorWindow) {
            UIWidgets::WindowButton(
                "Save Editor", "gWindows.SaveEditor", mSaveEditorWindow,
                { .tooltip = "Enables the Save Editor window, allowing you to edit your save file" });
        }
        if (mActorViewerWindow) {
            UIWidgets::WindowButton(
                "Actor Viewer", "gWindows.ActorViewer", mActorViewerWindow,
                { .tooltip = "Enables the Actor Viewer window, allowing you to view actors in the world." });
        }
        if (mEventLogWindow) {
            UIWidgets::WindowButton("Event Log", "gWindows.EventLog", mEventLogWindow);
        }
        ImGui::EndMenu();
    }
}

void BenMenuBar::DrawElement() {
    if (ImGui::BeginMenuBar()) {
        DrawMenuBarIcon();

        static ImVec2 sWindowPadding(8.0f, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, sWindowPadding);

        ImGui::SetCursorPosY(0.0f);

        DrawBenMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawSettingsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawEnhancementsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawCheatsMenu();

        ImGui::SetCursorPosY(0.0f);

        DrawDeveloperToolsMenu();

        ImGui::SetCursorPosY(0.0f);

        ImGui::PopStyleVar(1);
        ImGui::EndMenuBar();
    }
}
} // namespace BenGui
// namespace BenGui
