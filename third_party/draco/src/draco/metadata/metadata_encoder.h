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
#ifndef DRACO_METADATA_METADATA_ENCODER_H_
#define DRACO_METADATA_METADATA_ENCODER_H_

#include "draco/core/encoder_buffer.h"
#include "draco/metadata/geometry_metadata.h"
#include "draco/metadata/metadata.h"

namespace draco {

// Class for encoding metadata. It could encode either base Metadata class or
// a metadata of a geometry, e.g. a point cloud.
class MetadataEncoder {
 public:
  MetadataEncoder() {}

  bool EncodeGeometryMetadata(EncoderBuffer *out_buffer,
                              const GeometryMetadata *metadata);
  bool EncodeMetadata(EncoderBuffer *out_buffer, const Metadata *metadata);

 private:
  bool EncodeAttributeMetadata(EncoderBuffer *out_buffer,
                               const AttributeMetadata *metadata);
  bool EncodeString(EncoderBuffer *out_buffer, const std::string &str);
};
}  // namespace draco

#endif  // DRACO_METADATA_METADATA_ENCODER_H_
