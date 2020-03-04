// Copyright 2016 The Draco Authors.
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
#include "draco/compression/point_cloud/point_cloud_sequential_encoder.h"

#include "draco/compression/attributes/linear_sequencer.h"
#include "draco/compression/attributes/sequential_attribute_encoders_controller.h"

namespace draco {

Status PointCloudSequentialEncoder::EncodeGeometryData() {
  const int32_t num_points = point_cloud()->num_points();
  buffer()->Encode(num_points);
  return OkStatus();
}

bool PointCloudSequentialEncoder::GenerateAttributesEncoder(int32_t att_id) {
  // Create only one attribute encoder that is going to encode all points in a
  // linear sequence.
  if (att_id == 0) {
    // Create a new attribute encoder only for the first attribute.
    AddAttributesEncoder(std::unique_ptr<AttributesEncoder>(
        new SequentialAttributeEncodersController(
            std::unique_ptr<PointsSequencer>(
                new LinearSequencer(point_cloud()->num_points())),
            att_id)));
  } else {
    // Reuse the existing attribute encoder for other attributes.
    attributes_encoder(0)->AddAttributeId(att_id);
  }
  return true;
}

void PointCloudSequentialEncoder::ComputeNumberOfEncodedPoints() {
  set_num_encoded_points(point_cloud()->num_points());
}

}  // namespace draco
