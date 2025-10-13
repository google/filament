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
    /** The requested operation is not supported. */
    UNSUPPORTED,
};

/**
 * Returns the StatusCode to indicate whether the request was successful.
 * If successful, it returns OK with an optional message, if not it returns
 * other codes with an optional error message.
 */
class UTILS_PUBLIC Status {
public:
    /**
     * Creates a new Status with a StatusCode of OK.
     */
    Status() : mStatusCode(StatusCode::OK) {}

    /**
     * Creates a new Status with the given status code and supplementary message.
     *
     * @param statusCode The status code to use.
     * @param message An optional message, usually contains the reason for the failure.
     */
    Status(StatusCode statusCode, std::string_view message) :
          mStatusCode(statusCode),
          mMessage(message.data(), message.length()) {}

    Status(const Status& other) = default;

    Status(Status&& other) noexcept = default;

    ~Status() = default;

    Status& operator=(const Status& other) = default;
    Status& operator=(Status&& other) noexcept = default;

    bool operator==(const Status& other) const {
        return mStatusCode == other.mStatusCode && mMessage == other.mMessage;
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
     * Returns the message for this Status.
     * @return The message string. Can be empty if it's not set.
     */
    std::string_view getMessage() const {
        return std::string_view(mMessage.begin(), mMessage.length());
    }

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
     * Creates a success Status with a StatusCode of OK with a supplementary message.
     * @return a success Status with a StatusCode of OK with a supplementary message.
     */
    static Status ok(std::string_view message) {
        return {StatusCode::OK, message};
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

    /**
     * Creates an error Status with an UNSUPPORTED status code.
     * @param message The error message to include.
     * @return an error Status with an UNSUPPORTED status code.
     */
    static Status unsupported(std::string_view message) {
        return { StatusCode::UNSUPPORTED, message };
    }

private:
    StatusCode mStatusCode;
    // Additional message for the Status. Usually contains the reason for the error.
    utils::CString mMessage;
};

utils::io::ostream& operator<<(utils::io::ostream& os, const Status& status);

} // namespace utils

#endif // TNT_UTILS_STATUS_H
