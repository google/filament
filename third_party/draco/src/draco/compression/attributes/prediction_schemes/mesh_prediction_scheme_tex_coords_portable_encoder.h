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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_PORTABLE_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_PORTABLE_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_predictor.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"

namespace draco {

// Prediction scheme designed for predicting texture coordinates from known
// spatial position of vertices. For isometric parametrizations, the ratios
// between triangle edge lengths should be about the same in both the spatial
// and UV coordinate spaces, which makes the positions a good predictor for the
// UV coordinates. Note that this may not be the optimal approach for other
// parametrizations such as projective ones.
template <typename DataTypeT, class TransformT, class MeshDataT>
class MeshPredictionSchemeTexCoordsPortableEncoder
    : public MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT> {
 public:
  using CorrType = typename MeshPredictionSchemeEncoder<DataTypeT, TransformT,
                                                        MeshDataT>::CorrType;
  MeshPredictionSchemeTexCoordsPortableEncoder(const PointAttribute *attribute,
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
    return MESH_PREDICTION_TEX_COORDS_PORTABLE;
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
  MeshPredictionSchemeTexCoordsPortablePredictor<DataTypeT, MeshDataT>
      predictor_;
};

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeTexCoordsPortableEncoder<DataTypeT, TransformT,
                                                  MeshDataT>::
    ComputeCorrectionValues(const DataTypeT *in_data, CorrType *out_corr,
                            int size, int num_components,
                            const PointIndex *entry_to_point_id_map) {
  predictor_.SetEntryToPointIdMap(entry_to_point_id_map);
  this->transform().Init(in_data, size, num_components);
  // We start processing from the end because this prediction uses data from
  // previous entries that could be overwritten when an entry is processed.
  for (int p =
           static_cast<int>(this->mesh_data().data_to_corner_map()->size() - 1);
       p >= 0; --p) {
    const CornerIndex corner_id = this->mesh_data().data_to_corner_map()->at(p);
    predictor_.template ComputePredictedValue<true>(corner_id, in_data, p);

    const int dst_offset = p * num_components;
    this->transform().ComputeCorrection(in_data + dst_offset,
                                        predictor_.predicted_value(),
                                        out_corr + dst_offset);
  }
  return true;
}

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeTexCoordsPortableEncoder<
    DataTypeT, TransformT, MeshDataT>::EncodePredictionData(EncoderBuffer
                                                                *buffer) {
  // Encode the delta-coded orientations using arithmetic coding.
  const int32_t num_orientations = predictor_.num_orientations();
  buffer->Encode(num_orientations);
  bool last_orientation = true;
  RAnsBitEncoder encoder;
  encoder.StartEncoding();
  for (int i = 0; i < num_orientations; ++i) {
    const bool orientation = predictor_.orientation(i);
    encoder.EncodeBit(orientation == last_orientation);
    last_orientation = orientation;
  }
  encoder.EndEncoding(buffer);
  return MeshPredictionSchemeEncoder<DataTypeT, TransformT,
                                     MeshDataT>::EncodePredictionData(buffer);
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_PORTABLE_ENCODER_H_
