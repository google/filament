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
#include "draco/compression/attributes/attributes_encoder.h"

#include "draco/core/varint_encoding.h"

namespace draco {

AttributesEncoder::AttributesEncoder()
    : point_cloud_encoder_(nullptr), point_cloud_(nullptr) {}

AttributesEncoder::AttributesEncoder(int att_id) : AttributesEncoder() {
  AddAttributeId(att_id);
}

bool AttributesEncoder::Init(PointCloudEncoder *encoder, const PointCloud *pc) {
  point_cloud_encoder_ = encoder;
  point_cloud_ = pc;
  return true;
}

bool AttributesEncoder::EncodeAttributesEncoderData(EncoderBuffer *out_buffer) {
  // Encode data about all attributes.
  EncodeVarint(num_attributes(), out_buffer);
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    const int32_t att_id = point_attribute_ids_[i];
    const PointAttribute *const pa = point_cloud_->attribute(att_id);
    out_buffer->Encode(static_cast<uint8_t>(pa->attribute_type()));
    out_buffer->Encode(static_cast<uint8_t>(pa->data_type()));
    out_buffer->Encode(static_cast<uint8_t>(pa->num_components()));
    out_buffer->Encode(static_cast<uint8_t>(pa->normalized()));
    EncodeVarint(pa->unique_id(), out_buffer);
  }
  return true;
}

}  // namespace draco
