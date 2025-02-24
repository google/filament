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

#ifndef VK_DEBUG_TYPE_HPP_
#define VK_DEBUG_TYPE_HPP_

#include <memory>

#include <cstdint>
#include <string>

namespace vk {
namespace dbg {

// clang-format off
template <typename T> struct TypeOf;
template <> struct TypeOf<bool>     { static std::string name; };
template <> struct TypeOf<uint8_t>  { static std::string name; };
template <> struct TypeOf<int8_t>   { static std::string name; };
template <> struct TypeOf<uint16_t> { static std::string name; };
template <> struct TypeOf<int16_t>  { static std::string name; };
template <> struct TypeOf<float>    { static std::string name; };
template <> struct TypeOf<uint32_t> { static std::string name; };
template <> struct TypeOf<int32_t>  { static std::string name; };
template <> struct TypeOf<double>   { static std::string name; };
template <> struct TypeOf<uint64_t> { static std::string name; };
template <> struct TypeOf<int64_t>  { static std::string name; };
// clang-format on

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_TYPE_HPP_
