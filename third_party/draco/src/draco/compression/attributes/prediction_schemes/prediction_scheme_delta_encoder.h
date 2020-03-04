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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_DELTA_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_DELTA_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder.h"

namespace draco {

// Basic prediction scheme based on computing backward differences between
// stored attribute values (also known as delta-coding). Usually works better
// than the reference point prediction scheme, because nearby values are often
// encoded next to each other.
template <typename DataTypeT, class TransformT>
class PredictionSchemeDeltaEncoder
    : public PredictionSchemeEncoder<DataTypeT, TransformT> {
 public:
  using CorrType =
      typename PredictionSchemeEncoder<DataTypeT, TransformT>::CorrType;
  // Initialized the prediction scheme.
  explicit PredictionSchemeDeltaEncoder(const PointAttribute *attribute)
      : PredictionSchemeEncoder<DataTypeT, TransformT>(attribute) {}
  PredictionSchemeDeltaEncoder(const PointAttribute *attribute,
                               const TransformT &transform)
      : PredictionSchemeEncoder<DataTypeT, TransformT>(attribute, transform) {}

  bool ComputeCorrectionValues(
      const DataTypeT *in_data, CorrType *out_corr, int size,
      int num_components, const PointIndex *entry_to_point_id_map) override;
  PredictionSchemeMethod GetPredictionMethod() const override {
    return PREDICTION_DIFFERENCE;
  }
  bool IsInitialized() const override { return true; }
};

template <typename DataTypeT, class TransformT>
bool PredictionSchemeDeltaEncoder<
    DataTypeT, TransformT>::ComputeCorrectionValues(const DataTypeT *in_data,
                                                    CorrType *out_corr,
                                                    int size,
                                                    int num_components,
                                                    const PointIndex *) {
  this->transform().Init(in_data, size, num_components);
  // Encode data from the back using D(i) = D(i) - D(i - 1).
  for (int i = size - num_components; i > 0; i -= num_components) {
    this->transform().ComputeCorrection(
        in_data + i, in_data + i - num_components, out_corr + i);
  }
  // Encode correction for the first element.
  std::unique_ptr<DataTypeT[]> zero_vals(new DataTypeT[num_components]());
  this->transform().ComputeCorrection(in_data, zero_vals.get(), out_corr);
  return true;
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_DELTA_ENCODER_H_
