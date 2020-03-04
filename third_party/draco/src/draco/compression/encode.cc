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
#include "draco/compression/encode.h"

#include "draco/compression/expert_encode.h"

namespace draco {

Encoder::Encoder() {}

Status Encoder::EncodePointCloudToBuffer(const PointCloud &pc,
                                         EncoderBuffer *out_buffer) {
  ExpertEncoder encoder(pc);
  encoder.Reset(CreateExpertEncoderOptions(pc));
  return encoder.EncodeToBuffer(out_buffer);
}

Status Encoder::EncodeMeshToBuffer(const Mesh &m, EncoderBuffer *out_buffer) {
  ExpertEncoder encoder(m);
  encoder.Reset(CreateExpertEncoderOptions(m));
  DRACO_RETURN_IF_ERROR(encoder.EncodeToBuffer(out_buffer));
  set_num_encoded_points(encoder.num_encoded_points());
  set_num_encoded_faces(encoder.num_encoded_faces());
  return OkStatus();
}

EncoderOptions Encoder::CreateExpertEncoderOptions(const PointCloud &pc) const {
  EncoderOptions ret_options = EncoderOptions::CreateEmptyOptions();
  ret_options.SetGlobalOptions(options().GetGlobalOptions());
  ret_options.SetFeatureOptions(options().GetFeaturelOptions());
  // Convert type-based attribute options to specific attributes in the provided
  // point cloud.
  for (int i = 0; i < pc.num_attributes(); ++i) {
    const Options *att_options =
        options().FindAttributeOptions(pc.attribute(i)->attribute_type());
    if (att_options) {
      ret_options.SetAttributeOptions(i, *att_options);
    }
  }
  return ret_options;
}

void Encoder::Reset(
    const EncoderOptionsBase<GeometryAttribute::Type> &options) {
  Base::Reset(options);
}

void Encoder::Reset() { Base::Reset(); }

void Encoder::SetSpeedOptions(int encoding_speed, int decoding_speed) {
  Base::SetSpeedOptions(encoding_speed, decoding_speed);
}

void Encoder::SetAttributeQuantization(GeometryAttribute::Type type,
                                       int quantization_bits) {
  options().SetAttributeInt(type, "quantization_bits", quantization_bits);
}

void Encoder::SetAttributeExplicitQuantization(GeometryAttribute::Type type,
                                               int quantization_bits,
                                               int num_dims,
                                               const float *origin,
                                               float range) {
  options().SetAttributeInt(type, "quantization_bits", quantization_bits);
  options().SetAttributeVector(type, "quantization_origin", num_dims, origin);
  options().SetAttributeFloat(type, "quantization_range", range);
}

void Encoder::SetEncodingMethod(int encoding_method) {
  Base::SetEncodingMethod(encoding_method);
}

Status Encoder::SetAttributePredictionScheme(GeometryAttribute::Type type,
                                             int prediction_scheme_method) {
  Status status = CheckPredictionScheme(type, prediction_scheme_method);
  if (!status.ok()) {
    return status;
  }
  options().SetAttributeInt(type, "prediction_scheme",
                            prediction_scheme_method);
  return status;
}

}  // namespace draco
