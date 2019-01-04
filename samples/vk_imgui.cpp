/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <imgui.h>

#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Scene.h>

#include "app/Config.h"
#include "app/FilamentApp.h"

int main(int argc, char** argv) {
    bool showDemo = false;
    bool showMetrics = false;
    auto imgui = [&showDemo, &showMetrics] (filament::Engine*, filament::View*) {
        // In ImGui, the window title is a unique identifier, so don't call this "ImGui Demo" to
        // avoid colliding with ImGui::ShowDemoWindow().
        ImGui::Begin("ImGui", nullptr, ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Close"))  {
                    FilamentApp::get().close();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::Checkbox("Widgets", &showDemo);
        ImGui::Checkbox("Metrics", &showMetrics);
        if (showDemo) {
            ImGui::ShowDemoWindow(&showDemo);
        }
        if (showMetrics) {
            ImGui::ShowMetricsWindow(&showMetrics);
        }
        ImGui::End();
    };
    Config config;
    config.backend = filament::Engine::Backend::VULKAN;
    config.title = "ImGui Demo";
    auto nop = [](filament::Engine*, filament::View*, filament::Scene*) {};
    FilamentApp::get().run(config, nop, nop, imgui);

    return 0;
}
