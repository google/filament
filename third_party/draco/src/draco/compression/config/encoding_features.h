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
// File provides helpful macros that define features available for encoding
// the input of the input geometry. These macros can be used as an input in
// the EncoderOptions::SetSupportedFeature() method instead of the text.
// The most recent set of features supported
// by the default implementation is:
//
// kEdgebreaker
//     - edgebreaker method for encoding meshes.
// kPredictiveEdgebreaker
//     - advanced version of the edgebreaker method (slower but better
//       compression).
//
#ifndef DRACO_COMPRESSION_CONFIG_ENCODING_FEATURES_H_
#define DRACO_COMPRESSION_CONFIG_ENCODING_FEATURES_H_

namespace draco {
namespace features {

constexpr const char *kEdgebreaker = "standard_edgebreaker";
constexpr const char *kPredictiveEdgebreaker = "predictive_edgebreaker";

}  // namespace features
}  // namespace draco

#endif  // DRACO_COMPRESSION_CONFIG_ENCODING_FEATURES_H_
