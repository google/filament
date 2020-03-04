// Copyright 2018 The Draco Authors.
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
#ifndef DRACO_CORE_BOUNDING_BOX_H_
#define DRACO_CORE_BOUNDING_BOX_H_

#include "draco/core/vector_d.h"

namespace draco {

// Class for detecting the bounding box of a point_cloud or mesh.
// Use the minimum point and the maximum point to define the bounding box.
// TODO(xiaoxumeng): Change the class of BoundingBox to a template, similar to
// draco/src/draco/core/vector_d.h
class BoundingBox {
 public:
  // Initialization
  BoundingBox(const Vector3f &min_point_in, const Vector3f &max_point_in);

  inline const Vector3f &min_point() const { return min_point_; }
  inline const Vector3f &max_point() const { return max_point_; }

  // Conditionally updates the bounding box.
  // TODO(xiaoxumeng): Change the function to a template function and change the
  // argument to an iterator.
  inline void update_bounding_box(const Vector3f &new_point) {
    for (int i = 0; i < 3; i++) {
      if (new_point[i] < min_point_[i]) {
        min_point_[i] = new_point[i];
      }
      if (new_point[i] > max_point_[i]) {
        max_point_[i] = new_point[i];
      }
    }
  }

 private:
  Vector3f min_point_;
  Vector3f max_point_;
};
}  // namespace draco

#endif  //  DRACO_CORE_BOUNDING_BOX_H_
