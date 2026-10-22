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

#ifndef DRACO_SCENE_SCENE_ARE_EQUIVALENT_H_
#define DRACO_SCENE_SCENE_ARE_EQUIVALENT_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED
#include "draco/scene/scene.h"

namespace draco {

// A functor to compare two scenes for equivalency up to permutation of mesh
// vertices.
class SceneAreEquivalent {
 public:
  // Returns true if both scenes are equivalent up to permutation of
  // the internal order of mesh vertices. This includes all attributes.
  bool operator()(const Scene &scene0, const Scene &scene1);

 private:
  static bool AreEquivalent(const Mesh &mesh0, const Mesh &mesh1);
  static bool AreEquivalent(const SceneNode &node0, const SceneNode &node1);
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_SCENE_SCENE_ARE_EQUIVALENT_H_
