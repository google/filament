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
#ifndef DRACO_IO_PLY_PROPERTY_READER_H_
#define DRACO_IO_PLY_PROPERTY_READER_H_

#include <functional>

#include "draco/io/ply_reader.h"

namespace draco {

// Class for reading PlyProperty with a given type, performing data conversion
// if necessary.
template <typename ReadTypeT>
class PlyPropertyReader {
 public:
  explicit PlyPropertyReader(const PlyProperty *property)
      : property_(property) {
    // Find the suitable function for converting values.
    switch (property->data_type()) {
      case DT_UINT8:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<uint8_t>(val_id);
        };
        break;
      case DT_INT8:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<int8_t>(val_id);
        };
        break;
      case DT_UINT16:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<uint16_t>(val_id);
        };
        break;
      case DT_INT16:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<int16_t>(val_id);
        };
        break;
      case DT_UINT32:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<uint32_t>(val_id);
        };
        break;
      case DT_INT32:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<int32_t>(val_id);
        };
        break;
      case DT_FLOAT32:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<float>(val_id);
        };
        break;
      case DT_FLOAT64:
        convert_value_func_ = [=](int val_id) {
          return this->ConvertValue<double>(val_id);
        };
        break;
      default:
        break;
    }
  }

  ReadTypeT ReadValue(int value_id) const {
    return convert_value_func_(value_id);
  }

 private:
  template <typename SourceTypeT>
  ReadTypeT ConvertValue(int value_id) const {
    const void *const address = property_->GetDataEntryAddress(value_id);
    const SourceTypeT src_val = *reinterpret_cast<const SourceTypeT *>(address);
    return static_cast<ReadTypeT>(src_val);
  }

  const PlyProperty *property_;
  std::function<ReadTypeT(int)> convert_value_func_;
};

}  // namespace draco

#endif  // DRACO_IO_PLY_PROPERTY_READER_H_
