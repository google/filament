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
#ifndef DRACO_IO_PLY_PROPERTY_WRITER_H_
#define DRACO_IO_PLY_PROPERTY_WRITER_H_

#include <functional>

#include "draco/io/ply_reader.h"

namespace draco {

// Class for writing PlyProperty with a given type, performing data conversion
// if necessary.
template <typename WriteTypeT>
class PlyPropertyWriter {
 public:
  explicit PlyPropertyWriter(PlyProperty *property) : property_(property) {
    // Find the suitable function for converting values.
    switch (property->data_type()) {
      case DT_UINT8:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<uint8_t>(val);
        };
        break;
      case DT_INT8:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<int8_t>(val);
        };
        break;
      case DT_UINT16:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<uint16_t>(val);
        };
        break;
      case DT_INT16:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<int16_t>(val);
        };
        break;
      case DT_UINT32:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<uint32_t>(val);
        };
        break;
      case DT_INT32:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<int32_t>(val);
        };
        break;
      case DT_FLOAT32:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<float>(val);
        };
        break;
      case DT_FLOAT64:
        convert_value_func_ = [=](WriteTypeT val) {
          return this->ConvertValue<double>(val);
        };
        break;
      default:
        break;
    }
  }

  void PushBackValue(WriteTypeT value) const {
    return convert_value_func_(value);
  }

 private:
  template <typename SourceTypeT>
  void ConvertValue(WriteTypeT value) const {
    const SourceTypeT src_val = static_cast<SourceTypeT>(value);
    property_->push_back_value(&src_val);
  }

  PlyProperty *property_;
  std::function<void(WriteTypeT)> convert_value_func_;
};

}  // namespace draco

#endif  // DRACO_IO_PLY_PROPERTY_WRITER_H_
