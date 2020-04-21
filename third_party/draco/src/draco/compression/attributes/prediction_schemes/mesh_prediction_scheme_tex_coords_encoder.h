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
#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_ENCODER_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_ENCODER_H_

#include <math.h>

#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_encoder.h"
#include "draco/compression/bit_coders/rans_bit_encoder.h"
#include "draco/core/varint_encoding.h"
#include "draco/core/vector_d.h"
#include "draco/mesh/corner_table.h"

namespace draco {

// Prediction scheme designed for predicting texture coordinates from known
// spatial position of vertices. For good parametrization, the ratios between
// triangle edge lengths should be about the same in both the spatial and UV
// coordinate spaces, which makes the positions a good predictor for the UV
// coordinates.
template <typename DataTypeT, class TransformT, class MeshDataT>
class MeshPredictionSchemeTexCoordsEncoder
    : public MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT> {
 public:
  using CorrType = typename MeshPredictionSchemeEncoder<DataTypeT, TransformT,
                                                        MeshDataT>::CorrType;
  MeshPredictionSchemeTexCoordsEncoder(const PointAttribute *attribute,
                                       const TransformT &transform,
                                       const MeshDataT &mesh_data)
      : MeshPredictionSchemeEncoder<DataTypeT, TransformT, MeshDataT>(
            attribute, transform, mesh_data),
        pos_attribute_(nullptr),
        entry_to_point_id_map_(nullptr),
        num_components_(0) {}

  bool ComputeCorrectionValues(
      const DataTypeT *in_data, CorrType *out_corr, int size,
      int num_components, const PointIndex *entry_to_point_id_map) override;

  bool EncodePredictionData(EncoderBuffer *buffer) override;

  PredictionSchemeMethod GetPredictionMethod() const override {
    return MESH_PREDICTION_TEX_COORDS_DEPRECATED;
  }

  bool IsInitialized() const override {
    if (pos_attribute_ == nullptr) {
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
    pos_attribute_ = att;
    return true;
  }

 protected:
  Vector3f GetPositionForEntryId(int entry_id) const {
    const PointIndex point_id = entry_to_point_id_map_[entry_id];
    Vector3f pos;
    pos_attribute_->ConvertValue(pos_attribute_->mapped_index(point_id),
                                 &pos[0]);
    return pos;
  }

  Vector2f GetTexCoordForEntryId(int entry_id, const DataTypeT *data) const {
    const int data_offset = entry_id * num_components_;
    return Vector2f(static_cast<float>(data[data_offset]),
                    static_cast<float>(data[data_offset + 1]));
  }

  void ComputePredictedValue(CornerIndex corner_id, const DataTypeT *data,
                             int data_id);

 private:
  const PointAttribute *pos_attribute_;
  const PointIndex *entry_to_point_id_map_;
  std::unique_ptr<DataTypeT[]> predicted_value_;
  int num_components_;
  // Encoded / decoded array of UV flips.
  std::vector<bool> orientations_;
};

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeTexCoordsEncoder<DataTypeT, TransformT, MeshDataT>::
    ComputeCorrectionValues(const DataTypeT *in_data, CorrType *out_corr,
                            int size, int num_components,
                            const PointIndex *entry_to_point_id_map) {
  num_components_ = num_components;
  entry_to_point_id_map_ = entry_to_point_id_map;
  predicted_value_ =
      std::unique_ptr<DataTypeT[]>(new DataTypeT[num_components]);
  this->transform().Init(in_data, size, num_components);
  // We start processing from the end because this prediction uses data from
  // previous entries that could be overwritten when an entry is processed.
  for (int p =
           static_cast<int>(this->mesh_data().data_to_corner_map()->size()) - 1;
       p >= 0; --p) {
    const CornerIndex corner_id = this->mesh_data().data_to_corner_map()->at(p);
    ComputePredictedValue(corner_id, in_data, p);

    const int dst_offset = p * num_components;
    this->transform().ComputeCorrection(
        in_data + dst_offset, predicted_value_.get(), out_corr + dst_offset);
  }
  return true;
}

template <typename DataTypeT, class TransformT, class MeshDataT>
bool MeshPredictionSchemeTexCoordsEncoder<DataTypeT, TransformT, MeshDataT>::
    EncodePredictionData(EncoderBuffer *buffer) {
  // Encode the delta-coded orientations using arithmetic coding.
  const uint32_t num_orientations = static_cast<uint32_t>(orientations_.size());
  EncodeVarint(num_orientations, buffer);
  bool last_orientation = true;
  RAnsBitEncoder encoder;
  encoder.StartEncoding();
  for (bool orientation : orientations_) {
    encoder.EncodeBit(orientation == last_orientation);
    last_orientation = orientation;
  }
  encoder.EndEncoding(buffer);
  return MeshPredictionSchemeEncoder<DataTypeT, TransformT,
                                     MeshDataT>::EncodePredictionData(buffer);
}

template <typename DataTypeT, class TransformT, class MeshDataT>
void MeshPredictionSchemeTexCoordsEncoder<DataTypeT, TransformT, MeshDataT>::
    ComputePredictedValue(CornerIndex corner_id, const DataTypeT *data,
                          int data_id) {
  // Compute the predicted UV coordinate from the positions on all corners
  // of the processed triangle. For the best prediction, the UV coordinates
  // on the next/previous corners need to be already encoded/decoded.
  const CornerIndex next_corner_id =
      this->mesh_data().corner_table()->Next(corner_id);
  const CornerIndex prev_corner_id =
      this->mesh_data().corner_table()->Previous(corner_id);
  // Get the encoded data ids from the next and previous corners.
  // The data id is the encoding order of the UV coordinates.
  int next_data_id, prev_data_id;

  int next_vert_id, prev_vert_id;
  next_vert_id =
      this->mesh_data().corner_table()->Vertex(next_corner_id).value();
  prev_vert_id =
      this->mesh_data().corner_table()->Vertex(prev_corner_id).value();

  next_data_id = this->mesh_data().vertex_to_data_map()->at(next_vert_id);
  prev_data_id = this->mesh_data().vertex_to_data_map()->at(prev_vert_id);

  if (prev_data_id < data_id && next_data_id < data_id) {
    // Both other corners have available UV coordinates for prediction.
    const Vector2f n_uv = GetTexCoordForEntryId(next_data_id, data);
    const Vector2f p_uv = GetTexCoordForEntryId(prev_data_id, data);
    if (p_uv == n_uv) {
      // We cannot do a reliable prediction on degenerated UV triangles.
      predicted_value_[0] = static_cast<int>(p_uv[0]);
      predicted_value_[1] = static_cast<int>(p_uv[1]);
      return;
    }

    // Get positions at all corners.
    const Vector3f tip_pos = GetPositionForEntryId(data_id);
    const Vector3f next_pos = GetPositionForEntryId(next_data_id);
    const Vector3f prev_pos = GetPositionForEntryId(prev_data_id);
    // Use the positions of the above triangle to predict the texture coordinate
    // on the tip corner C.
    // Convert the triangle into a new coordinate system defined by orthogonal
    // bases vectors S, T, where S is vector prev_pos - next_pos and T is an
    // perpendicular vector to S in the same plane as vector the
    // tip_pos - next_pos.
    // The transformed triangle in the new coordinate system is then going to
    // be represented as:
    //
    //        1 ^
    //          |
    //          |
    //          |   C
    //          |  /  \
    //          | /      \
    //          |/          \
    //          N--------------P
    //          0              1
    //
    // Where next_pos point (N) is at position (0, 0), prev_pos point (P) is
    // at (1, 0). Our goal is to compute the position of the tip_pos point (C)
    // in this new coordinate space (s, t).
    //
    const Vector3f pn = prev_pos - next_pos;
    const Vector3f cn = tip_pos - next_pos;
    const float pn_norm2_squared = pn.SquaredNorm();
    // Coordinate s of the tip corner C is simply the dot product of the
    // normalized vectors |pn| and |cn| (normalized by the length of |pn|).
    // Since both of these vectors are normalized, we don't need to perform the
    // normalization explicitly and instead we can just use the squared norm
    // of |pn| as a denominator of the resulting dot product of non normalized
    // vectors.
    float s, t;
    // |pn_norm2_squared| can be exactly 0 when the next_pos and prev_pos are
    // the same positions (e.g. because they were quantized to the same
    // location).
    if (pn_norm2_squared > 0) {
      s = pn.Dot(cn) / pn_norm2_squared;
      // To get the coordinate t, we can use formula:
      //      t = |C-N - (P-N) * s| / |P-N|
      // Do not use std::sqrt to avoid changes in the bitstream.
      t = sqrt((cn - pn * s).SquaredNorm() / pn_norm2_squared);
    } else {
      s = 0;
      t = 0;
    }

    // Now we need to transform the point (s, t) to the texture coordinate space
    // UV. We know the UV coordinates on points N and P (N_UV and P_UV). Lets
    // denote P_UV - N_UV = PN_UV. PN_UV is then 2 dimensional vector that can
    // be used to define transformation from the normalized coordinate system
    // to the texture coordinate system using a 3x3 affine matrix M:
    //
    //  M = | PN_UV[0]  -PN_UV[1]  N_UV[0] |
    //      | PN_UV[1]   PN_UV[0]  N_UV[1] |
    //      | 0          0         1       |
    //
    // The predicted point C_UV in the texture space is then equal to
    // C_UV = M * (s, t, 1). Because the triangle in UV space may be flipped
    // around the PN_UV axis, we also need to consider point C_UV' = M * (s, -t)
    // as the prediction.
    const Vector2f pn_uv = p_uv - n_uv;
    const float pnus = pn_uv[0] * s + n_uv[0];
    const float pnut = pn_uv[0] * t;
    const float pnvs = pn_uv[1] * s + n_uv[1];
    const float pnvt = pn_uv[1] * t;
    Vector2f predicted_uv;

    // When encoding compute both possible vectors and determine which one
    // results in a better prediction.
    const Vector2f predicted_uv_0(pnus - pnvt, pnvs + pnut);
    const Vector2f predicted_uv_1(pnus + pnvt, pnvs - pnut);
    const Vector2f c_uv = GetTexCoordForEntryId(data_id, data);
    if ((c_uv - predicted_uv_0).SquaredNorm() <
        (c_uv - predicted_uv_1).SquaredNorm()) {
      predicted_uv = predicted_uv_0;
      orientations_.push_back(true);
    } else {
      predicted_uv = predicted_uv_1;
      orientations_.push_back(false);
    }
    if (std::is_integral<DataTypeT>::value) {
      // Round the predicted value for integer types.
      predicted_value_[0] = static_cast<int>(floor(predicted_uv[0] + 0.5));
      predicted_value_[1] = static_cast<int>(floor(predicted_uv[1] + 0.5));
    } else {
      predicted_value_[0] = static_cast<int>(predicted_uv[0]);
      predicted_value_[1] = static_cast<int>(predicted_uv[1]);
    }
    return;
  }
  // Else we don't have available textures on both corners. For such case we
  // can't use positions for predicting the uv value and we resort to delta
  // coding.
  int data_offset = 0;
  if (prev_data_id < data_id) {
    // Use the value on the previous corner as the prediction.
    data_offset = prev_data_id * num_components_;
  }
  if (next_data_id < data_id) {
    // Use the value on the next corner as the prediction.
    data_offset = next_data_id * num_components_;
  } else {
    // None of the other corners have a valid value. Use the last encoded value
    // as the prediction if possible.
    if (data_id > 0) {
      data_offset = (data_id - 1) * num_components_;
    } else {
      // We are encoding the first value. Predict 0.
      for (int i = 0; i < num_components_; ++i) {
        predicted_value_[i] = 0;
      }
      return;
    }
  }
  for (int i = 0; i < num_components_; ++i) {
    predicted_value_[i] = data[data_offset + i];
  }
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_MESH_PREDICTION_SCHEME_TEX_COORDS_H_
