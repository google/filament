/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef GLTFIO_FFILAMENTINSTANCE_H
#define GLTFIO_FFILAMENTINSTANCE_H

#include <gltfio/FilamentInstance.h>

#include <utils/Entity.h>

#include <math/mat4.h>

#include <tsl/robin_map.h>

#include <string>
#include <vector>

#include "upcast.h"

struct cgltf_node;

namespace gltfio {

struct FFilamentAsset;
class Animator;

struct Skin {
    std::string name;
    std::vector<filament::math::mat4f> inverseBindMatrices;
    std::vector<utils::Entity> joints;
    std::vector<utils::Entity> targets;
};

using SkinVector = std::vector<Skin>;
using NodeMap = tsl::robin_map<const cgltf_node*, utils::Entity>;

struct FFilamentInstance : public FilamentInstance {
    std::vector<utils::Entity> entities;
    utils::Entity root;
    Animator* animator;
    FFilamentAsset* owner;
    SkinVector skins;
    NodeMap nodeMap;
    Animator* getAnimator() noexcept;
};

FILAMENT_UPCAST(FilamentInstance)

} // namespace gltfio

#endif // GLTFIO_FFILAMENTINSTANCE_H
