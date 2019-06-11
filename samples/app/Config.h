/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SAMPLE_CONFIG_H
#define TNT_FILAMENT_SAMPLE_CONFIG_H

#include <string>

#include <filament/Engine.h>

struct Config {
    std::string title;
    std::string iblDirectory;
    float scale = 1.0f;
    bool splitView = false;
    filament::Engine::Backend backend = filament::Engine::Backend::OPENGL;
    bool resizeable = true;
};

#endif // TNT_FILAMENT_SAMPLE_CONFIG_H
