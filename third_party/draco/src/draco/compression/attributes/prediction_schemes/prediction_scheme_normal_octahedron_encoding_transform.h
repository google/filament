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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_NORMAL_OCTAHEDRON_ENCODING_TRANSFORM_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_NORMAL_OCTAHEDRON_ENCODING_TRANSFORM_H_

#include <cmath>

#include "draco/compression/attributes/normal_compression_utils.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_normal_octahedron_transform_base.h"
#include "draco/core/encoder_buffer.h"
#include "draco/core/macros.h"
#include "draco/core/vector_d.h"

namespace draco {

// The transform works on octahedral coordinates for normals. The square is
// subdivided into four inner triangles (diamond) and four outer triangles. The
// inner triangles are associated with the upper part of the octahedron and the
// outer triangles are associated with the lower part.
// Given a prediction value P and the actual value Q that should be encoded,
// this transform first checks if P is outside the diamond. If so, the outer
// triangles are flipped towards the inside and vice versa. The actual
// correction value is then based on the mapped P and Q values. This tends to
// result in shorter correction vectors.
// This is possible since the P value is also known by the decoder, see also
// ComputeCorrection and ComputeOriginalValue functions.
// Note that the tile is not periodic, which implies that the outer edges can
// not be identified, which requires us to use an odd number of values on each
// axis.
// DataTypeT is expected to be some integral type.
//
template <typename DataTypeT>
class PredictionSchemeNormalOctahedronEncodingTransform
    : public PredictionSchemeNormalOctahedronTransformBase<DataTypeT> {
 public:
  typedef PredictionSchemeNormalOctahedronTransformBase<DataTypeT> Base;
  typedef VectorD<DataTypeT, 2> Point2;
  typedef DataTypeT CorrType;
  typedef DataTypeT DataType;

  // We expect the mod value to be of the form 2^b-1.
  explicit PredictionSchemeNormalOctahedronEncodingTransform(
      DataType max_quantized_value)
      : Base(max_quantized_value) {}

  void Init(const DataTypeT *orig_data, int size, int num_components) {}

  bool EncodeTransformData(EncoderBuffer *buffer) {
    buffer->Encode(this->max_quantized_value());
    return true;
  }

  inline void ComputeCorrection(const DataType *orig_vals,
                                const DataType *pred_vals,
                                CorrType *out_corr_vals) const {
    DRACO_DCHECK_LE(pred_vals[0], this->center_value() * 2);
    DRACO_DCHECK_LE(pred_vals[1], this->center_value() * 2);
    DRACO_DCHECK_LE(orig_vals[0], this->center_value() * 2);
    DRACO_DCHECK_LE(orig_vals[1], this->center_value() * 2);
    DRACO_DCHECK_LE(0, pred_vals[0]);
    DRACO_DCHECK_LE(0, pred_vals[1]);
    DRACO_DCHECK_LE(0, orig_vals[0]);
    DRACO_DCHECK_LE(0, orig_vals[1]);

    const Point2 orig = Point2(orig_vals[0], orig_vals[1]);
    const Point2 pred = Point2(pred_vals[0], pred_vals[1]);
    const Point2 corr = ComputeCorrection(orig, pred);

    out_corr_vals[0] = corr[0];
    out_corr_vals[1] = corr[1];
  }

 private:
  Point2 ComputeCorrection(Point2 orig, Point2 pred) const {
    const Point2 t(this->center_value(), this->center_value());
    orig = orig - t;
    pred = pred - t;

    if (!this->IsInDiamond(pred[0], pred[1])) {
      this->InvertDiamond(&orig[0], &orig[1]);
      this->InvertDiamond(&pred[0], &pred[1]);
    }

    Point2 corr = orig - pred;
    corr[0] = this->MakePositive(corr[0]);
    corr[1] = this->MakePositive(corr[1]);
    return corr;
  }
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_NORMAL_OCTAHEDRON_ENCODING_TRANSFORM_H_
