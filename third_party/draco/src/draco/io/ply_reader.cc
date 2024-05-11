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
#include "draco/io/ply_reader.h"

#include <array>
#include <regex>

#include "draco/core/status.h"
#include "draco/io/parser_utils.h"
#include "draco/io/ply_property_writer.h"

namespace draco {

PlyProperty::PlyProperty(const std::string &name, DataType data_type,
                         DataType list_type)
    : name_(name), data_type_(data_type), list_data_type_(list_type) {
  data_type_num_bytes_ = DataTypeLength(data_type);
  list_data_type_num_bytes_ = DataTypeLength(list_type);
}

PlyElement::PlyElement(const std::string &name, int64_t num_entries)
    : name_(name), num_entries_(num_entries) {}

PlyReader::PlyReader() : format_(kLittleEndian) {}

Status PlyReader::Read(DecoderBuffer *buffer) {
  std::string value;
  // The first line needs to by "ply".
  if (!parser::ParseString(buffer, &value) || value != "ply") {
    return Status(Status::INVALID_PARAMETER, "Not a valid ply file");
  }
  parser::SkipLine(buffer);

  // The second line needs to be the format of the ply file.
  parser::ParseLine(buffer, &value);
  std::string format, version;
  const std::vector<std::string> words = SplitWords(value);
  if (words.size() >= 3 && words[0] == "format") {
    format = words[1];
    version = words[2];
  } else {
    return Status(Status::INVALID_PARAMETER, "Missing or wrong format line");
  }
  if (version != "1.0") {
    return Status(Status::UNSUPPORTED_VERSION, "Unsupported PLY version");
  }
  if (format == "binary_big_endian") {
    return Status(Status::UNSUPPORTED_VERSION,
                  "Unsupported format. Currently we support only ascii and"
                  " binary_little_endian format.");
  }
  if (format == "ascii") {
    format_ = kAscii;
  } else {
    format_ = kLittleEndian;
  }
  DRACO_RETURN_IF_ERROR(ParseHeader(buffer));
  if (!ParsePropertiesData(buffer)) {
    return Status(Status::INVALID_PARAMETER, "Couldn't parse properties");
  }
  return OkStatus();
}

Status PlyReader::ParseHeader(DecoderBuffer *buffer) {
  while (true) {
    DRACO_ASSIGN_OR_RETURN(bool end, ParseEndHeader(buffer));
    if (end) {
      break;
    }
    if (ParseElement(buffer)) {
      continue;
    }
    DRACO_ASSIGN_OR_RETURN(bool property_parsed, ParseProperty(buffer));
    if (property_parsed) {
      continue;
    }
    parser::SkipLine(buffer);
  }
  return OkStatus();
}

StatusOr<bool> PlyReader::ParseEndHeader(DecoderBuffer *buffer) {
  parser::SkipWhitespace(buffer);
  std::array<char, 10> c;
  if (!buffer->Peek(&c)) {
    return Status(Status::INVALID_PARAMETER,
                  "End of file reached before the end_header");
  }
  if (std::memcmp(&c[0], "end_header", 10) != 0) {
    return false;
  }
  parser::SkipLine(buffer);
  return true;
}

bool PlyReader::ParseElement(DecoderBuffer *buffer) {
  DecoderBuffer line_buffer(*buffer);
  std::string line;
  parser::ParseLine(&line_buffer, &line);

  std::string element_name;
  int64_t count;
  const std::vector<std::string> words = SplitWords(line);
  if (words.size() >= 3 && words[0] == "element") {
    element_name = words[1];
    const std::string count_str = words[2];
    count = strtoll(count_str.c_str(), nullptr, 10);
  } else {
    return false;
  }
  element_index_[element_name] = static_cast<uint32_t>(elements_.size());
  elements_.emplace_back(PlyElement(element_name, count));
  *buffer = line_buffer;
  return true;
}

StatusOr<bool> PlyReader::ParseProperty(DecoderBuffer *buffer) {
  if (elements_.empty()) {
    return false;  // Ignore properties if there is no active element.
  }
  DecoderBuffer line_buffer(*buffer);
  std::string line;
  parser::ParseLine(&line_buffer, &line);

  std::string data_type_str, list_type_str, property_name;
  bool property_search = false;
  const std::vector<std::string> words = SplitWords(line);
  if (words.size() >= 3 && words[0] == "property" && words[1] != "list") {
    property_search = true;
    data_type_str = words[1];
    property_name = words[2];
  }

  bool property_list_search = false;
  if (words.size() >= 5 && words[0] == "property" && words[1] == "list") {
    property_list_search = true;
    list_type_str = words[2];
    data_type_str = words[3];
    property_name = words[4];
  }
  if (!property_search && !property_list_search) {
    return false;
  }
  const DataType data_type = GetDataTypeFromString(data_type_str);
  if (data_type == DT_INVALID) {
    return Status(Status::INVALID_PARAMETER, "Wrong property data type");
  }
  DataType list_type = DT_INVALID;
  if (property_list_search) {
    list_type = GetDataTypeFromString(list_type_str);
    if (list_type == DT_INVALID) {
      return Status(Status::INVALID_PARAMETER, "Wrong property list type");
    }
  }
  elements_.back().AddProperty(
      PlyProperty(property_name, data_type, list_type));
  *buffer = line_buffer;
  return true;
}

bool PlyReader::ParsePropertiesData(DecoderBuffer *buffer) {
  for (int i = 0; i < static_cast<int>(elements_.size()); ++i) {
    if (format_ == kLittleEndian) {
      if (!ParseElementData(buffer, i)) {
        return false;
      }
    } else if (format_ == kAscii) {
      if (!ParseElementDataAscii(buffer, i)) {
        return false;
      }
    }
  }
  return true;
}

bool PlyReader::ParseElementData(DecoderBuffer *buffer, int element_index) {
  PlyElement &element = elements_[element_index];
  for (int entry = 0; entry < element.num_entries(); ++entry) {
    for (int i = 0; i < element.num_properties(); ++i) {
      PlyProperty &prop = element.property(i);
      if (prop.is_list()) {
        // Parse the number of entries for the list element.
        int64_t num_entries = 0;
        buffer->Decode(&num_entries, prop.list_data_type_num_bytes());
        // Store offset to the main data entry.
        prop.list_data_.push_back(prop.data_.size() /
                                  prop.data_type_num_bytes_);
        // Store the number of entries.
        prop.list_data_.push_back(num_entries);
        // Read and store the actual property data
        const int64_t num_bytes_to_read =
            prop.data_type_num_bytes() * num_entries;
        prop.data_.insert(prop.data_.end(), buffer->data_head(),
                          buffer->data_head() + num_bytes_to_read);
        buffer->Advance(num_bytes_to_read);
      } else {
        // Non-list property
        prop.data_.insert(prop.data_.end(), buffer->data_head(),
                          buffer->data_head() + prop.data_type_num_bytes());
        buffer->Advance(prop.data_type_num_bytes());
      }
    }
  }
  return true;
}

bool PlyReader::ParseElementDataAscii(DecoderBuffer *buffer,
                                      int element_index) {
  PlyElement &element = elements_[element_index];
  for (int entry = 0; entry < element.num_entries(); ++entry) {
    for (int i = 0; i < element.num_properties(); ++i) {
      PlyProperty &prop = element.property(i);
      PlyPropertyWriter<double> prop_writer(&prop);
      int32_t num_entries = 1;
      if (prop.is_list()) {
        parser::SkipWhitespace(buffer);
        // Parse the number of entries for the list element.
        if (!parser::ParseSignedInt(buffer, &num_entries)) {
          return false;
        }

        // Store offset to the main data entry.
        prop.list_data_.push_back(prop.data_.size() /
                                  prop.data_type_num_bytes_);
        // Store the number of entries.
        prop.list_data_.push_back(num_entries);
      }
      // Read and store the actual property data.
      for (int v = 0; v < num_entries; ++v) {
        parser::SkipWhitespace(buffer);
        if (prop.data_type() == DT_FLOAT32 || prop.data_type() == DT_FLOAT64) {
          float val;
          if (!parser::ParseFloat(buffer, &val)) {
            return false;
          }
          prop_writer.PushBackValue(val);
        } else {
          int32_t val;
          if (!parser::ParseSignedInt(buffer, &val)) {
            return false;
          }
          prop_writer.PushBackValue(val);
        }
      }
    }
  }
  return true;
}

std::vector<std::string> PlyReader::SplitWords(const std::string &line) {
  std::vector<std::string> output;
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  // Check for isspace chars.
  while ((end = line.find_first_of(" \t\n\v\f\r", start)) !=
         std::string::npos) {
    const std::string word(line.substr(start, end - start));
    if (!std::all_of(word.begin(), word.end(), isspace)) {
      output.push_back(word);
    }
    start = end + 1;
  }

  const std::string last_word(line.substr(start));
  if (!std::all_of(last_word.begin(), last_word.end(), isspace)) {
    output.push_back(last_word);
  }
  return output;
}

DataType PlyReader::GetDataTypeFromString(const std::string &name) const {
  if (name == "char" || name == "int8") {
    return DT_INT8;
  }
  if (name == "uchar" || name == "uint8") {
    return DT_UINT8;
  }
  if (name == "short" || name == "int16") {
    return DT_INT16;
  }
  if (name == "ushort" || name == "uint16") {
    return DT_UINT16;
  }
  if (name == "int" || name == "int32") {
    return DT_INT32;
  }
  if (name == "uint" || name == "uint32") {
    return DT_UINT32;
  }
  if (name == "float" || name == "float32") {
    return DT_FLOAT32;
  }
  if (name == "double" || name == "float64") {
    return DT_FLOAT64;
  }
  return DT_INVALID;
}

}  // namespace draco
