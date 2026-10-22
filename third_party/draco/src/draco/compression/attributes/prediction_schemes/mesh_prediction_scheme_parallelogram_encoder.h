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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_PARALLELOGRAM_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_PARALLELOGRAM_ENCODER_H_

#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_shared.h"

namespace draco {

// Parallelogram prediction predicts an attribute value V from three vertices
// on the opposite face to the predicted vertex. The values on the three
// vertices are used to construct a parallelogram V' = O - A - B, where O is the
// value on the opposite vertex, and A, B are values on the shared vertices:
//     V
//    / \
//   /   \
//  /     \
// A-------B
//  \     /
//   \   /
//    \ /
//     O
//
template <typename DataTypeT, class TransformT, class MeshDataT>
class MeshPredictionSchemeParallelogramEncoder
    : public MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT> {
 public:
  using CorrType =
      typename PredictionSchemeEncoder<DataTypeT, TransformT>::CorrType;
  using CornerTable = typename MeshDataT::CornerTable;
  explicit MeshPredictionSchemeParallelogramEncoder(
      const PointAttribute *attribute)
      : MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT>(
            attribute) {}
  MeshPredictionSchemeParallelogramEncoder(const PointAttribute *attribute,
                                           const TransformT &transform,
                                           const MeshDataT &mesh_data)
      : MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT>(
            attribute, transform, mesh_data) {}

  bool ComputeCorrectionValues(
      const DataTypeT *in_data, CorrType *out_corr, int size,
      int num_components, const PointIndex *entry_to_point_id_map) override;
  PredictionSchemeMethod GetPredictionMethod() const override {
    return MESH_PREDICTION_PARALLELOGRAM;
  }

  bool IsInitialized() const override {
    return this->mesh_data().IsInitialized();
  }
};

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeParallelogramEncoder<DataTypeT, TransformT,
                                              MeshDataT>::
    ComputeCorrectionValues(const DataTypeT *in_data, CorrType *out_corr,
                            int size, int num_components,
                            const PointIndex * /* entry_to_point_id_map */) {
  this->transform().Init(in_data, size, num_components);
  // For storage of prediction values (already initialized to zero).
  std::unique_ptr<DataTypeT[]> pred_vals(new DataTypeT[num_components]());

  // We start processing from the end because this prediction uses data from
  // previous entries that could be overwritten when an entry is processed.
  const CornerTable *const table = this->mesh_data().corner_table();
  const std::vector<int32_t> *const vertex_to_data_map =
      this->mesh_data().vertex_to_data_map();
  for (int p =
           static_cast<int>(this->mesh_data().data_to_corner_map()->size() - 1);
       p > 0; --p) {
    const CornerIndex corner_id = this->mesh_data().data_to_corner_map()->at(p);
    const int dst_offset = p * num_components;
    if (!ComputeParallelogramPrediction(p, corner_id, table,
                                        *vertex_to_data_map, in_data,
                                        num_components, pred_vals.get())) {
      // Parallelogram could not be computed, Possible because some of the
      // vertices are not valid (not encoded yet).
      // We use the last encoded point as a reference (delta coding).
      const int src_offset = (p - 1) * num_components;
      this->transform().ComputeCorrection(
          in_data + dst_offset, in_data + src_offset, out_corr + dst_offset);
    } else {
      // Apply the parallelogram prediction.
      this->transform().ComputeCorrection(in_data + dst_offset, pred_vals.get(),
                                          out_corr + dst_offset);
    }
  }
  // First element is always fixed because it cannot be predicted.
  for (int i = 0; i < num_components; ++i) {
    pred_vals[i] = static_cast<DataTypeT>(0);
  }
  this->transform().ComputeCorrection(in_data, pred_vals.get(), out_corr);
  return true;
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_PARALLELOGRAM_ENCODER_H_
