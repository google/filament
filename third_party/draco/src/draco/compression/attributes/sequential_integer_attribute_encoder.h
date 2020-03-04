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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_INTEGER_ATTRIBUTE_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_INTEGER_ATTRIBUTE_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder.h"
#include "draco/compression/attributes/sequential_attribute_encoder.h"

namespace draco {

// Attribute encoder designed for lossless encoding of integer attributes. The
// attribute values can be pre-processed by a prediction scheme and compressed
// with a built-in entropy coder.
class SequentialIntegerAttributeEncoder : public SequentialAttributeEncoder {
 public:
  SequentialIntegerAttributeEncoder();
  uint8_t GetUniqueId() const override {
    return SEQUENTIAL_ATTRIBUTE_ENCODER_INTEGER;
  }

  bool Init(PointCloudEncoder *encoder, int attribute_id) override;
  bool TransformAttributeToPortableFormat(
      const std::vector<PointIndex> &point_ids) override;

 protected:
  bool EncodeValues(const std::vector<PointIndex> &point_ids,
                    EncoderBuffer *out_buffer) override;

  // Returns a prediction scheme that should be used for encoding of the
  // integer values.
  virtual std::unique_ptr<PredictionSchemeTypedEncoderInterface<int32_t>>
  CreateIntPredictionScheme(PredictionSchemeMethod method);

  // Prepares the integer values that are going to be encoded.
  virtual bool PrepareValues(const std::vector<PointIndex> &point_ids,
                             int num_points);

  void PreparePortableAttribute(int num_entries, int num_components,
                                int num_points);

  int32_t *GetPortableAttributeData() {
    return reinterpret_cast<int32_t *>(
        portable_attribute()->GetAddress(AttributeValueIndex(0)));
  }

 private:
  // Optional prediction scheme can be used to modify the integer values in
  // order to make them easier to compress.
  std::unique_ptr<PredictionSchemeTypedEncoderInterface<int32_t>>
      prediction_scheme_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_SEQUENTIAL_INTEGER_ATTRIBUTE_ENCODER_H_
