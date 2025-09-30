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

#ifndef TNT_UTILS_STATUS_H
#define TNT_UTILS_STATUS_H

#include <ostream>
#include <string>
#include <string_view>
#include <utils/compiler.h>

namespace utils {

enum class StatusCode {
    OK,
    INVALID_ARGUMENT,
    INTERNAL,
};

// Returns the StatusCode to indicate whether the request was successful.
// If successful, it returns OK with no error message, if not it returns
// other codes with an optional error message.
class UTILS_PUBLIC Status {
public:
    // Default to StatusCode::OK
    Status() : mStatusCode(StatusCode::OK) {}
    Status(StatusCode statusCode, std::string_view errorMessage) :
          mStatusCode(statusCode),
          mErrorMessage(errorMessage) {}

    Status(const Status& other) = default;
    Status(Status&& other) noexcept = default;

    ~Status() = default;

    Status& operator=(const Status& other) = default;
    Status& operator=(Status&& other) noexcept = default;

    bool operator==(const Status& other) const {
        return mStatusCode == other.mStatusCode && mErrorMessage == other.mErrorMessage;
    }

    bool operator!=(const Status& other) const {
        return !(*this == other);
    }

    // Returns if the status is ok.
    bool isOk() const {
        return mStatusCode == StatusCode::OK;
    }

    StatusCode getCode() const {
        return mStatusCode;
    }

    std::string_view getErrorMessage() const {
        return mErrorMessage;
    }

    // Convenient static functions that help construct Status.
    // E.g., Usage `return utils::Status::internal("internal error");`
    static Status ok() {
        return {};
    }

    static Status internal(std::string_view message) {
        return {StatusCode::INTERNAL, message};
    }

    static Status invalidArgument(std::string_view message) {
        return {StatusCode::INVALID_ARGUMENT, message};
    }

    friend std::ostream& operator<<(std::ostream& os, const Status& status) {
        os << "Status: ";
        switch (status.mStatusCode) {
            case StatusCode::OK: os << "Ok";
                break;
            case StatusCode::INVALID_ARGUMENT: os << "Invalid argument";
                break;
            case StatusCode::INTERNAL: os << "Internal error";
                break;
        }
        os << ", error message: " << status.mErrorMessage;
        return os;
    }

private:
    StatusCode mStatusCode;
    // Reason for the error if exists.
    std::string mErrorMessage;
};

} // namespace utils

#endif // TNT_UTILS_STATUS_H
