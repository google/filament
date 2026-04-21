// Copyright 2022 The Draco Authors.
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
#include "draco/core/status.h"

#include <string>

namespace draco {

std::string Status::code_string() const {
  switch (code_) {
    case Code::OK:
      return "OK";
    case Code::DRACO_ERROR:
      return "DRACO_ERROR";
    case Code::IO_ERROR:
      return "IO_ERROR";
    case Code::INVALID_PARAMETER:
      return "INVALID_PARAMETER";
    case Code::UNSUPPORTED_VERSION:
      return "UNSUPPORTED_VERSION";
    case Code::UNKNOWN_VERSION:
      return "UNKNOWN_VERSION";
    case Code::UNSUPPORTED_FEATURE:
      return "UNSUPPORTED_FEATURE";
  }
  return "UNKNOWN_STATUS_VALUE";
}

std::string Status::code_and_error_string() const {
  return code_string() + ": " + error_msg_string();
}

}  // namespace draco
