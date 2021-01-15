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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_KD_TREE_ATTRIBUTES_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_KD_TREE_ATTRIBUTES_ENCODER_H_

#include "draco/attributes/attribute_quantization_transform.h"
#include "draco/compression/attributes/attributes_encoder.h"
#include "draco/compression/config/compression_shared.h"

namespace draco {

// Encodes all attributes of a given PointCloud using one of the available
// Kd-tree compression methods.
// See compression/point_cloud/point_cloud_kd_tree_encoder.h for more details.
class KdTreeAttributesEncoder : public AttributesEncoder {
 public:
  KdTreeAttributesEncoder();
  explicit KdTreeAttributesEncoder(int att_id);

  uint8_t GetUniqueId() const override { return KD_TREE_ATTRIBUTE_ENCODER; }

 protected:
  bool TransformAttributesToPortableFormat() override;
  bool EncodePortableAttributes(EncoderBuffer *out_buffer) override;
  bool EncodeDataNeededByPortableTransforms(EncoderBuffer *out_buffer) override;

 private:
  std::vector<AttributeQuantizationTransform>
      attribute_quantization_transforms_;
  // Min signed values are used to transform signed integers into unsigned ones
  // (by subtracting the min signed value for each component).
  std::vector<int32_t> min_signed_values_;
  std::vector<std::unique_ptr<PointAttribute>> quantized_portable_attributes_;
  int num_components_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_KD_TREE_ATTRIBUTES_ENCODER_H_
