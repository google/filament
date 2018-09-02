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

#include <filament/Scene.h>

#include "API.h"

using namespace filament;
using namespace utils;

void Filament_Scene_SetSkybox(Scene *scene, Skybox *skybox) {
    scene->setSkybox(skybox);
}

void Filament_Scene_SetIndirectLight(Scene *scene,
        IndirectLight *indirectLight) {
    scene->setIndirectLight(indirectLight);
}

void Filament_Scene_AddEntity(Scene *scene, Entity entity) {
    scene->addEntity(entity);
}

void Filament_Scene_Remove(Scene *scene, Entity entity) {
    scene->remove(entity);
}

int Filament_Scene_GetRenderableCount(Scene *scene) {
    return scene->getRenderableCount();
}

int Filament_Scene_GetLightCount(Scene *scene) {
    return scene->getLightCount();
}
