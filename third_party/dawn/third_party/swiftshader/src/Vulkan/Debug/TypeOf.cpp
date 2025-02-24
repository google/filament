// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "TypeOf.hpp"

namespace vk {
namespace dbg {

std::string TypeOf<bool>::name = "bool";
std::string TypeOf<uint8_t>::name = "uint8_t";
std::string TypeOf<int8_t>::name = "int8_t";
std::string TypeOf<uint16_t>::name = "uint16_t";
std::string TypeOf<int16_t>::name = "int16_t";
std::string TypeOf<float>::name = "float";
std::string TypeOf<uint32_t>::name = "uint32_t";
std::string TypeOf<int32_t>::name = "int32_t";
std::string TypeOf<double>::name = "double";
std::string TypeOf<uint64_t>::name = "uint64_t";
std::string TypeOf<int64_t>::name = "int64_t";

}  // namespace dbg
}  // namespace vk
