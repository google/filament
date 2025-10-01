/*
* Copyright (C) 2025 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
 */
#include <utils/Status.h>

namespace utils {
std::string_view Status::getErrorMessage() const {
    const char* ptr = mErrorMessage.c_str();
    return (ptr != nullptr) ? ptr : "";
}

std::ostream& operator<<(std::ostream& os, const Status& status) {
    os << "Status: ";
    switch (status.getCode()) {
        case StatusCode::OK: os << "Ok";
            break;
        case StatusCode::INVALID_ARGUMENT: os << "Invalid argument";
            break;
        case StatusCode::INTERNAL: os << "Internal error";
            break;
    }
    os << ", error message: " << status.getErrorMessage();
    return os;
}
} // namespace utils