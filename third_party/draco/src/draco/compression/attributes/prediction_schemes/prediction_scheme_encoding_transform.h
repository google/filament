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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODING_TRANSFORM_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODING_TRANSFORM_H_

#include "draco/compression/config/compression_shared.h"
#include "draco/core/encoder_buffer.h"

namespace draco {

// PredictionSchemeEncodingTransform is used to transform predicted values into
// correction values.
// CorrTypeT is the data type used for storing corrected values. It allows
// transforms to store corrections into a different type or format compared to
// the predicted data.
template <typename DataTypeT, typename CorrTypeT>
class PredictionSchemeEncodingTransform {
 public:
  typedef CorrTypeT CorrType;
  PredictionSchemeEncodingTransform() : num_components_(0) {}

  PredictionSchemeTransformType GetType() const {
    return PREDICTION_TRANSFORM_DELTA;
  }

  // Performs any custom initialization of the transform for the encoder.
  // |size| = total number of values in |orig_data| (i.e., number of entries *
  // number of components).
  void Init(const DataTypeT * /* orig_data */, int /* size */,
            int num_components) {
    num_components_ = num_components;
  }

  // Computes the corrections based on the input original values and the
  // predicted values. The correction is always computed for all components
  // of the input element. |val_id| is the id of the input value
  // (i.e., element_id * num_components). The default implementation is equal to
  // std::minus.
  inline void ComputeCorrection(const DataTypeT *original_vals,
                                const DataTypeT *predicted_vals,
                                CorrTypeT *out_corr_vals) {
    static_assert(std::is_same<DataTypeT, CorrTypeT>::value,
                  "For the default prediction transform, correction and input "
                  "data must be of the same type.");
    for (int i = 0; i < num_components_; ++i) {
      out_corr_vals[i] = original_vals[i] - predicted_vals[i];
    }
  }

  // Encode any transform specific data.
  bool EncodeTransformData(EncoderBuffer * /* buffer */) { return true; }

  // Should return true if all corrected values are guaranteed to be positive.
  bool AreCorrectionsPositive() const { return false; }

 protected:
  int num_components() const { return num_components_; }

 private:
  int num_components_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODING_TRANSFORM_H_
