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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_ENCODING_TRANSFORM_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_ENCODING_TRANSFORM_H_

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_wrap_transform_base.h"
#include "draco/core/encoder_buffer.h"

namespace draco {

// PredictionSchemeWrapEncodingTransform wraps input values using the wrapping
// scheme described in: prediction_scheme_wrap_transform_base.h .
template <typename DataTypeT, typename CorrTypeT = DataTypeT>
class PredictionSchemeWrapEncodingTransform
    : public PredictionSchemeWrapTransformBase<DataTypeT> {
 public:
  typedef CorrTypeT CorrType;
  PredictionSchemeWrapEncodingTransform() {}

  void Init(const DataTypeT *orig_data, int size, int num_components) {
    PredictionSchemeWrapTransformBase<DataTypeT>::Init(num_components);
    // Go over the original values and compute the bounds.
    if (size == 0) {
      return;
    }
    DataTypeT min_value = orig_data[0];
    DataTypeT max_value = min_value;
    for (int i = 1; i < size; ++i) {
      if (orig_data[i] < min_value) {
        min_value = orig_data[i];
      } else if (orig_data[i] > max_value) {
        max_value = orig_data[i];
      }
    }
    this->set_min_value(min_value);
    this->set_max_value(max_value);
    this->InitCorrectionBounds();
  }

  // Computes the corrections based on the input original value and the
  // predicted value. Out of bound correction values are wrapped around the max
  // range of input values.
  inline void ComputeCorrection(const DataTypeT *original_vals,
                                const DataTypeT *predicted_vals,
                                CorrTypeT *out_corr_vals) const {
    for (int i = 0; i < this->num_components(); ++i) {
      predicted_vals = this->ClampPredictedValue(predicted_vals);
      out_corr_vals[i] = original_vals[i] - predicted_vals[i];
      // Wrap around if needed.
      DataTypeT &corr_val = out_corr_vals[i];
      if (corr_val < this->min_correction()) {
        corr_val += this->max_dif();
      } else if (corr_val > this->max_correction()) {
        corr_val -= this->max_dif();
      }
    }
  }

  bool EncodeTransformData(EncoderBuffer *buffer) {
    // Store the input value range as it is needed by the decoder.
    buffer->Encode(this->min_value());
    buffer->Encode(this->max_value());
    return true;
  }
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_WRAP_ENCODING_TRANSFORM_H_
