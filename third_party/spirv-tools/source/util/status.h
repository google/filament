// Copyright (c) 2025 Google LLC
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

#ifndef SOURCE_UTIL_STATUS_H_
#define SOURCE_UTIL_STATUS_H_

namespace spvtools {
namespace utils {

// The result of processing a module.
enum class Status {
  Failure = 0x0,
  SuccessWithChange = 0x10,
  SuccessWithoutChange = 0x11
};

}  // namespace utils
}  // namespace spvtools

#endif  // SOURCE_UTIL_STATUS_H_
