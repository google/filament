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
#ifndef DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_KD_TREE_ENCODER_H_
#define DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_KD_TREE_ENCODER_H_

#include "draco/compression/point_cloud/point_cloud_encoder.h"

namespace draco {

// Encodes a PointCloud using one of the available Kd-tree compression methods.
// See FloatPointsKdTreeEncoder and DynamicIntegerPointsKdTreeEncoder for more
// details. Currently, the input PointCloud must satisfy the following
// requirements to use this encoder:
//  1. PointCloud has only one attribute of type GeometryAttribute::POSITION.
//  2. The position attribute has three components (x,y,z).
//  3. The position values are stored as either DT_FLOAT32 or DT_UINT32.
//  4. If the position values are stored as DT_FLOAT32, quantization needs to
//     be enabled for the position attribute.
class PointCloudKdTreeEncoder : public PointCloudEncoder {
 public:
  uint8_t GetEncodingMethod() const override {
    return POINT_CLOUD_KD_TREE_ENCODING;
  }

 protected:
  Status EncodeGeometryData() override;
  bool GenerateAttributesEncoder(int32_t att_id) override;
  void ComputeNumberOfEncodedPoints() override;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_KD_TREE_ENCODER_H_
