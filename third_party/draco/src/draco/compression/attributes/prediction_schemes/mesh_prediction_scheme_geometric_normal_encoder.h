// Copyright 2017 The Draco Authors.
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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_GEOMETRIC_NORMAL_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_GEOMETRIC_NORMAL_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_predictor_area.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/compression/config/compression_shared.h"

namespace draco {

// Prediction scheme for normals based on the underlying geometry.
// At a smooth vertices normals are computed by weighting the normals of
// adjacent faces with the area of these faces. At seams, the same approach
// applies for seam corners.
template <typename DataTypeT, class TransformT, class MeshDataT>
class MeshPredictionSchemeGeometricNormalEncoder
    : public MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT> {
 public:
  using CorrType = typename MeshPredictionSchemeEncoder<DataTypeT, TransformT,
                                                        MeshDataT>::CorrType;
  MeshPredictionSchemeGeometricNormalEncoder(const PointAttribute *attribute,
                                             const TransformT &transform,
                                             const MeshDataT &mesh_data)
      : MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT>(
            attribute, transform, mesh_data),
        predictor_(mesh_data) {}

  bool ComputeCorrectionValues(
      const DataTypeT *in_data, CorrType *out_corr, int size,
      int num_components, const PointIndex *entry_to_point_id_map) override;

  bool EncodePredictionData(EncoderBuffer *buffer) override;

  PredictionSchemeMethod GetPredictionMethod() const override {
    return MESH_PREDICTION_GEOMETRIC_NORMAL;
  }

  bool IsInitialized() const override {
    if (!predictor_.IsInitialized()) {
      return false;
    }
    if (!this->mesh_data().IsInitialized()) {
      return false;
    }
    return true;
  }

  int GetNumParentAttributes() const override { return 1; }

  GeometryAttribute::Type GetParentAttributeType(int i) const override {
    DRACO_DCHECK_EQ(i, 0);
    (void)i;
    return GeometryAttribute::POSITION;
  }

  bool SetParentAttribute(const PointAttribute *att) override {
    if (att->attribute_type() != GeometryAttribute::POSITION) {
      return false;  // Invalid attribute type.
    }
    if (att->num_components() != 3) {
      return false;  // Currently works only for 3 component positions.
    }
    predictor_.SetPositionAttribute(*att);
    return true;
  }

 private:
  void SetQuantizationBits(int q) {
    DRACO_DCHECK_GE(q, 2);
    DRACO_DCHECK_LE(q, 30);
    octahedron_tool_box_.SetQuantizationBits(q);
  }
  MeshPredictionSchemeGeometricNormalPredictorArea<DataTypeT, TransformT,
                                                   MeshDataT>
      predictor_;

  OctahedronToolBox octahedron_tool_box_;
  RAnsBitEncoder flip_normal_bit_encoder_;
};

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeGeometricNormalEncoder<DataTypeT, TransformT,
                                                MeshDataT>::
    ComputeCorrectionValues(const DataTypeT *in_data, CorrType *out_corr,
                            int size, int num_components,
                            const PointIndex *entry_to_point_id_map) {
  this->SetQuantizationBits(this->transform().quantization_bits());
  predictor_.SetEntryToPointIdMap(entry_to_point_id_map);
  DRACO_DCHECK(this->IsInitialized());
  // Expecting in_data in octahedral coordinates, i.e., portable attribute.
  DRACO_DCHECK_EQ(num_components, 2);

  flip_normal_bit_encoder_.StartEncoding();

  const int corner_map_size =
      static_cast<int>(this->mesh_data().data_to_corner_map()->size());

  VectorD<int32_t, 3> pred_normal_3d;
  VectorD<int32_t, 2> pos_pred_normal_oct;
  VectorD<int32_t, 2> neg_pred_normal_oct;
  VectorD<int32_t, 2> pos_correction;
  VectorD<int32_t, 2> neg_correction;
  for (int data_id = 0; data_id < corner_map_size; ++data_id) {
    const CornerIndex corner_id =
        this->mesh_data().data_to_corner_map()->at(data_id);
    predictor_.ComputePredictedValue(corner_id, pred_normal_3d.data());

    // Compute predicted octahedral coordinates.
    octahedron_tool_box_.CanonicalizeIntegerVector(pred_normal_3d.data());
    DRACO_DCHECK_EQ(pred_normal_3d.AbsSum(),
                    octahedron_tool_box_.center_value());

    // Compute octahedral coordinates for both possible directions.
    octahedron_tool_box_.IntegerVectorToQuantizedOctahedralCoords(
        pred_normal_3d.data(), pos_pred_normal_oct.data(),
        pos_pred_normal_oct.data() + 1);
    pred_normal_3d = -pred_normal_3d;
    octahedron_tool_box_.IntegerVectorToQuantizedOctahedralCoords(
        pred_normal_3d.data(), neg_pred_normal_oct.data(),
        neg_pred_normal_oct.data() + 1);

    // Choose the one with the best correction value.
    const int data_offset = data_id * 2;
    this->transform().ComputeCorrection(in_data + data_offset,
                                        pos_pred_normal_oct.data(),
                                        pos_correction.data());
    this->transform().ComputeCorrection(in_data + data_offset,
                                        neg_pred_normal_oct.data(),
                                        neg_correction.data());
    pos_correction[0] = octahedron_tool_box_.ModMax(pos_correction[0]);
    pos_correction[1] = octahedron_tool_box_.ModMax(pos_correction[1]);
    neg_correction[0] = octahedron_tool_box_.ModMax(neg_correction[0]);
    neg_correction[1] = octahedron_tool_box_.ModMax(neg_correction[1]);
    if (pos_correction.AbsSum() < neg_correction.AbsSum()) {
      flip_normal_bit_encoder_.EncodeBit(false);
      (out_corr + data_offset)[0] =
          octahedron_tool_box_.MakePositive(pos_correction[0]);
      (out_corr + data_offset)[1] =
          octahedron_tool_box_.MakePositive(pos_correction[1]);
    } else {
      flip_normal_bit_encoder_.EncodeBit(true);
      (out_corr + data_offset)[0] =
          octahedron_tool_box_.MakePositive(neg_correction[0]);
      (out_corr + data_offset)[1] =
          octahedron_tool_box_.MakePositive(neg_correction[1]);
    }
  }
  return true;
}

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeGeometricNormalEncoder<
    DataTypeT, TransformT, MeshDataT>::EncodePredictionData(EncoderBuffer
                                                                *buffer) {
  if (!this->transform().EncodeTransformData(buffer)) {
    return false;
  }

  // Encode normal flips.
  flip_normal_bit_encoder_.EndEncoding(buffer);
  return true;
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_GEOMETRIC_NORMAL_ENCODER_H_
