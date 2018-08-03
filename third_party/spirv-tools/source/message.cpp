// Copyright (c) 2016 Google Inc.
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

#include "message.h"

#include <sstream>

namespace spvtools {

std::string StringifyMessage(spv_message_level_t level, const char* source,
                             const spv_position_t& position,
                             const char* message) {
  const char* level_string = nullptr;
  switch (level) {
    case SPV_MSG_FATAL:
      level_string = "fatal";
      break;
    case SPV_MSG_INTERNAL_ERROR:
      level_string = "internal error";
      break;
    case SPV_MSG_ERROR:
      level_string = "error";
      break;
    case SPV_MSG_WARNING:
      level_string = "warning";
      break;
    case SPV_MSG_INFO:
      level_string = "info";
      break;
    case SPV_MSG_DEBUG:
      level_string = "debug";
      break;
  }
  std::ostringstream oss;
  oss << level_string << ": ";
  if (source) oss << source << ":";
  oss << position.line << ":" << position.column << ":";
  oss << position.index << ": ";
  if (message) oss << message;
  return oss.str();
}

}  // namespace spvtools
