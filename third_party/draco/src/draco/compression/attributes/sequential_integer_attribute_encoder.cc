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
#include "draco/compression/attributes/sequential_integer_attribute_encoder.h"

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder_factory.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_wrap_encoding_transform.h"
#include "draco/compression/entropy/symbol_encoding.h"
#include "draco/core/bit_utils.h"

namespace draco {

SequentialIntegerAttributeEncoder::SequentialIntegerAttributeEncoder() {}

bool SequentialIntegerAttributeEncoder::Init(PointCloudEncoder *encoder,
                                             int attribute_id) {
  if (!SequentialAttributeEncoder::Init(encoder, attribute_id)) {
    return false;
  }
  if (GetUniqueId() == SEQUENTIAL_ATTRIBUTE_ENCODER_INTEGER) {
    // When encoding integers, this encoder currently works only for integer
    // attributes up to 32 bits.
    switch (attribute()->data_type()) {
      case DT_INT8:
      case DT_UINT8:
      case DT_INT16:
      case DT_UINT16:
      case DT_INT32:
      case DT_UINT32:
        break;
      default:
        return false;
    }
  }
  // Init prediction scheme.
  const PredictionSchemeMethod prediction_scheme_method =
      GetPredictionMethodFromOptions(attribute_id, *encoder->options());

  prediction_scheme_ = CreateIntPredictionScheme(prediction_scheme_method);

  if (prediction_scheme_ && !InitPredictionScheme(prediction_scheme_.get())) {
    prediction_scheme_ = nullptr;
  }

  return true;
}

bool SequentialIntegerAttributeEncoder::TransformAttributeToPortableFormat(
    const std::vector<PointIndex> &point_ids) {
  if (encoder()) {
    if (!PrepareValues(point_ids, encoder()->point_cloud()->num_points())) {
      return false;
    }
  } else {
    if (!PrepareValues(point_ids, 0)) {
      return false;
    }
  }

  // Update point to attribute mapping with the portable attribute if the
  // attribute is a parent attribute (for now, we can skip it otherwise).
  if (is_parent_encoder()) {
    // First create map between original attribute value indices and new ones
    // (determined by the encoding order).
    const PointAttribute *const orig_att = attribute();
    PointAttribute *const portable_att = portable_attribute();
    IndexTypeVector<AttributeValueIndex, AttributeValueIndex>
        value_to_value_map(orig_att->size());
    for (int i = 0; i < point_ids.size(); ++i) {
      value_to_value_map[orig_att->mapped_index(point_ids[i])] =
          AttributeValueIndex(i);
    }
    if (portable_att->is_mapping_identity()) {
      portable_att->SetExplicitMapping(encoder()->point_cloud()->num_points());
    }
    // Go over all points of the original attribute and update the mapping in
    // the portable attribute.
    for (PointIndex i(0); i < encoder()->point_cloud()->num_points(); ++i) {
      portable_att->SetPointMapEntry(
          i, value_to_value_map[orig_att->mapped_index(i)]);
    }
  }
  return true;
}

std::unique_ptr<PredictionSchemeTypedEncoderInterface<int32_t>>
SequentialIntegerAttributeEncoder::CreateIntPredictionScheme(
    PredictionSchemeMethod method) {
  return CreatePredictionSchemeForEncoder<
      int32_t, PredictionSchemeWrapEncodingTransform<int32_t>>(
      method, attribute_id(), encoder());
}

bool SequentialIntegerAttributeEncoder::EncodeValues(
    const std::vector<PointIndex> &point_ids, EncoderBuffer *out_buffer) {
  // Initialize general quantization data.
  const PointAttribute *const attrib = attribute();
  if (attrib->size() == 0) {
    return true;
  }

  int8_t prediction_scheme_method = PREDICTION_NONE;
  if (prediction_scheme_) {
    if (!SetPredictionSchemeParentAttributes(prediction_scheme_.get())) {
      return false;
    }
    prediction_scheme_method =
        static_cast<int8_t>(prediction_scheme_->GetPredictionMethod());
  }
  out_buffer->Encode(prediction_scheme_method);
  if (prediction_scheme_) {
    out_buffer->Encode(
        static_cast<int8_t>(prediction_scheme_->GetTransformType()));
  }

  const int num_components = portable_attribute()->num_components();
  const int num_values =
      static_cast<int>(num_components * portable_attribute()->size());
  const int32_t *const portable_attribute_data = GetPortableAttributeData();

  // We need to keep the portable data intact, but several encoding steps can
  // result in changes of this data, e.g., by applying prediction schemes that
  // change the data in place. To preserve the portable data we store and
  // process all encoded data in a separate array.
  std::vector<int32_t> encoded_data(num_values);

  // All integer values are initialized. Process them using the prediction
  // scheme if we have one.
  if (prediction_scheme_) {
    prediction_scheme_->ComputeCorrectionValues(
        portable_attribute_data, &encoded_data[0], num_values, num_components,
        point_ids.data());
  }

  if (prediction_scheme_ == nullptr ||
      !prediction_scheme_->AreCorrectionsPositive()) {
    const int32_t *const input =
        prediction_scheme_ ? encoded_data.data() : portable_attribute_data;
    ConvertSignedIntsToSymbols(input, num_values,
                               reinterpret_cast<uint32_t *>(&encoded_data[0]));
  }

  if (encoder() == nullptr || encoder()->options()->GetGlobalBool(
                                  "use_built_in_attribute_compression", true)) {
    out_buffer->Encode(static_cast<uint8_t>(1));
    Options symbol_encoding_options;
    if (encoder() != nullptr) {
      SetSymbolEncodingCompressionLevel(&symbol_encoding_options,
                                        10 - encoder()->options()->GetSpeed());
    }
    if (!EncodeSymbols(reinterpret_cast<uint32_t *>(encoded_data.data()),
                       static_cast<int>(point_ids.size()) * num_components,
                       num_components, &symbol_encoding_options, out_buffer)) {
      return false;
    }
  } else {
    // No compression. Just store the raw integer values, using the number of
    // bytes as needed.

    // To compute the maximum bit-length, first OR all values.
    uint32_t masked_value = 0;
    for (uint32_t i = 0; i < static_cast<uint32_t>(num_values); ++i) {
      masked_value |= encoded_data[i];
    }
    // Compute the msb of the ORed value.
    int value_msb_pos = 0;
    if (masked_value != 0) {
      value_msb_pos = MostSignificantBit(masked_value);
    }
    const int num_bytes = 1 + value_msb_pos / 8;

    out_buffer->Encode(static_cast<uint8_t>(0));
    out_buffer->Encode(static_cast<uint8_t>(num_bytes));

    if (num_bytes == DataTypeLength(DT_INT32)) {
      out_buffer->Encode(encoded_data.data(), sizeof(int32_t) * num_values);
    } else {
      for (uint32_t i = 0; i < static_cast<uint32_t>(num_values); ++i) {
        out_buffer->Encode(encoded_data.data() + i, num_bytes);
      }
    }
  }
  if (prediction_scheme_) {
    prediction_scheme_->EncodePredictionData(out_buffer);
  }
  return true;
}

bool SequentialIntegerAttributeEncoder::PrepareValues(
    const std::vector<PointIndex> &point_ids, int num_points) {
  // Convert all values to int32_t format.
  const PointAttribute *const attrib = attribute();
  const int num_components = attrib->num_components();
  const int num_entries = static_cast<int>(point_ids.size());
  PreparePortableAttribute(num_entries, num_components, num_points);
  int32_t dst_index = 0;
  int32_t *const portable_attribute_data = GetPortableAttributeData();
  for (PointIndex pi : point_ids) {
    const AttributeValueIndex att_id = attrib->mapped_index(pi);
    if (!attrib->ConvertValue<int32_t>(att_id,
                                       portable_attribute_data + dst_index)) {
      return false;
    }
    dst_index += num_components;
  }
  return true;
}

void SequentialIntegerAttributeEncoder::PreparePortableAttribute(
    int num_entries, int num_components, int num_points) {
  GeometryAttribute va;
  va.Init(attribute()->attribute_type(), nullptr, num_components, DT_INT32,
          false, num_components * DataTypeLength(DT_INT32), 0);
  std::unique_ptr<PointAttribute> port_att(new PointAttribute(va));
  port_att->Reset(num_entries);
  SetPortableAttribute(std::move(port_att));
  if (num_points) {
    portable_attribute()->SetExplicitMapping(num_points);
  }
}

}  // namespace draco
