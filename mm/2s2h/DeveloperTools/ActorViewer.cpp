#include "ActorViewer.h"
#include "2s2h/BenGui/UIWidgets.hpp"
#include "global.h"
#include "2s2h/Enhancements/GameInteractor/GameInteractor.h"

typedef struct ActorInfo {
    u16 id;
    u16 params;
    Vec3f pos;
    Vec3s rot;
} ActorInfo;

typedef enum Method {
    LIST,
    TARGET,
    HOLD,
} Method;

std::array<const char*, 12> acMapping = { "Switch", "Background",  "Player", "Explosive", "NPC",  "Enemy",
                                          "Prop",   "Item/Action", "Misc.",  "Boss",      "Door", "Chest" };

#define DEFINE_ACTOR(name, _enumValue, _allocType, _debugName, _humanName) { _enumValue, _humanName },
#define DEFINE_ACTOR_INTERNAL(_name, _enumValue, _allocType, _debugName, _humanName) { _enumValue, _humanName },
#define DEFINE_ACTOR_UNSET(_enumValue) { _enumValue, "Unset" },

std::unordered_map<s16, const char*> actorDescriptions = {
#include "tables/actor_table.h"
};

#undef DEFINE_ACTOR
#undef DEFINE_ACTOR_INTERNAL
#undef DEFINE_ACTOR_UNSET

std::string GetActorDescription(u16 actorNum) {
    return actorDescriptions.contains(actorNum) ? actorDescriptions[actorNum] : "???";
}

std::vector<Actor*> GetCurrentSceneActors() {
    if (!gPlayState)
        return {};

    std::vector<Actor*> sceneActors;
    for (size_t category = ACTORCAT_SWITCH; category < ACTORCAT_MAX; category++) {
        Actor* currentActor = gPlayState->actorCtx.actorLists[category].first;
        if (currentActor != nullptr) {
            while (currentActor != nullptr) {
                sceneActors.push_back(currentActor);
                currentActor = currentActor->next;
            }
        }
    }
    return sceneActors;
}

static bool needs_reset = false;

static Actor* display{};
static Actor* fetch;
static ActorInfo newActor;
static std::vector<Actor*> list;

static s16 lastSceneId = 0;
static s16 newActorId = 0;
static u8 category = 0;
static s8 method = -1;
static std::string filler = "Please select";
static HOOK_ID preventActorDrawHookId = 0;
static HOOK_ID preventActorUpdateHookId = 0;

void ResetVariables() {
    display = fetch = {};
    newActor = {};
    filler = "Please select";
    GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorDraw>(preventActorDrawHookId);
    GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorUpdate>(preventActorUpdateHookId);
    preventActorDrawHookId = 0;
    preventActorUpdateHookId = 0;
}

