/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENTAPP_FILAMENTAPPGUI_H
#define TNT_FILAMENTAPP_FILAMENTAPPGUI_H

#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>
#include <filament/View.h>

#include <utils/Path.h>

#include <vector>

namespace filament::app {
struct AppEvent;
class DisplayManager;
}

/**
 * FilamentAppGui encapsulates the graphical user interface components of FilamentApp.
 *
 * Its primary purpose is to abstract the ImGui implementation (filagui::ImGuiHelper)
 * away from the main FilamentApp code. This abstraction allows FilamentApp to be built
 * for platforms (such as Android) where ImGui is not supported or desired, without
 * littering the main codebase with preprocessor macros (e.g., #if defined(FILAMENTAPP_HAS_IMGUI)).
 *
 * When ImGui is disabled, this class degrades gracefully by acting as a no-op layer,
 * ensuring seamless multi-platform compilation while keeping the public headers clean.
 */
class FilamentAppGui {
public:
    FilamentAppGui(filament::Engine* engine, filament::View* view, const utils::Path& fontPath);
    ~FilamentAppGui();

    void processAppEvents(std::vector<filament::app::AppEvent> const& events);

    bool wantCaptureMouse() const;

    void render(float timeStep, filament::app::DisplayManager* displayManager, void* window,
            FilamentApp2::ImGuiCallback imguiCallback, bool mousePressed[3]);

    filament::View* getView() const noexcept;

private:
    struct Impl;
    Impl* pImpl;
};

#endif
