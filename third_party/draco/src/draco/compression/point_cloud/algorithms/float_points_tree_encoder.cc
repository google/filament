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
#include "draco/compression/point_cloud/algorithms/float_points_tree_encoder.h"

#include <algorithm>
#include <cmath>

#include "draco/compression/point_cloud/algorithms/dynamic_integer_points_kd_tree_encoder.h"
#include "draco/core/math_utils.h"

namespace draco {

const uint32_t FloatPointsTreeEncoder::version_ = 3;

FloatPointsTreeEncoder::FloatPointsTreeEncoder(
    PointCloudCompressionMethod method)
    : method_(method), num_points_(0), compression_level_(6) {
  qinfo_.quantization_bits = 16;
  qinfo_.range = 0;
}

FloatPointsTreeEncoder::FloatPointsTreeEncoder(
    PointCloudCompressionMethod method, uint32_t quantization_bits,
    uint32_t compression_level)
    : method_(method), num_points_(0), compression_level_(compression_level) {
  DRACO_DCHECK_LE(compression_level_, 6);
  qinfo_.quantization_bits = quantization_bits;
  qinfo_.range = 0;
}

bool FloatPointsTreeEncoder::EncodePointCloudKdTreeInternal(
    std::vector<Point3ui> *qpoints) {
  DRACO_DCHECK_LE(compression_level_, 6);
  switch (compression_level_) {
    case 0: {
      DynamicIntegerPointsKdTreeEncoder<0> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    case 1: {
      DynamicIntegerPointsKdTreeEncoder<1> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    case 2: {
      DynamicIntegerPointsKdTreeEncoder<2> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    case 3: {
      DynamicIntegerPointsKdTreeEncoder<3> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    case 4: {
      DynamicIntegerPointsKdTreeEncoder<4> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    case 5: {
      DynamicIntegerPointsKdTreeEncoder<5> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
    default: {
      DynamicIntegerPointsKdTreeEncoder<6> qpoints_encoder(3);
      qpoints_encoder.EncodePoints(qpoints->begin(), qpoints->end(),
                                   qinfo_.quantization_bits + 1, &buffer_);
      break;
    }
  }

  return true;
}

}  // namespace draco
