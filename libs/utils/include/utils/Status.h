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
#include <string_view>
#include <utils/compiler.h>
#include <utils/CString.h>

namespace utils {

/**
 * A code indicating the success or failure of an operation.
 */
enum class StatusCode {
    /** The operation completed successfully. */
    OK,
    /** The caller provided invalid arguments in the request. */
    INVALID_ARGUMENT,
    /** Internal error was occurred while processing the request. */
    INTERNAL,
};

/**
 * Returns the StatusCode to indicate whether the request was successful.
 * If successful, it returns OK with no error message, if not it returns
 * other codes with an optional error message.
 */
class UTILS_PUBLIC Status {
public:
    /**
     * Creates a new Status with a StatusCode of OK.
     */
    Status() : mStatusCode(StatusCode::OK) {}

    /**
     * Creates a new Status with the given status code and error message.
     *
     * @param statusCode The status code to use.
     * @param errorMessage An optional error message.
     */
    Status(StatusCode statusCode, std::string_view errorMessage) :
          mStatusCode(statusCode),
          mErrorMessage(errorMessage.data(), errorMessage.length()) {}

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

    /**
     * Returns true if the status is OK.
     * @return true if the operation was successful, false otherwise.
     */
    bool isOk() const {
        return mStatusCode == StatusCode::OK;
    }

    /**
     * Returns the StatusCode for this Status.
     * @return the StatusCode for this Status.
     */
    StatusCode getCode() const {
        return mStatusCode;
    }

    /**
     * Returns the error message for this Status.
     * @return The error message string. Will be empty if the status is OK.
     */
    std::string_view getErrorMessage() const;

    /**
     * Convenient factory functions for creating Status objects.
     * Example usage: `return utils::Status::internal("internal error");`
     */

    /**
     * Creates a success Status with a StatusCode of OK.
     * @return a success Status with a StatusCode of OK
     */
    static Status ok() {
        return {};
    }

    /**
     * Creates an error Status with an INTERNAL status code.
     * @param message The error message to include.
     * @return an error Status with an INTERNAL status code.
     */
    static Status internal(std::string_view message) {
        return {StatusCode::INTERNAL, message};
    }

    /**
     * Creates an error Status with an INVALID_ARGUMENT status code.
     * @param message The error message to include.
     * @return an error Status with an INVALID_ARGUMENT status code.
     */
    static Status invalidArgument(std::string_view message) {
        return {StatusCode::INVALID_ARGUMENT, message};
    }

    friend std::ostream& operator<<(std::ostream& os, const Status& status);

private:
    StatusCode mStatusCode;
    // Reason for the error if exists.
    utils::CString mErrorMessage;
};

} // namespace utils

#endif // TNT_UTILS_STATUS_H
