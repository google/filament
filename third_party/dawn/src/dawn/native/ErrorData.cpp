// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/ErrorData.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

std::unique_ptr<ErrorData> ErrorData::Create(InternalErrorType type,
                                             std::string message,
                                             const char* file,
                                             const char* function,
                                             int line) {
    std::unique_ptr<ErrorData> error = std::make_unique<ErrorData>(type, std::move(message));
    error->AppendBacktrace(file, function, line);

    auto [var, present] = GetEnvironmentVar("DAWN_DEBUG_BREAK_ON_ERROR");
    if (present && !var.empty() && var != "0") {
        ErrorLog() << error->GetMessage();
        BreakPoint();
    }
    return error;
}

ErrorData::ErrorData(InternalErrorType type, std::string message)
    : mType(type), mMessage(std::move(message)) {}

ErrorData::~ErrorData() = default;

void ErrorData::AppendBacktrace(const char* file, const char* function, int line) {
    BacktraceRecord record;
    record.file = file;
    record.function = function;
    record.line = line;

    mBacktrace.push_back(std::move(record));
}

void ErrorData::AppendContext(std::string context) {
    mContexts.push_back(std::move(context));
}

void ErrorData::AppendDebugGroup(std::string_view label) {
    mDebugGroups.push_back(std::string(label));
}

void ErrorData::AppendBackendMessage(std::string message) {
    mBackendMessages.push_back(std::move(message));
}

InternalErrorType ErrorData::GetType() const {
    return mType;
}

const std::string& ErrorData::GetMessage() const {
    return mMessage;
}

const std::vector<ErrorData::BacktraceRecord>& ErrorData::GetBacktrace() const {
    return mBacktrace;
}

const std::vector<std::string>& ErrorData::GetContexts() const {
    return mContexts;
}

const std::vector<std::string>& ErrorData::GetDebugGroups() const {
    return mDebugGroups;
}

const std::vector<std::string>& ErrorData::GetBackendMessages() const {
    return mBackendMessages;
}

std::string ErrorData::GetFormattedMessage() const {
    std::ostringstream ss;
    ss << mMessage << "\n";

    if (!mContexts.empty()) {
        for (auto context : mContexts) {
            ss << " - While " << context << "\n";
        }
    }

    // For non-validation errors, or errors that lack a context include the
    // stack trace for debugging purposes.
    if (mContexts.empty() || mType != InternalErrorType::Validation) {
        for (const auto& callsite : mBacktrace) {
            ss << "    at " << callsite.function << " (" << callsite.file << ":" << callsite.line
               << ")\n";
        }
    }

    if (!mDebugGroups.empty()) {
        ss << "\nDebug group stack:\n";
        for (auto label : mDebugGroups) {
            ss << " > \"" << label << "\"\n";
        }
    }

    if (!mBackendMessages.empty()) {
        ss << "\nBackend messages:\n";
        for (auto message : mBackendMessages) {
            ss << " * " << message << "\n";
        }
    }

    return ss.str();
}

}  // namespace dawn::native