void ActorViewerWindow::DrawElement() {
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Actor Viewer", &mIsVisible, ImGuiWindowFlags_NoFocusOnAppearing)) {
        ImGui::End();
        return;
    }

    if (gPlayState != nullptr) {
        needs_reset = lastSceneId != gPlayState->sceneId;
        if (needs_reset) {
            ResetVariables();
            lastSceneId = gPlayState->sceneId;
            needs_reset = false;
        }

        if (ImGui::BeginCombo("Actor", filler.c_str())) {
            if (gPlayState != nullptr) {
                list = GetCurrentSceneActors();
                lastSceneId = gPlayState->sceneId;
            }
            if (!list.empty()) {
                u8 count = 0;
                u8 prev_category = 0xff;
                for (size_t i = 0; i < list.size(); i++) {
                    std::string label = acMapping[list[i]->category];
                    if (list[i]->category != prev_category) {
                        prev_category = list[i]->category;
                        count = 1;
                    } else {
                        count++;
                    }
                    label += " " + std::to_string(count) + ": " + GetActorDescription(list[i]->id);
                    if (ImGui::Selectable(label.c_str())) {
                        method = LIST;
                        display = list[i];
                        newActorId = i;
                        filler = label;
                        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorDraw>(
                            preventActorDrawHookId);
                        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorUpdate>(
                            preventActorUpdateHookId);
                        preventActorDrawHookId = 0;
                        preventActorUpdateHookId = 0;
                        break;
                    }
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::TreeNode("Selected Actor")) {
            if (display != nullptr) {
                ImGui::BeginGroup();
                ImGui::Text("ID: %hd", display->id);
                ImGui::Text("Description: %s", GetActorDescription(display->id).c_str());
                ImGui::Text("Category: %s", acMapping[display->category]);
                ImGui::Text("Params: %hd", display->params);
                ImGui::EndGroup();

                ImGui::BeginGroup();
                if (preventActorDrawHookId) {
                    if (ImGui::Button("Continue Drawing", ImVec2(ImGui::GetFontSize() * 10, 0))) {
                        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorDraw>(
                            preventActorDrawHookId);
                        preventActorDrawHookId = 0;
                    }
                } else {
                    if (ImGui::Button("Stop Drawing", ImVec2(ImGui::GetFontSize() * 10, 0))) {
                        preventActorDrawHookId =
                            GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::ShouldActorDraw>(
                                (uintptr_t)display, [](Actor* _, bool* result) { *result = false; });
                    }
                }
                ImGui::SameLine();
                if (preventActorUpdateHookId) {
                    if (ImGui::Button("Continue Updating", ImVec2(ImGui::GetFontSize() * 10, 0))) {
                        GameInteractor::Instance->UnregisterGameHookForPtr<GameInteractor::ShouldActorUpdate>(
                            preventActorUpdateHookId);
                        preventActorUpdateHookId = 0;
                    }
                } else {
                    if (ImGui::Button("Stop Updating", ImVec2(ImGui::GetFontSize() * 10, 0))) {
                        preventActorUpdateHookId =
                            GameInteractor::Instance->RegisterGameHookForPtr<GameInteractor::ShouldActorUpdate>(
                                (uintptr_t)display, [](Actor* _, bool* result) { *result = false; });
                    }
                }
                ImGui::EndGroup();

                ImGui::PushItemWidth(ImGui::GetFontSize() * 10);

                ImGui::BeginGroup();
                ImGui::Text("Actor Position");
                ImGui::InputScalar("x", ImGuiDataType_Float, &display->world.pos.x);
                ImGui::SameLine();
                ImGui::InputScalar("y", ImGuiDataType_Float, &display->world.pos.y);
                ImGui::SameLine();
                ImGui::InputScalar("z", ImGuiDataType_Float, &display->world.pos.z);
                ImGui::EndGroup();

                ImGui::BeginGroup();
                ImGui::Text("Actor Rotation");
                ImGui::InputScalar("rx", ImGuiDataType_S16, &display->world.rot.x);
                ImGui::SameLine();
                ImGui::InputScalar("ry", ImGuiDataType_S16, &display->world.rot.y);
                ImGui::SameLine();
                ImGui::InputScalar("rz", ImGuiDataType_S16, &display->world.rot.z);
                ImGui::EndGroup();

                ImGui::PopItemWidth();
                ImGui::PushItemWidth(ImGui::GetFontSize() * 5);

                if ((display->category == ACTORCAT_BOSS || display->category == ACTORCAT_ENEMY) &&
                    display->colChkInfo.health > 0) {
                    ImGui::InputScalar("Enemy Health", ImGuiDataType_U8, &display->colChkInfo.health);
                }
                ImGui::PopItemWidth();

                ImGui::BeginGroup();
                ImGui::Text("Flags");
                UIWidgets::DrawFlagArray32("flags", display->flags);
                ImGui::EndGroup();

                ImGui::BeginGroup();
                ImGui::Text("BG Check");
                UIWidgets::DrawFlagArray16("bgCheckFlags", display->bgCheckFlags);
                ImGui::EndGroup();
            }

            if (UIWidgets::Button("Refresh")) {
                list = GetCurrentSceneActors();
                display = list[newActorId];
            }

            if (UIWidgets::Button("Go to Actor")) {
                Player* player = GET_PLAYER(gPlayState);
                if (display != nullptr) {
                    Math_Vec3f_Copy(&player->actor.world.pos, &display->world.pos);
                    Math_Vec3f_Copy(&player->actor.home.pos, &display->world.pos);
                }
            }

            if (UIWidgets::Button("Fetch: Target", { .tooltip = "Grabs actor with target arrow above it." })) {
                Player* player = GET_PLAYER(gPlayState);
                fetch = player->lockOnActor;
                if (fetch != nullptr) {
                    display = fetch;
                    category = fetch->category;
                    list = GetCurrentSceneActors();
                    method = TARGET;
                } else {
                    display = {};
                }
            }
            if (UIWidgets::Button("Fetch: Held", { .tooltip = "Grabs actor Link is currently holding." })) {
                Player* player = GET_PLAYER(gPlayState);
                fetch = player->heldActor;
                if (fetch != nullptr) {
                    display = fetch;
                    category = fetch->category;
                    list = GetCurrentSceneActors();
                    method = HOLD;
                } else {
                    display = {};
                }
            }

            if (UIWidgets::Button("Kill", { .color = UIWidgets::Colors::Red }) && display != nullptr &&
                display->id != ACTOR_PLAYER) {
                Actor_Kill(display);
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("New...")) {
            ImGui::PushItemWidth(ImGui::GetFontSize() * 10);
            ImU16 one = 1;

            ImGui::Text("%s", GetActorDescription(newActor.id).c_str());
            ImGui::InputScalar("ID", ImGuiDataType_S16, &newActor.id, &one);
            ImGui::InputScalar("Params", ImGuiDataType_S16, &newActor.params, &one);

            ImGui::BeginGroup();
            ImGui::Text("New Actor Position");
            ImGui::InputScalar("x", ImGuiDataType_Float, &newActor.pos.x);
            ImGui::SameLine();
            ImGui::InputScalar("y", ImGuiDataType_Float, &newActor.pos.y);
            ImGui::SameLine();
            ImGui::InputScalar("z", ImGuiDataType_Float, &newActor.pos.z);
            ImGui::EndGroup();

            ImGui::BeginGroup();
            ImGui::Text("New Actor Rotation");
            ImGui::InputScalar("rX", ImGuiDataType_S16, &newActor.rot.x);
            ImGui::SameLine();
            ImGui::InputScalar("rY", ImGuiDataType_S16, &newActor.rot.y);
            ImGui::SameLine();
            ImGui::InputScalar("rZ", ImGuiDataType_S16, &newActor.rot.z);
            ImGui::EndGroup();

            UIWidgets::CVarCheckbox("Disable Object Dependency", "gDeveloperTools.DisableObjectDependency");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::TextColored(
                    ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                    "Warning: This will cause some actors to spawn in places they normally wouldn't,\nplease ensure "
                    "this is disabled before reporting any actor related issues");
                ImGui::EndTooltip();
            }

            if (UIWidgets::Button("Fetch from Link")) {
                Player* player = GET_PLAYER(gPlayState);
                newActor.pos = player->actor.world.pos;
                newActor.rot = player->actor.world.rot;
            }

            if (UIWidgets::Button("Spawn", { .color = UIWidgets::Colors::Green })) {
                Actor_Spawn(&gPlayState->actorCtx, gPlayState, newActor.id, newActor.pos.x, newActor.pos.y,
                            newActor.pos.z, newActor.rot.x, newActor.rot.y, newActor.rot.z, newActor.params);
            }

            if (UIWidgets::Button("Spawn as Child", { .color = UIWidgets::Colors::Green })) {
                Actor* parent = display;
                if (parent != nullptr) {
                    Actor_SpawnAsChild(&gPlayState->actorCtx, parent, gPlayState, newActor.id, newActor.pos.x,
                                       newActor.pos.y, newActor.pos.z, newActor.rot.x, newActor.rot.y, newActor.rot.z,
                                       newActor.params);
                } else {
                    Audio_PlaySfx(NA_SE_SY_ERROR);
                }
            }

            if (UIWidgets::Button("Reset")) {
                newActor = {};
            }

            ImGui::TreePop();
        }
    } else {
        ImGui::Text("Playstate needed for actors!");
    }
    ImGui::End();
}

void ActorViewerWindow::InitElement() {
}
