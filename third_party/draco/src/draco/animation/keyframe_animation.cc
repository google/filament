// Copyright 2017 The Draco Authors.
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
#include "draco/animation/keyframe_animation.h"

namespace draco {

KeyframeAnimation::KeyframeAnimation() {}

bool KeyframeAnimation::SetTimestamps(
    const std::vector<TimestampType> &timestamp) {
  // Already added attributes.
  const int32_t num_frames = timestamp.size();
  if (num_attributes() > 0) {
    // Timestamp attribute could be added only once.
    if (timestamps()->size()) {
      return false;
    } else {
      // Check if the number of frames is consistent with
      // the existing keyframes.
      if (num_frames != num_points()) {
        return false;
      }
    }
  } else {
    // This is the first attribute.
    set_num_frames(num_frames);
  }

  // Add attribute for time stamp data.
  std::unique_ptr<PointAttribute> timestamp_att =
      std::unique_ptr<PointAttribute>(new PointAttribute());
  timestamp_att->Init(GeometryAttribute::GENERIC, 1, DT_FLOAT32, false,
                      num_frames);
  for (PointIndex i(0); i < num_frames; ++i) {
    timestamp_att->SetAttributeValue(timestamp_att->mapped_index(i),
                                     &timestamp[i.value()]);
  }
  this->SetAttribute(kTimestampId, std::move(timestamp_att));
  return true;
}

}  // namespace draco
