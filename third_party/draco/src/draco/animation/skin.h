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
#ifndef DRACO_ANIMATION_SKIN_H_
#define DRACO_ANIMATION_SKIN_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include <vector>

#include "draco/animation/node_animation_data.h"
#include "draco/scene/scene_indices.h"

namespace draco {

// This class is used to store information on animation skins.
class Skin {
 public:
  Skin() : joint_root_index_(-1) {}

  void Copy(const Skin &s);

  NodeAnimationData &GetInverseBindMatrices() { return inverse_bind_matrices_; }
  const NodeAnimationData &GetInverseBindMatrices() const {
    return inverse_bind_matrices_;
  }

  int AddJoint(SceneNodeIndex index) {
    joints_.push_back(index);
    return joints_.size() - 1;
  }
  int NumJoints() const { return joints_.size(); }
  SceneNodeIndex GetJoint(int index) const { return joints_[index]; }
  SceneNodeIndex &GetJoint(int index) { return joints_[index]; }
  const std::vector<SceneNodeIndex> &GetJoints() const { return joints_; }

  void SetJointRoot(SceneNodeIndex index) { joint_root_index_ = index; }
  SceneNodeIndex GetJointRoot() const { return joint_root_index_; }

 private:
  NodeAnimationData inverse_bind_matrices_;

  // List of node indices that make up the joint hierarchy.
  std::vector<SceneNodeIndex> joints_;
  SceneNodeIndex joint_root_index_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_ANIMATION_SKIN_H_
