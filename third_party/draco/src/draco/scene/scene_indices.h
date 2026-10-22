// Copyright 2019 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifdef DRACO_TRANSCODER_SUPPORTED
#ifndef DRACO_SCENE_SCENE_INDICES_H_
#define DRACO_SCENE_SCENE_INDICES_H_

#include <inttypes.h>

#include <limits>

#include "draco/core/draco_index_type.h"

namespace draco {

// Index of a mesh in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, MeshIndex)

// Index of a mesh instance in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, MeshInstanceIndex)

// Index of a mesh group in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, MeshGroupIndex)

// Index of a node in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, SceneNodeIndex)

// Index of an animation in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, AnimationIndex)

// Index of a skin in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, SkinIndex)

// Index of a light in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, LightIndex)

// Index of a mesh group GPU instancing in a scene.
DEFINE_NEW_DRACO_INDEX_TYPE(uint32_t, InstanceArrayIndex)

// Constants denoting invalid indices.
static constexpr MeshIndex kInvalidMeshIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr MeshInstanceIndex kInvalidMeshInstanceIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr MeshGroupIndex kInvalidMeshGroupIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr SceneNodeIndex kInvalidSceneNodeIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr AnimationIndex kInvalidAnimationIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr SkinIndex kInvalidSkinIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr LightIndex kInvalidLightIndex(
    std::numeric_limits<uint32_t>::max());
static constexpr InstanceArrayIndex kInvalidInstanceArrayIndex(
    std::numeric_limits<uint32_t>::max());

}  // namespace draco

#endif  // DRACO_SCENE_SCENE_INDICES_H_
#endif  // DRACO_TRANSCODER_SUPPORTED
