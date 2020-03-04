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
#include "draco/metadata/metadata_encoder.h"

#include "draco/core/varint_encoding.h"

namespace draco {

bool MetadataEncoder::EncodeMetadata(EncoderBuffer *out_buffer,
                                     const Metadata *metadata) {
  const std::map<std::string, EntryValue> &entries = metadata->entries();
  // Encode number of entries.
  EncodeVarint(static_cast<uint32_t>(metadata->num_entries()), out_buffer);
  // Encode all entries.
  for (const auto &entry : entries) {
    if (!EncodeString(out_buffer, entry.first)) {
      return false;
    }
    const std::vector<uint8_t> &entry_value = entry.second.data();
    const uint32_t data_size = static_cast<uint32_t>(entry_value.size());
    EncodeVarint(data_size, out_buffer);
    out_buffer->Encode(entry_value.data(), data_size);
  }
  const std::map<std::string, std::unique_ptr<Metadata>> &sub_metadatas =
      metadata->sub_metadatas();
  // Encode number of sub-metadata
  EncodeVarint(static_cast<uint32_t>(sub_metadatas.size()), out_buffer);
  // Encode each sub-metadata
  for (auto &&sub_metadata_entry : sub_metadatas) {
    if (!EncodeString(out_buffer, sub_metadata_entry.first)) {
      return false;
    }
    EncodeMetadata(out_buffer, sub_metadata_entry.second.get());
  }

  return true;
}

bool MetadataEncoder::EncodeAttributeMetadata(
    EncoderBuffer *out_buffer, const AttributeMetadata *metadata) {
  if (!metadata) {
    return false;
  }
  // Encode attribute id.
  EncodeVarint(metadata->att_unique_id(), out_buffer);
  EncodeMetadata(out_buffer, static_cast<const Metadata *>(metadata));
  return true;
}

bool MetadataEncoder::EncodeGeometryMetadata(EncoderBuffer *out_buffer,
                                             const GeometryMetadata *metadata) {
  if (!metadata) {
    return false;
  }
  // Encode number of attribute metadata.
  const std::vector<std::unique_ptr<AttributeMetadata>> &att_metadatas =
      metadata->attribute_metadatas();
  // TODO(draco-eng): Limit the number of attributes.
  EncodeVarint(static_cast<uint32_t>(att_metadatas.size()), out_buffer);
  // Encode each attribute metadata
  for (auto &&att_metadata : att_metadatas) {
    EncodeAttributeMetadata(out_buffer, att_metadata.get());
  }
  // Encode normal metadata part.
  EncodeMetadata(out_buffer, static_cast<const Metadata *>(metadata));

  return true;
}

bool MetadataEncoder::EncodeString(EncoderBuffer *out_buffer,
                                   const std::string &str) {
  // We only support string of maximum length 255 which is using one byte to
  // encode the length.
  if (str.size() > 255) {
    return false;
  }
  if (str.empty()) {
    out_buffer->Encode(static_cast<uint8_t>(0));
  } else {
    out_buffer->Encode(static_cast<uint8_t>(str.size()));
    out_buffer->Encode(str.c_str(), str.size());
  }
  return true;
}
}  // namespace draco
