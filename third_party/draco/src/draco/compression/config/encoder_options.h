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
#ifndef DRACO_COMPRESSION_CONFIG_ENCODER_OPTIONS_H_
#define DRACO_COMPRESSION_CONFIG_ENCODER_OPTIONS_H_

#include "draco/attributes/geometry_attribute.h"
#include "draco/compression/config/draco_options.h"
#include "draco/compression/config/encoding_features.h"
#include "draco/draco_features.h"

namespace draco {

// EncoderOptions allow users to specify so called feature options that are used
// to inform the encoder which encoding features can be used (i.e. which
// features are going to be available to the decoder).
template <typename AttributeKeyT>
class EncoderOptionsBase : public DracoOptions<AttributeKeyT> {
 public:
  static EncoderOptionsBase CreateDefaultOptions() {
    EncoderOptionsBase options;
#ifdef DRACO_STANDARD_EDGEBREAKER_SUPPORTED
    options.SetSupportedFeature(features::kEdgebreaker, true);
#endif
#ifdef DRACO_PREDICTIVE_EDGEBREAKER_SUPPORTED
    options.SetSupportedFeature(features::kPredictiveEdgebreaker, true);
#endif
    return options;
  }
  static EncoderOptionsBase CreateEmptyOptions() {
    return EncoderOptionsBase();
  }

  // Returns speed options with default value of 5.
  int GetEncodingSpeed() const {
    return this->GetGlobalInt("encoding_speed", 5);
  }
  int GetDecodingSpeed() const {
    return this->GetGlobalInt("decoding_speed", 5);
  }

  // Returns the maximum speed for both encoding/decoding.
  int GetSpeed() const {
    const int encoding_speed = this->GetGlobalInt("encoding_speed", -1);
    const int decoding_speed = this->GetGlobalInt("decoding_speed", -1);
    const int max_speed = std::max(encoding_speed, decoding_speed);
    if (max_speed == -1) {
      return 5;  // Default value.
    }
    return max_speed;
  }

  void SetSpeed(int encoding_speed, int decoding_speed) {
    this->SetGlobalInt("encoding_speed", encoding_speed);
    this->SetGlobalInt("decoding_speed", decoding_speed);
  }
  bool IsSpeedSet() const {
    return this->IsGlobalOptionSet("encoding_speed") ||
           this->IsGlobalOptionSet("decoding_speed");
  }

  // Sets a given feature as supported or unsupported by the target decoder.
  // Encoder will always use only supported features when encoding the input
  // geometry.
  void SetSupportedFeature(const std::string &name, bool supported) {
    feature_options_.SetBool(name, supported);
  }
  bool IsFeatureSupported(const std::string &name) const {
    return feature_options_.GetBool(name);
  }

  void SetFeatureOptions(const Options &options) { feature_options_ = options; }
  const Options &GetFeaturelOptions() const { return feature_options_; }

 private:
  // Use helper methods to construct the encoder options.
  // See CreateDefaultOptions();
  EncoderOptionsBase() {}

  // List of supported/unsupported features that can be used by the encoder.
  Options feature_options_;
};

// Encoder options where attributes are identified by their attribute id.
// Used to set options that are specific to a given geometry.
typedef EncoderOptionsBase<int32_t> EncoderOptions;

}  // namespace draco

#endif  // DRACO_COMPRESSION_CONFIG_ENCODER_OPTIONS_H_
