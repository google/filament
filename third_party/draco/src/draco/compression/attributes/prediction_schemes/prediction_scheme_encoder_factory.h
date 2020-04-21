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
// Functions for creating prediction schemes for encoders using the provided
// prediction method id.

#ifndef DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODER_FACTORY_H_
#define DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODER_FACTORY_H_

#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_constrained_multi_parallelogram_encoder.h"
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_geometric_normal_encoder.h"
#endif
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_multi_parallelogram_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_parallelogram_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_encoder.h"
#include "draco/compression/attributes/prediction_schemes/mesh_prediction_scheme_tex_coords_portable_encoder.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_delta_encoder.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_encoder.h"
#include "draco/compression/attributes/prediction_schemes/prediction_scheme_factory.h"
#include "draco/compression/mesh/mesh_encoder.h"

namespace draco {

// Selects a prediction method based on the input geometry type and based on the
// encoder options.
PredictionSchemeMethod SelectPredictionMethod(int att_id,
                                              const PointCloudEncoder *encoder);

// Factory class for creating mesh prediction schemes.
template <typename DataTypeT>
struct MeshPredictionSchemeEncoderFactory {
  template <class TransformT, class MeshDataT>
  std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>> operator()(
      PredictionSchemeMethod method, const PointAttribute *attribute,
      const TransformT &transform, const MeshDataT &mesh_data,
      uint16_t bitstream_version) {
    if (method == MESH_PREDICTION_PARALLELOGRAM) {
      return std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>(
          new MeshPredictionSchemeParallelogramEncoder<DataTypeT, TransformT,
                                                       MeshDataT>(
              attribute, transform, mesh_data));
    } else if (method == MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM) {
      return std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>(
          new MeshPredictionSchemeConstrainedMultiParallelogramEncoder<
              DataTypeT, TransformT, MeshDataT>(attribute, transform,
                                                mesh_data));
    } else if (method == MESH_PREDICTION_TEX_COORDS_PORTABLE) {
      return std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>(
          new MeshPredictionSchemeTexCoordsPortableEncoder<
              DataTypeT, TransformT, MeshDataT>(attribute, transform,
                                                mesh_data));
    }
#ifdef DRACO_NORMAL_ENCODING_SUPPORTED
    else if (method == MESH_PREDICTION_GEOMETRIC_NORMAL) {
      return std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>(
          new MeshPredictionSchemeGeometricNormalEncoder<DataTypeT, TransformT,
                                                         MeshDataT>(
              attribute, transform, mesh_data));
    }
#endif
    return nullptr;
  }
};

// Creates a prediction scheme for a given encoder and given prediction method.
// The prediction schemes are automatically initialized with encoder specific
// data if needed.
template <typename DataTypeT, class TransformT>
std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>
CreatePredictionSchemeForEncoder(PredictionSchemeMethod method, int att_id,
                                 const PointCloudEncoder *encoder,
                                 const TransformT &transform) {
  const PointAttribute *const att = encoder->point_cloud()->attribute(att_id);
  if (method == PREDICTION_UNDEFINED) {
    method = SelectPredictionMethod(att_id, encoder);
  }
  if (method == PREDICTION_NONE) {
    return nullptr;  // No prediction is used.
  }
  if (encoder->GetGeometryType() == TRIANGULAR_MESH) {
    // Cast the encoder to mesh encoder. This is not necessarily safe if there
    // is some other encoder decides to use TRIANGULAR_MESH as the return type,
    // but unfortunately there is not nice work around for this without using
    // RTTI (double dispatch and similar concepts will not work because of the
    // template nature of the prediction schemes).
    const MeshEncoder *const mesh_encoder =
        static_cast<const MeshEncoder *>(encoder);
    auto ret = CreateMeshPredictionScheme<
        MeshEncoder, PredictionSchemeEncoder<DataTypeT, TransformT>,
        MeshPredictionSchemeEncoderFactory<DataTypeT>>(
        mesh_encoder, method, att_id, transform, kDracoMeshBitstreamVersion);
    if (ret) {
      return ret;
    }
    // Otherwise try to create another prediction scheme.
  }
  // Create delta encoder.
  return std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>(
      new PredictionSchemeDeltaEncoder<DataTypeT, TransformT>(att, transform));
}

// Create a prediction scheme using a default transform constructor.
template <typename DataTypeT, class TransformT>
std::unique_ptr<PredictionSchemeEncoder<DataTypeT, TransformT>>
CreatePredictionSchemeForEncoder(PredictionSchemeMethod method, int att_id,
                                 const PointCloudEncoder *encoder) {
  return CreatePredictionSchemeForEncoder<DataTypeT, TransformT>(
      method, att_id, encoder, TransformT());
}

// Returns the preferred prediction scheme based on the encoder options.
PredictionSchemeMethod GetPredictionMethodFromOptions(
    int att_id, const EncoderOptions &options);

}  // namespace draco

#endif  // DRACO_COMPRESSION_ATTRIBUTES_PREDICTION_SCHEMES_PREDICTION_SCHEME_ENCODER_FACTORY_H_
