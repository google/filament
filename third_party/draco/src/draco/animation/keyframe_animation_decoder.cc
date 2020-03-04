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
#include "draco/animation/keyframe_animation_decoder.h"

namespace draco {

Status KeyframeAnimationDecoder::Decode(const DecoderOptions &options,
                                        DecoderBuffer *in_buffer,
                                        KeyframeAnimation *animation) {
  const auto status = PointCloudSequentialDecoder::Decode(
      options, in_buffer, static_cast<PointCloud *>(animation));
  if (!status.ok()) {
    return status;
  }
  return OkStatus();
}

}  // namespace draco
