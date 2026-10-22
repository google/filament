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
#include "draco/compression/attributes/sequential_attribute_encoders_controller.h"
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
#include "draco/compression/attributes/sequential_normal_attribute_encoder.h"
#endif
#include "draco/compression/attributes/sequential_quantization_attribute_encoder.h"
#include "draco/compression/point_cloud/point_cloud_encoder.h"

namespace draco {

SequentialAttributeEncodersController::SequentialAttributeEncodersController(
    std::unique_ptr<PointsSequencer> sequencer)
    : sequencer_(std::move(sequencer)) {}

SequentialAttributeEncodersController::SequentialAttributeEncodersController(
    std::unique_ptr<PointsSequencer> sequencer, int point_attrib_id)
    : AttributesEncoder(point_attrib_id), sequencer_(std::move(sequencer)) {}

bool SequentialAttributeEncodersController::Init(PointCloudEncoder *encoder,
                                                 const PointCloud *pc) {
  if (!AttributesEncoder::Init(encoder, pc)) {
    return false;
  }
  if (!CreateSequentialEncoders()) {
    return false;
  }
  // Initialize all value encoders.
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    const int32_t att_id = GetAttributeId(i);
    if (!sequential_encoders_[i]->Init(encoder, att_id)) {
      return false;
    }
  }
  return true;
}

bool SequentialAttributeEncodersController::EncodeAttributesEncoderData(
    EncoderBuffer *out_buffer) {
  if (!AttributesEncoder::EncodeAttributesEncoderData(out_buffer)) {
    return false;
  }
  // Encode a unique id of every sequential encoder.
  for (uint32_t i = 0; i < sequential_encoders_.size(); ++i) {
    out_buffer->Encode(sequential_encoders_[i]->GetUniqueId());
  }
  return true;
}

bool SequentialAttributeEncodersController::EncodeAttributes(
    EncoderBuffer *buffer) {
  if (!sequencer_ || !sequencer_->GenerateSequence(&point_ids_)) {
    return false;
  }
  return AttributesEncoder::EncodeAttributes(buffer);
}

bool SequentialAttributeEncodersController::
    TransformAttributesToPortableFormat() {
  for (uint32_t i = 0; i < sequential_encoders_.size(); ++i) {
    if (!sequential_encoders_[i]->TransformAttributeToPortableFormat(
            point_ids_)) {
      return false;
    }
  }
  return true;
}

bool SequentialAttributeEncodersController::EncodePortableAttributes(
    EncoderBuffer *out_buffer) {
  for (uint32_t i = 0; i < sequential_encoders_.size(); ++i) {
    if (!sequential_encoders_[i]->EncodePortableAttribute(point_ids_,
                                                          out_buffer)) {
      return false;
    }
  }
  return true;
}

bool SequentialAttributeEncodersController::
    EncodeDataNeededByPortableTransforms(EncoderBuffer *out_buffer) {
  for (uint32_t i = 0; i < sequential_encoders_.size(); ++i) {
    if (!sequential_encoders_[i]->EncodeDataNeededByPortableTransform(
            out_buffer)) {
      return false;
    }
  }
  return true;
}

bool SequentialAttributeEncodersController::CreateSequentialEncoders() {
  sequential_encoders_.resize(num_attributes());
  for (uint32_t i = 0; i < num_attributes(); ++i) {
    sequential_encoders_[i] = CreateSequentialEncoder(i);
    if (sequential_encoders_[i] == nullptr) {
      return false;
    }
    if (i < sequential_encoder_marked_as_parent_.size()) {
      if (sequential_encoder_marked_as_parent_[i]) {
        sequential_encoders_[i]->MarkParentAttribute();
      }
    }
  }
  return true;
}

std::unique_ptr<SequentialAttributeEncoder>
SequentialAttributeEncodersController::CreateSequentialEncoder(int i) {
  const int32_t att_id = GetAttributeId(i);
  const PointAttribute *const att = encoder()->point_cloud()->attribute(att_id);

  switch (att->data_type()) {
    case DT_UINT8:
    case DT_INT8:
    case DT_UINT16:
    case DT_INT16:
    case DT_UINT32:
    case DT_INT32:
      return std::unique_ptr<SequentialAttributeEncoder>(
          new SequentialIntegerAttributeEncoder());
    case DT_FLOAT32:
      if (encoder()->options()->GetAttributeInt(att_id, "quantization_bits",
                                                -1) > 0) {
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
        if (att->attribute_type() == GeometryAttribute::NORMAL) {
          // We currently only support normals with float coordinates
          // and must be quantized.
          return std::unique_ptr<SequentialAttributeEncoder>(
              new SequentialNormalAttributeEncoder());
        } else {
#endif
          return std::unique_ptr<SequentialAttributeEncoder>(
              new SequentialQuantizationAttributeEncoder());
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
        }
#endif
      }
      break;
    default:
      break;
  }
  // Return the default attribute encoder.
  return std::unique_ptr<SequentialAttributeEncoder>(
      new SequentialAttributeEncoder());
}

}  // namespace draco
