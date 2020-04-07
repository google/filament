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

#include <filament/Engine.h>
#include <filament/View.h>
#include <filament/Scene.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <cmath>

using namespace filament;

int main(int argc, char** argv) {
    Config config;
    config.title = "strobecolor";
    config.backend = Engine::Backend::VULKAN;

    auto setup = [](Engine*, View* view, Scene*) {
        view->setClearColor({0.0, 0.25, 0.5, 1.0});
        view->setPostProcessingEnabled(false);
    };

    auto cleanup = [](Engine*, View*, Scene*) {
    };

    FilamentApp::get().animate([](Engine*, View* view, double now) {
        constexpr float SPEED = 4;
        float r = 0.5f + 0.5f * std::sin(SPEED * now);
        float g = 0.5f + 0.5f * std::sin(SPEED * now + M_PI * 2 / 3);
        float b = 0.5f + 0.5f * std::sin(SPEED * now + M_PI * 4 / 3);
        view->setClearColor({r, g, b, 1.0});
    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
