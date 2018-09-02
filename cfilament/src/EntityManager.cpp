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

#include <utils/EntityManager.h>

#include "API.h"

using namespace utils;

static EntityManager &GetEntityManager() {
    static EntityManager &sEntityManager = EntityManager::get();
    return sEntityManager;
}

void Filament_EntityManager_CreateEntities(FEntity *entities, int count) {
    GetEntityManager().create((size_t) count, reinterpret_cast<Entity *>(entities));
}

int Filament_EntityManager_CreateEntity() {
    return GetEntityManager().create().getId();
}

void Filament_EntityManager_DestroyEntities(FEntity *entities, int count) {
    GetEntityManager().destroy((size_t) count, reinterpret_cast<Entity *>(entities));
}

void Filament_EntityManager_DestroyEntity(FEntity entity) {
    GetEntityManager().destroy(convertEntity(entity));
}

FBool Filament_EntityManager_IsAlive(FEntity entity) {
    return GetEntityManager().isAlive(convertEntity(entity));
}
