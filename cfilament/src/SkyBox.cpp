/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <filament/Skybox.h>

#include "API.h"

using namespace filament;

Skybox::Builder *Filament_Skybox_CreateBuilder() {
  return new Skybox::Builder{};
}

void Filament_Skybox_DestroyBuilder(Skybox::Builder *builder) {
  delete builder;
}

void Filament_Skybox_BuilderEnvironment(Skybox::Builder *builder, Texture *texture) {
  builder->environment(texture);
}

void Filament_Skybox_BuilderShowSun(Skybox::Builder *builder, FBool show) {
  builder->showSun(show);
}

Skybox *Filament_Skybox_BuilderBuild(Skybox::Builder *builder, Engine *engine) {
  return builder->build(*engine);
}

void Filament_Skybox_SetLayerMask(Skybox *skybox, uint8_t select, uint8_t value) {
  skybox->setLayerMask(select, value);
}

uint8_t Filament_Skybox_GetLayerMask(Skybox *skybox) {
  return skybox->getLayerMask();
}
