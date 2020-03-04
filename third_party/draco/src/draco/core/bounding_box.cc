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

#include "draco/core/bounding_box.h"

namespace draco {

BoundingBox::BoundingBox(const Vector3f &min_point_in,
                         const Vector3f &max_point_in)
    : min_point_(min_point_in), max_point_(max_point_in) {}

}  // namespace draco
