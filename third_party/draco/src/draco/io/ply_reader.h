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
// File contains helper classes used for parsing of PLY files. The classes are
// used by the PlyDecoder (ply_decoder.h) to read a point cloud or mesh from a
// source PLY file.
// TODO(ostava): Currently, we support only binary PLYs encoded in the little
// endian format ("format binary_little_endian 1.0").

#ifndef DRACO_IO_PLY_READER_H_
#define DRACO_IO_PLY_READER_H_

#include <map>
#include <vector>

#include "draco/core/decoder_buffer.h"
#include "draco/core/draco_types.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"

namespace draco {

// A single PLY property of a given PLY element. For "vertex" element this can
// contain data such as "x", "y", or "z" coordinate of the vertex, while for
// "face" element this usually contains corner indices.
class PlyProperty {
 public:
  friend class PlyReader;

  PlyProperty(const std::string &name, DataType data_type, DataType list_type);
  void ReserveData(int num_entries) {
    data_.reserve(DataTypeLength(data_type_) * num_entries);
  }

  int64_t GetListEntryOffset(int entry_id) const {
    return list_data_[entry_id * 2];
  }
  int64_t GetListEntryNumValues(int entry_id) const {
    return list_data_[entry_id * 2 + 1];
  }
  const void *GetDataEntryAddress(int entry_id) const {
    return data_.data() + entry_id * data_type_num_bytes_;
  }
  void push_back_value(const void *data) {
    data_.insert(data_.end(), static_cast<const uint8_t *>(data),
                 static_cast<const uint8_t *>(data) + data_type_num_bytes_);
  }

  const std::string &name() const { return name_; }
  bool is_list() const { return list_data_type_ != DT_INVALID; }
  DataType data_type() const { return data_type_; }
  int data_type_num_bytes() const { return data_type_num_bytes_; }
  DataType list_data_type() const { return list_data_type_; }
  int list_data_type_num_bytes() const { return list_data_type_num_bytes_; }

 private:
  std::string name_;
  std::vector<uint8_t> data_;
  // List data contain pairs of <offset, number_of_values>
  std::vector<int64_t> list_data_;
  DataType data_type_;
  int data_type_num_bytes_;
  DataType list_data_type_;
  int list_data_type_num_bytes_;
};

// A single PLY element such as "vertex" or "face". Each element can store
// arbitrary properties such as vertex coordinates or face indices.
class PlyElement {
 public:
  PlyElement(const std::string &name, int64_t num_entries);
  void AddProperty(const PlyProperty &prop) {
    property_index_[prop.name()] = static_cast<int>(properties_.size());
    properties_.emplace_back(prop);
    if (!properties_.back().is_list()) {
      properties_.back().ReserveData(static_cast<int>(num_entries_));
    }
  }

  const PlyProperty *GetPropertyByName(const std::string &name) const {
    const auto it = property_index_.find(name);
    if (it != property_index_.end()) {
      return &properties_[it->second];
    }
    return nullptr;
  }

  int num_properties() const { return static_cast<int>(properties_.size()); }
  int num_entries() const { return static_cast<int>(num_entries_); }
  const PlyProperty &property(int prop_index) const {
    return properties_[prop_index];
  }
  PlyProperty &property(int prop_index) { return properties_[prop_index]; }

 private:
  std::string name_;
  int64_t num_entries_;
  std::vector<PlyProperty> properties_;
  std::map<std::string, int> property_index_;
};

// Class responsible for parsing PLY data. It produces a list of PLY elements
// and their properties that can be used to construct a mesh or a point cloud.
class PlyReader {
 public:
  PlyReader();
  Status Read(DecoderBuffer *buffer);

  const PlyElement *GetElementByName(const std::string &name) const {
    const auto it = element_index_.find(name);
    if (it != element_index_.end()) {
      return &elements_[it->second];
    }
    return nullptr;
  }

  int num_elements() const { return static_cast<int>(elements_.size()); }
  const PlyElement &element(int element_index) const {
    return elements_[element_index];
  }

 private:
  enum Format { kLittleEndian = 0, kAscii };

  Status ParseHeader(DecoderBuffer *buffer);
  StatusOr<bool> ParseEndHeader(DecoderBuffer *buffer);
  bool ParseElement(DecoderBuffer *buffer);
  StatusOr<bool> ParseProperty(DecoderBuffer *buffer);
  bool ParsePropertiesData(DecoderBuffer *buffer);
  bool ParseElementData(DecoderBuffer *buffer, int element_index);
  bool ParseElementDataAscii(DecoderBuffer *buffer, int element_index);

  // Splits |line| by whitespace characters.
  std::vector<std::string> SplitWords(const std::string &line);
  DataType GetDataTypeFromString(const std::string &name) const;

  std::vector<PlyElement> elements_;
  std::map<std::string, int> element_index_;
  Format format_;
};

}  // namespace draco

#endif  // DRACO_IO_PLY_READER_H_
