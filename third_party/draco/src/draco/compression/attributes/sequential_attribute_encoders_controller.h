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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODERS_CONTROLLER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODERS_CONTROLLER_H_

#include "draco/compression/attributes/attributes_encoder.h"
#include "draco/compression/attributes/points_sequencer.h"
#include "draco/compression/attributes/sequential_attribute_encoder.h"

namespace draco {

// A basic implementation of an attribute encoder that can be used to encode
// an arbitrary set of attributes. The encoder creates a sequential attribute
// encoder for each encoded attribute (see sequential_attribute_encoder.h) and
// then it encodes all attribute values in an order defined by a point sequence
// generated in the GeneratePointSequence() method. The default implementation
// generates a linear sequence of all points, but derived classes can generate
// any custom sequence.
class SequentialAttributeEncodersController : public AttributesEncoder {
 public:
  explicit SequentialAttributeEncodersController(
      std::unique_ptr<PointsSequencer> sequencer);
  SequentialAttributeEncodersController(
      std::unique_ptr<PointsSequencer> sequencer, int point_attrib_id);

  bool Init(PointCloudEncoder *encoder, const PointCloud *pc) override;
  bool EncodeAttributesEncoderData(EncoderBuffer *out_buffer) override;
  bool EncodeAttributes(EncoderBuffer *buffer) override;
  uint8_t GetUniqueId() const override { return BASIC_ATTRIBUTE_ENCODER; }

  int NumParentAttributes(int32_t point_attribute_id) const override {
    const int32_t loc_id = GetLocalIdForPointAttribute(point_attribute_id);
    if (loc_id < 0) {
      return 0;
    }
    return sequential_encoders_[loc_id]->NumParentAttributes();
  }

  int GetParentAttributeId(int32_t point_attribute_id,
                           int32_t parent_i) const override {
    const int32_t loc_id = GetLocalIdForPointAttribute(point_attribute_id);
    if (loc_id < 0) {
      return -1;
    }
    return sequential_encoders_[loc_id]->GetParentAttributeId(parent_i);
  }

  bool MarkParentAttribute(int32_t point_attribute_id) override {
    const int32_t loc_id = GetLocalIdForPointAttribute(point_attribute_id);
    if (loc_id < 0) {
      return false;
    }
    // Mark the attribute encoder as parent (even when if it is not created
    // yet).
    if (sequential_encoder_marked_as_parent_.size() <= loc_id) {
      sequential_encoder_marked_as_parent_.resize(loc_id + 1, false);
    }
    sequential_encoder_marked_as_parent_[loc_id] = true;

    if (sequential_encoders_.size() <= loc_id) {
      return true;  // Sequential encoders not generated yet.
    }
    sequential_encoders_[loc_id]->MarkParentAttribute();
    return true;
  }

  const PointAttribute *GetPortableAttribute(
      int32_t point_attribute_id) override {
    const int32_t loc_id = GetLocalIdForPointAttribute(point_attribute_id);
    if (loc_id < 0) {
      return nullptr;
    }
    return sequential_encoders_[loc_id]->GetPortableAttribute();
  }

 protected:
  bool TransformAttributesToPortableFormat() override;
  bool EncodePortableAttributes(EncoderBuffer *out_buffer) override;
  bool EncodeDataNeededByPortableTransforms(EncoderBuffer *out_buffer) override;

  // Creates all sequential encoders (one for each attribute associated with the
  // encoder).
  virtual bool CreateSequentialEncoders();

  // Create a sequential encoder for a given attribute based on the attribute
  // type
  // and the provided encoder options.
  virtual std::unique_ptr<SequentialAttributeEncoder> CreateSequentialEncoder(
      int i);

 private:
  std::vector<std::unique_ptr<SequentialAttributeEncoder>> sequential_encoders_;

  // Flag for each sequential attribute encoder indicating whether it was marked
  // as parent attribute or not.
  std::vector<bool> sequential_encoder_marked_as_parent_;
  std::vector<PointIndex> point_ids_;
  std::unique_ptr<PointsSequencer> sequencer_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_ATTRIBUTE_ENCODERS_CONTROLLER_H_
