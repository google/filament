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
#ifndef DRACO_COMPRESSION_EXPERT_ENCODE_H_
#define DRACO_COMPRESSION_EXPERT_ENCODE_H_

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/config/encoder_options.h"
#include "draco/compression/encode_base.h"
#include "draco/core/encoder_buffer.h"
#include "draco/core/status.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Advanced helper class for encoding geometry using the Draco compression
// library. Unlike the basic Encoder (encode.h), this class allows users to
// specify options for each attribute individually using provided attribute ids.
// The drawback of this encoder is that it can be used to encode only one model
// at a time, and for each new model the options need to be set again,
class ExpertEncoder : public EncoderBase<EncoderOptions> {
 public:
  typedef EncoderBase<EncoderOptions> Base;
  typedef EncoderOptions OptionsType;

  explicit ExpertEncoder(const PointCloud &point_cloud);
  explicit ExpertEncoder(const Mesh &mesh);

  // Encodes the geometry provided in the constructor to the target buffer.
  Status EncodeToBuffer(EncoderBuffer *out_buffer);

  // Set encoder options used during the geometry encoding. Note that this call
  // overwrites any modifications to the options done with the functions below.
  void Reset(const EncoderOptions &options);
  void Reset();

  // Sets the desired encoding and decoding speed for the given options.
  //
  //  0 = slowest speed, but the best compression.
  // 10 = fastest, but the worst compression.
  // -1 = undefined.
  //
  // Note that both speed options affect the encoder choice of used methods and
  // algorithms. For example, a requirement for fast decoding may prevent the
  // encoder from using the best compression methods even if the encoding speed
  // is set to 0. In general, the faster of the two options limits the choice of
  // features that can be used by the encoder. Additionally, setting
  // |decoding_speed| to be faster than the |encoding_speed| may allow the
  // encoder to choose the optimal method out of the available features for the
  // given |decoding_speed|.
  void SetSpeedOptions(int encoding_speed, int decoding_speed);

  // Sets the quantization compression options for a specific attribute. The
  // attribute values will be quantized in a box defined by the maximum extent
  // of the attribute values. I.e., the actual precision of this option depends
  // on the scale of the attribute values.
  void SetAttributeQuantization(int32_t attribute_id, int quantization_bits);

  // Sets the explicit quantization compression for a named attribute. The
  // attribute values will be quantized in a coordinate system defined by the
  // provided origin and range (the input values should be within interval:
  // <origin, origin + range>).
  void SetAttributeExplicitQuantization(int32_t attribute_id,
                                        int quantization_bits, int num_dims,
                                        const float *origin, float range);

  // Enables/disables built in entropy coding of attribute values. Disabling
  // this option may be useful to improve the performance when third party
  // compression is used on top of the Draco compression. Default: [true].
  void SetUseBuiltInAttributeCompression(bool enabled);

  // Sets the desired encoding method for a given geometry. By default, encoding
  // method is selected based on the properties of the input geometry and based
  // on the other options selected in the used EncoderOptions (such as desired
  // encoding and decoding speed). This function should be called only when a
  // specific method is required.
  //
  // |encoding_method| can be one of the values defined in
  // compression/config/compression_shared.h based on the type of the input
  // geometry that is going to be encoded. For point clouds, allowed entries are
  //   POINT_CLOUD_SEQUENTIAL_ENCODING
  //   POINT_CLOUD_KD_TREE_ENCODING
  //
  // For meshes the input can be
  //   MESH_SEQUENTIAL_ENCODING
  //   MESH_EDGEBREAKER_ENCODING
  //
  // If the selected method cannot be used for the given input, the subsequent
  // call of EncodePointCloudToBuffer or EncodeMeshToBuffer is going to fail.
  void SetEncodingMethod(int encoding_method);

  // Sets the desired encoding submethod, only for MESH_EDGEBREAKER_ENCODING.
  // Valid values for |encoding_submethod| are:
  //   MESH_EDGEBREAKER_STANDARD_ENCODING
  //   MESH_EDGEBREAKER_VALENCE_ENCODING
  // see also compression/config/compression_shared.h.
  void SetEncodingSubmethod(int encoding_submethod);

  // Sets the desired prediction method for a given attribute. By default,
  // prediction scheme is selected automatically by the encoder using other
  // provided options (such as speed) and input geometry type (mesh, point
  // cloud). This function should be called only when a specific prediction is
  // preferred (e.g., when it is known that the encoder would select a less
  // optimal prediction for the given input data).
  //
  // |prediction_scheme_method| should be one of the entries defined in
  // compression/config/compression_shared.h :
  //
  //   PREDICTION_NONE - use no prediction.
  //   PREDICTION_DIFFERENCE - delta coding
  //   MESH_PREDICTION_PARALLELOGRAM - parallelogram prediction for meshes.
  //   MESH_PREDICTION_CONSTRAINED_PARALLELOGRAM
  //      - better and more costly version of the parallelogram prediction.
  //   MESH_PREDICTION_TEX_COORDS_PORTABLE
  //      - specialized predictor for tex coordinates.
  //   MESH_PREDICTION_GEOMETRIC_NORMAL
  //      - specialized predictor for normal coordinates.
  //
  // Note that in case the desired prediction cannot be used, the default
  // prediction will be automatically used instead.
  Status SetAttributePredictionScheme(int32_t attribute_id,
                                      int prediction_scheme_method);

#ifdef DRACO_TRANSCODER_SUPPORTED
  // Applies grid quantization to position attribute in point cloud |pc| at
  // |attribute_index| with a given grid |spacing|.
  Status SetAttributeGridQuantization(const PointCloud &pc, int attribute_index,
                                      float spacing);
#endif  // DRACO_TRANSCODER_SUPPORTED

 private:
  Status EncodePointCloudToBuffer(const PointCloud &pc,
                                  EncoderBuffer *out_buffer);

  Status EncodeMeshToBuffer(const Mesh &m, EncoderBuffer *out_buffer);

#ifdef DRACO_TRANSCODER_SUPPORTED
  // Applies compression options stored in |pc|.
  Status ApplyCompressionOptions(const PointCloud &pc);
  Status ApplyGridQuantization(const PointCloud &pc, int attribute_index);
#endif  // DRACO_TRANSCODER_SUPPORTED

  const PointCloud *point_cloud_;
  const Mesh *mesh_;
};

}  // namespace draco

#endif  // DRACO_COMPRESSION_EXPERT_ENCODE_H_
