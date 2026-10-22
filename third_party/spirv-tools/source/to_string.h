// Copyright (c) 2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_TO_STRING_H_
#define SOURCE_TO_STRING_H_

#include <cstdint>
#include <string>

namespace spvtools {

// Returns the decimal representation of a number as a string,
// without using the locale.
std::string to_string(uint32_t n);

}  // namespace spvtools

#endif  // SOURCE_TO_STRING_H_
