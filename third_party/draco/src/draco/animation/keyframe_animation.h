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
#ifndef DRACO_ANIMATION_KEYFRAME_ANIMATION_H_
#define DRACO_ANIMATION_KEYFRAME_ANIMATION_H_

#include <vector>

#include "draco/point_cloud/point_cloud.h"

namespace draco {

// Class for holding keyframe animation data. It will have two or more
// attributes as a point cloud. The first attribute is always the timestamp
// of the animation. Each KeyframeAnimation could have multiple animations with
// the same number of frames. Each animation will be treated as a point
// attribute.
class KeyframeAnimation : public PointCloud {
 public:
  // Force time stamp to be float type.
  using TimestampType = float;

  KeyframeAnimation();

  // Animation must have only one timestamp attribute.
  // This function must be called before adding any animation data.
  // Returns false if timestamp already exists.
  bool SetTimestamps(const std::vector<TimestampType> &timestamp);

  // Returns an id for the added animation data. This id will be used to
  // identify this animation.
  // Returns -1 if error, e.g. number of frames is not consistent.
  // Type |T| should be consistent with |DataType|, e.g:
  //    float - DT_FLOAT32,
  //    int32_t - DT_INT32, ...
  template <typename T>
  int32_t AddKeyframes(DataType data_type, uint32_t num_components,
                       const std::vector<T> &data);

  const PointAttribute *timestamps() const {
    return GetAttributeByUniqueId(kTimestampId);
  }
  const PointAttribute *keyframes(int32_t animation_id) const {
    return GetAttributeByUniqueId(animation_id);
  }

  // Number of frames should be equal to number points in the point cloud.
  void set_num_frames(int32_t num_frames) { set_num_points(num_frames); }
  int32_t num_frames() const { return static_cast<int32_t>(num_points()); }

  int32_t num_animations() const { return num_attributes() - 1; }

 private:
  // Attribute id of timestamp is fixed to 0.
  static constexpr int32_t kTimestampId = 0;
};

template <typename T>
int32_t KeyframeAnimation::AddKeyframes(DataType data_type,
                                        uint32_t num_components,
                                        const std::vector<T> &data) {
  // TODO(draco-eng): Verify T is consistent with |data_type|.
  if (num_components == 0) {
    return -1;
  }
  // If timestamps is not added yet, then reserve attribute 0 for timestamps.
  if (!num_attributes()) {
    // Add a temporary attribute with 0 points to fill attribute id 0.
    std::unique_ptr<PointAttribute> temp_att =
        std::unique_ptr<PointAttribute>(new PointAttribute());
    temp_att->Init(GeometryAttribute::GENERIC, num_components, data_type, false,
                   0);
    this->AddAttribute(std::move(temp_att));

    set_num_frames(data.size() / num_components);
  }

  if (data.size() != num_components * num_frames()) {
    return -1;
  }

  std::unique_ptr<PointAttribute> keyframe_att =
      std::unique_ptr<PointAttribute>(new PointAttribute());
  keyframe_att->Init(GeometryAttribute::GENERIC, num_components, data_type,
                     false, num_frames());
  const size_t stride = num_components;
  for (PointIndex i(0); i < num_frames(); ++i) {
    keyframe_att->SetAttributeValue(keyframe_att->mapped_index(i),
                                    &data[i.value() * stride]);
  }
  return this->AddAttribute(std::move(keyframe_att));
}

}  // namespace draco

#endif  // DRACO_ANIMATION_KEYFRAME_ANIMATION_H_
