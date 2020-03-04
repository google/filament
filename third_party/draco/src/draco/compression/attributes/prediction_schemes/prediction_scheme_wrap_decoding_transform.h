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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_DECODING_TRANSFORM_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_DECODING_TRANSFORM_H_

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_wrap_transform_base.h"
#include "draco/core/decoder_buffer.h"

namespace draco {

// PredictionSchemeWrapDecodingTransform unwraps values encoded with the
// PredictionSchemeWrapEncodingTransform.
// See prediction_scheme_wrap_transform_base.h for more details about the
// method.
template <typename DataTypeT, typename CorrTypeT = DataTypeT>
class PredictionSchemeWrapDecodingTransform
    : public PredictionSchemeWrapTransformBase<DataTypeT> {
 public:
  typedef CorrTypeT CorrType;
  PredictionSchemeWrapDecodingTransform() {}

  // Computes the original value from the input predicted value and the decoded
  // corrections. Values out of the bounds of the input values are unwrapped.
  inline void ComputeOriginalValue(const DataTypeT *predicted_vals,
                                   const CorrTypeT *corr_vals,
                                   DataTypeT *out_original_vals) const {
    predicted_vals = this->ClampPredictedValue(predicted_vals);
    for (int i = 0; i < this->num_components(); ++i) {
      out_original_vals[i] = predicted_vals[i] + corr_vals[i];
      if (out_original_vals[i] > this->max_value()) {
        out_original_vals[i] -= this->max_dif();
      } else if (out_original_vals[i] < this->min_value()) {
        out_original_vals[i] += this->max_dif();
      }
    }
  }

  bool DecodeTransformData(DecoderBuffer *buffer) {
    DataTypeT min_value, max_value;
    if (!buffer->Decode(&min_value)) {
      return false;
    }
    if (!buffer->Decode(&max_value)) {
      return false;
    }
    if (min_value > max_value) {
      return false;
    }
    this->set_min_value(min_value);
    this->set_max_value(max_value);
    if (!this->InitCorrectionBounds()) {
      return false;
    }
    return true;
  }
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_DECODING_TRANSFORM_H_
