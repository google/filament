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
#ifndef DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_SEQUENTIAL_ENCODER_H_
#define DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_SEQUENTIAL_ENCODER_H_

#include "draco/compression/point_cloud/point_cloud_encoder.h"

namespace draco {

// A basic point cloud encoder that iterates over all points and encodes all
// attribute values for every visited point. The attribute values encoding
// can be controlled using provided encoding option to enable features such
// as quantization or prediction schemes.
// This encoder preserves the order and the number of input points, but the
// mapping between point ids and attribute values may be different for the
// decoded point cloud.
class PointCloudSequentialEncoder : public PointCloudEncoder {
 public:
  uint8_t GetEncodingMethod() const override {
    return POINT_CLOUD_SEQUENTIAL_ENCODING;
  }

 protected:
  Status EncodeGeometryData() override;
  bool GenerateAttributesEncoder(int32_t att_id) override;
  void ComputeNumberOfEncodedPoints() override;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_POINT_CLOUD_POINT_CLOUD_SEQUENTIAL_ENCODER_H_
