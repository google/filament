// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_
#define SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_

#include <memory>
#include <string>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/native/Error.h"
#include "dawn/native/dawn_platform.h"

namespace tint::diag {
class Diagnostic;
class List;
}  // namespace tint::diag

namespace dawn::native {

ResultOrError<uint64_t> CountUTF16CodeUnitsFromUTF8String(const std::string_view& utf8String);

class OwnedCompilationMessages : public NonCopyable {
  public:
    OwnedCompilationMessages();
    ~OwnedCompilationMessages();

    // Adds a message on line 0 (before the first line).
    void AddUnanchoredMessage(
        std::string_view message,
        wgpu::CompilationMessageType type = wgpu::CompilationMessageType::Info);
    // For testing only. Uses the linePos/offset/length for both utf8 and utf16
    // (which is incorrect for non-ASCII strings).
    void AddMessageForTesting(
        std::string_view message,
        wgpu::CompilationMessageType type = wgpu::CompilationMessageType::Info,
        uint64_t lineNum = 0,
        uint64_t linePos = 0,
        uint64_t offset = 0,
        uint64_t length = 0);
    MaybeError AddMessages(const tint::diag::List& diagnostics);
    void ClearMessages();

    const CompilationInfo* GetCompilationInfo();
    const std::vector<std::string>& GetFormattedTintMessages() const;
    bool HasWarningsOrErrors() const;

  private:
    MaybeError AddMessage(const tint::diag::Diagnostic& diagnostic);
    void AddMessage(const CompilationMessage& message);
    void AddFormattedTintMessages(const tint::diag::List& diagnostics);

    MutexProtected<std::optional<CompilationInfo>> mCompilationInfo = std::nullopt;
    std::vector<std::unique_ptr<std::string>> mMessageStrings;
    std::vector<CompilationMessage> mMessages;
    std::vector<DawnCompilationMessageUtf16> mUtf16;
    std::vector<std::string> mFormattedTintMessages;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMPILATIONMESSAGES_H_
