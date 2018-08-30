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

#include <filament/TransformManager.h>
#include <utils/Entity.h>

#include "API.h"

using namespace utils;
using namespace filament;

FBool Filament_TransformManager_HasComponent(TransformManager *tm,
                                            Entity entity) {
  return tm->hasComponent((Entity &) entity);
}

int Filament_TransformManager_GetInstance(TransformManager *tm,
                                          Entity entity) {
  return tm->getInstance(entity);
}

int Filament_TransformManager_CreateUninitialized(TransformManager *tm,
                                                  Entity entity) {
  tm->create(entity);
  return tm->getInstance(entity);
}

int Filament_TransformManager_Create(TransformManager *tm,
                                     Entity entity, int parent,
                                     math::mat4f *localTransform) {

  if (localTransform) {
    tm->create(entity, (TransformManager::Instance) parent, *localTransform);
  } else {
    tm->create(entity, (TransformManager::Instance) parent);
  }
  return tm->getInstance(entity);
}

void Filament_TransformManager_Destroy(TransformManager *tm,
                                       Entity entity) {
  tm->destroy(entity);
}

void Filament_TransformManager_SetParent(TransformManager *tm, int i,
                                         int newParent) {
  tm->setParent((TransformManager::Instance) i,
                (TransformManager::Instance) newParent);
}

void Filament_TransformManager_SetTransform(
    TransformManager *tm, int i, math::mat4f *localTransform) {
  tm->setTransform((TransformManager::Instance) i, *localTransform);
}

void Filament_TransformManager_GetTransform(
    TransformManager *tm, int i, math::mat4f *outLocalTransform) {
  *outLocalTransform = tm->getTransform((TransformManager::Instance) i);
}

void Filament_TransformManager_GetWorldTransform(
    TransformManager *tm, int i, math::mat4f *outWorldTransform) {
  *outWorldTransform = tm->getWorldTransform((TransformManager::Instance) i);
}

void
Filament_TransformManager_OpenLocalTransformTransaction(TransformManager *tm) {
  tm->openLocalTransformTransaction();
}

void
Filament_TransformManager_CommitLocalTransformTransaction(TransformManager *tm) {
  tm->commitLocalTransformTransaction();
}
