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
#ifndef DRACO_COMPRESSION_ENCODE_BASE_H_
#define DRACO_COMPRESSION_ENCODE_BASE_H_

#include "draco/attributes/geometry_attribute.h"
#include "draco/compression/config/compression_shared.h"
#include "draco/core/status.h"

namespace draco {

// Base class for our geometry encoder classes. |EncoderOptionsT| specifies
// options class used by the encoder. Please, see encode.h and expert_encode.h
// for more details and method descriptions.
template <class EncoderOptionsT>
class EncoderBase {
 public:
  typedef EncoderOptionsT OptionsType;

  EncoderBase()
      : options_(EncoderOptionsT::CreateDefaultOptions()),
        num_encoded_points_(0),
        num_encoded_faces_(0) {}
  virtual ~EncoderBase() {}

  const EncoderOptionsT &options() const { return options_; }
  EncoderOptionsT &options() { return options_; }

  // If enabled, it tells the encoder to keep track of the number of encoded
  // points and faces (default = false).
  // Note that this can slow down encoding for certain encoders.
  void SetTrackEncodedProperties(bool flag);

  // Returns the number of encoded points and faces during the last encoding
  // operation. Returns 0 if SetTrackEncodedProperties() was not set.
  size_t num_encoded_points() const { return num_encoded_points_; }
  size_t num_encoded_faces() const { return num_encoded_faces_; }

 protected:
  void Reset(const EncoderOptionsT &options) { options_ = options; }

  void Reset() { options_ = EncoderOptionsT::CreateDefaultOptions(); }

  void SetSpeedOptions(int encoding_speed, int decoding_speed) {
    options_.SetSpeed(encoding_speed, decoding_speed);
  }

  void SetEncodingMethod(int encoding_method) {
    options_.SetGlobalInt("encoding_method", encoding_method);
  }

  void SetEncodingSubmethod(int encoding_submethod) {
    options_.SetGlobalInt("encoding_submethod", encoding_submethod);
  }

  Status CheckPredictionScheme(GeometryAttribute::Type att_type,
                               int prediction_scheme) const {
    // Out of bound checks:
    if (prediction_scheme < PREDICTION_NONE) {
      return Status(Status::DRACO_ERROR,
                    "Invalid prediction scheme requested.");
    }
    if (prediction_scheme >= NUM_PREDICTION_SCHEMES) {
      return Status(Status::DRACO_ERROR,
                    "Invalid prediction scheme requested.");
    }
    // Deprecated prediction schemes:
    if (prediction_scheme == MESH_PREDICTION_TEX_COORDS_DEPRECATED) {
      return Status(Status::DRACO_ERROR,
                    "MESH_PREDICTION_TEX_COORDS_DEPRECATED is deprecated.");
    }
    if (prediction_scheme == MESH_PREDICTION_MULTI_PARALLELOGRAM) {
      return Status(Status::DRACO_ERROR,
                    "MESH_PREDICTION_MULTI_PARALLELOGRAM is deprecated.");
    }
    // Attribute specific checks:
    if (prediction_scheme == MESH_PREDICTION_TEX_COORDS_PORTABLE) {
      if (att_type != GeometryAttribute::TEX_COORD) {
        return Status(Status::DRACO_ERROR,
                      "Invalid prediction scheme for attribute type.");
      }
    }
    if (prediction_scheme == MESH_PREDICTION_GEOMETRIC_NORMAL) {
      if (att_type != GeometryAttribute::NORMAL) {
        return Status(Status::DRACO_ERROR,
                      "Invalid prediction scheme for attribute type.");
      }
    }
    // TODO(hemmer): Try to enable more prediction schemes for normals.
    if (att_type == GeometryAttribute::NORMAL) {
      if (!(prediction_scheme == PREDICTION_DIFFERENCE ||
            prediction_scheme == MESH_PREDICTION_GEOMETRIC_NORMAL)) {
        return Status(Status::DRACO_ERROR,
                      "Invalid prediction scheme for attribute type.");
      }
    }
    return OkStatus();
  }

 protected:
  void set_num_encoded_points(size_t num) { num_encoded_points_ = num; }
  void set_num_encoded_faces(size_t num) { num_encoded_faces_ = num; }

 private:
  EncoderOptionsT options_;

  size_t num_encoded_points_;
  size_t num_encoded_faces_;
};

template <class EncoderOptionsT>
void EncoderBase<EncoderOptionsT>::SetTrackEncodedProperties(bool flag) {
  options_.SetGlobalBool("store_number_of_encoded_points", flag);
  options_.SetGlobalBool("store_number_of_encoded_faces", flag);
}

}  // namespace draco

#endif  // DRACO_COMPRESSION_ENCODE_BASE_H_
