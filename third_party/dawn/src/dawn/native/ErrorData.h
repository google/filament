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

#ifndef SRC_DAWN_NATIVE_ERRORDATA_H_
#define SRC_DAWN_NATIVE_ERRORDATA_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "dawn/common/Compiler.h"

namespace wgpu {
enum class ErrorType : uint32_t;
}

namespace dawn {
using ErrorType = wgpu::ErrorType;
}

namespace dawn::native {
enum class InternalErrorType : uint32_t;

class [[nodiscard]] ErrorData {
  public:
    [[nodiscard]] static std::unique_ptr<ErrorData> Create(InternalErrorType type,
                                                           std::string message,
                                                           const char* file,
                                                           const char* function,
                                                           int line);
    ErrorData(InternalErrorType type, std::string message);
    ~ErrorData();

    struct BacktraceRecord {
        const char* file;
        const char* function;
        int line;
    };
    void AppendBacktrace(const char* file, const char* function, int line);
    void AppendContext(std::string context);
    template <typename... Args>
    void AppendContext(const char* formatStr, const Args&... args) {
        std::string out;
        absl::UntypedFormatSpec format(formatStr);
        if (absl::FormatUntyped(&out, format, {absl::FormatArg(args)...})) {
            AppendContext(std::move(out));
        } else {
            AppendContext(absl::StrFormat("[Failed to format error: \"%s\"]", formatStr));
        }
    }
    void AppendDebugGroup(std::string_view label);
    void AppendBackendMessage(std::string message);
    template <typename... Args>
    void AppendBackendMessage(const char* formatStr, const Args&... args) {
        std::string out;
        absl::UntypedFormatSpec format(formatStr);
        if (absl::FormatUntyped(&out, format, {absl::FormatArg(args)...})) {
            AppendBackendMessage(std::move(out));
        } else {
            AppendBackendMessage(absl::StrFormat("[Failed to format error: \"%s\"]", formatStr));
        }
    }

    InternalErrorType GetType() const;
    const std::string& GetMessage() const;
    const std::vector<BacktraceRecord>& GetBacktrace() const;
    const std::vector<std::string>& GetContexts() const;
    const std::vector<std::string>& GetDebugGroups() const;
    const std::vector<std::string>& GetBackendMessages() const;

    std::string GetFormattedMessage() const;

  private:
    InternalErrorType mType;
    std::string mMessage;
    std::vector<BacktraceRecord> mBacktrace;
    std::vector<std::string> mContexts;
    std::vector<std::string> mDebugGroups;
    std::vector<std::string> mBackendMessages;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERRORDATA_H_
