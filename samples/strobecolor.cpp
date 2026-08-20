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

#include "common/arguments.h"
#include "common/SampleConfig.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/View.h>

#include <cmath>

using namespace filament;

namespace {
struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
    Skybox* skybox;
};
} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        app->skybox = Skybox::Builder().color({ 0.0, 0.25, 0.5, 1.0 }).build(*engine);
        scene->setSkybox(app->skybox);
        view->setPostProcessingEnabled(false);
    };

    auto cleanup = [app](Engine*, View*, Scene*) {};

    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .animation([app](Engine*, View* view, double now) {
                            constexpr float SPEED = 4;
                            float r = 0.5f + 0.5f * std::sin(SPEED * now);
                            float g = 0.5f + 0.5f * std::sin(SPEED * now + M_PI * 2 / 3);
                            float b = 0.5f + 0.5f * std::sin(SPEED * now + M_PI * 4 / 3);
                            app->skybox->setColor({ r, g, b, 1.0 });
                        })
                        .build();

    app->filamentApp = fApp.get();
    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "strobecolor";
    samples::handleCommandLineArguments(argc, argv, &config);

    auto dm = samples::getDisplayManager(config);
    auto fApp = createSampleApp(config, dm.get(), nullptr);
    fApp->run();

    return 0;
}
#endif
