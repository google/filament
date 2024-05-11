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
#include "draco/compression/mesh/mesh_sequential_encoder.h"

#include <cstdlib>

#include "draco/compression/attributes/linear_sequencer.h"
#include "draco/compression/attributes/sequential_attribute_encoders_controller.h"
#include "draco/compression/entropy/symbol_encoding.h"
#include "draco/core/varint_encoding.h"

namespace draco {

MeshSequentialEncoder::MeshSequentialEncoder() {}

Status MeshSequentialEncoder::EncodeConnectivity() {
  // Serialize indices.
  const uint32_t num_faces = mesh()->num_faces();
  EncodeVarint(num_faces, buffer());
  EncodeVarint(static_cast<uint32_t>(mesh()->num_points()), buffer());

  // We encode all attributes in the original (possibly duplicated) format.
  // TODO(ostava): This may not be optimal if we have only one attribute or if
  // all attributes share the same index mapping.
  if (options()->GetGlobalBool("compress_connectivity", false)) {
    // 0 = Encode compressed indices.
    buffer()->Encode(static_cast<uint8_t>(0));
    if (!CompressAndEncodeIndices()) {
      return Status(Status::DRACO_ERROR, "Failed to compress connectivity.");
    }
  } else {
    // 1 = Encode indices directly.
    buffer()->Encode(static_cast<uint8_t>(1));
    // Store vertex indices using a smallest data type that fits their range.
    // TODO(ostava): This can be potentially improved by using a tighter
    // fit that is not bound by a bit-length of any particular data type.
    if (mesh()->num_points() < 256) {
      // Serialize indices as uint8_t.
      for (FaceIndex i(0); i < num_faces; ++i) {
        const auto &face = mesh()->face(i);
        buffer()->Encode(static_cast<uint8_t>(face[0].value()));
        buffer()->Encode(static_cast<uint8_t>(face[1].value()));
        buffer()->Encode(static_cast<uint8_t>(face[2].value()));
      }
    } else if (mesh()->num_points() < (1 << 16)) {
      // Serialize indices as uint16_t.
      for (FaceIndex i(0); i < num_faces; ++i) {
        const auto &face = mesh()->face(i);
        buffer()->Encode(static_cast<uint16_t>(face[0].value()));
        buffer()->Encode(static_cast<uint16_t>(face[1].value()));
        buffer()->Encode(static_cast<uint16_t>(face[2].value()));
      }
    } else if (mesh()->num_points() < (1 << 21)) {
      // Serialize indices as varint.
      for (FaceIndex i(0); i < num_faces; ++i) {
        const auto &face = mesh()->face(i);
        EncodeVarint(static_cast<uint32_t>(face[0].value()), buffer());
        EncodeVarint(static_cast<uint32_t>(face[1].value()), buffer());
        EncodeVarint(static_cast<uint32_t>(face[2].value()), buffer());
      }
    } else {
      // Serialize faces as uint32_t (default).
      for (FaceIndex i(0); i < num_faces; ++i) {
        const auto &face = mesh()->face(i);
        buffer()->Encode(face);
      }
    }
  }
  return OkStatus();
}

bool MeshSequentialEncoder::GenerateAttributesEncoder(int32_t att_id) {
  // Create only one attribute encoder that is going to encode all points in a
  // linear sequence.
  if (att_id == 0) {
    // Create a new attribute encoder only for the first attribute.
    AddAttributesEncoder(std::unique_ptr<AttributesEncoder>(
        new SequentialAttributeEncodersController(
            std::unique_ptr<PointsSequencer>(
                new LinearSequencer(point_cloud()->num_points())),
            att_id)));
  } else {
    // Reuse the existing attribute encoder for other attributes.
    attributes_encoder(0)->AddAttributeId(att_id);
  }
  return true;
}

bool MeshSequentialEncoder::CompressAndEncodeIndices() {
  // Collect all indices to a buffer and encode them.
  // Each new index is a difference from the previous value.
  std::vector<uint32_t> indices_buffer;
  int32_t last_index_value = 0;
  const int num_faces = mesh()->num_faces();
  for (FaceIndex i(0); i < num_faces; ++i) {
    const auto &face = mesh()->face(i);
    for (int j = 0; j < 3; ++j) {
      const int32_t index_value = face[j].value();
      const int32_t index_diff = index_value - last_index_value;
      // Encode signed value to an unsigned one (put the sign to lsb pos).
      const uint32_t encoded_val =
          (abs(index_diff) << 1) | (index_diff < 0 ? 1 : 0);
      indices_buffer.push_back(encoded_val);
      last_index_value = index_value;
    }
  }
  EncodeSymbols(indices_buffer.data(), static_cast<int>(indices_buffer.size()),
                1, nullptr, buffer());
  return true;
}

void MeshSequentialEncoder::ComputeNumberOfEncodedPoints() {
  set_num_encoded_points(mesh()->num_points());
}

void MeshSequentialEncoder::ComputeNumberOfEncodedFaces() {
  set_num_encoded_faces(mesh()->num_faces());
}

}  // namespace draco
