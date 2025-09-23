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

#include "dawn/native/CompilationMessages.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/dawn_platform.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

wgpu::CompilationMessageType TintSeverityToMessageType(tint::diag::Severity severity) {
    switch (severity) {
        case tint::diag::Severity::Note:
            return wgpu::CompilationMessageType::Info;
        case tint::diag::Severity::Warning:
            return wgpu::CompilationMessageType::Warning;
        default:
            return wgpu::CompilationMessageType::Error;
    }
}

}  // anonymous namespace

ResultOrError<uint64_t> CountUTF16CodeUnitsFromUTF8String(const std::string_view& utf8String) {
    if (tint::utf8::IsASCII(utf8String)) {
        return utf8String.size();
    }

    uint64_t numberOfUTF16CodeUnits = 0;
    std::string_view remaining = utf8String;
    while (!remaining.empty()) {
        auto [codePoint, utf8CharacterByteLength] = tint::utf8::Decode(remaining);
        // Directly return as something wrong has happened during the UTF-8 decoding.
        if (utf8CharacterByteLength == 0) {
            return DAWN_INTERNAL_ERROR("Fail to decode the unicode string");
        }
        remaining = remaining.substr(utf8CharacterByteLength);

        // Count the number of code units in UTF-16. See https://en.wikipedia.org/wiki/UTF-16 for
        // more details.
        if (codePoint.value <= 0xD7FF || (codePoint.value >= 0xE000 && codePoint.value <= 0xFFFF)) {
            // Code points from U+0000 to U+D7FF and U+E000 to U+FFFF are encoded as single 16-bit
            // code units.
            ++numberOfUTF16CodeUnits;
        } else if (codePoint.value >= 0x10000) {
            // Code points from U+010000 to U+10FFFF are encoded as two 16-bit code units.
            numberOfUTF16CodeUnits += 2;
        } else {
            // UTF-16 cannot encode the code points from U+D800 to U+DFFF.
            return DAWN_INTERNAL_ERROR("The unicode string contains illegal unicode code point.");
        }
    }

    return numberOfUTF16CodeUnits;
}

void ParsedCompilationMessages::AddUnanchoredMessage(std::string_view message,
                                                     wgpu::CompilationMessageType type) {
    CompilationMessageContent m = {};
    m.message = message;
    m.type = type;
    AddMessage(std::move(m));
}

void ParsedCompilationMessages::AddMessageForTesting(std::string_view message,
                                                     wgpu::CompilationMessageType type,
                                                     uint64_t lineNum,
                                                     uint64_t linePos,
                                                     uint64_t offset,
                                                     uint64_t length) {
    AddMessage({{
        .message = std::string(message),
        .type = type,
        .lineNum = lineNum,
        .linePosInBytes = linePos,
        .offsetInBytes = offset,
        .lengthInBytes = length,
        // Incorrect for non-ACSII strings
        .linePosInUTF16 = linePos,
        .offsetInUTF16 = offset,
        .lengthInUTF16 = length,
    }});
}

MaybeError ParsedCompilationMessages::AddMessage(const tint::diag::Diagnostic& diagnostic) {
    // Tint line and column values are 1-based.
    uint64_t lineNum = diagnostic.source.range.begin.line;
    uint64_t linePosInBytes = diagnostic.source.range.begin.column;
    // The offset is 0-based.
    uint64_t offsetInBytes = 0;
    uint64_t lengthInBytes = 0;
    uint64_t linePosInUTF16 = 0;
    uint64_t offsetInUTF16 = 0;
    uint64_t lengthInUTF16 = 0;

    if (lineNum && linePosInBytes && diagnostic.source.file) {
        const tint::Source::FileContent& content = diagnostic.source.file->content;

        // Tint stores line as std::string_view in a complete source std::string that's in the
        // source file. So to get the offset in bytes of a line we just need to substract its start
        // pointer with the start of the file's content. Note that line numbering in Tint source
        // range starts at 1 while the array of lines start at 0 (hence the -1).
        const char* fileStart = content.data.data();
        const char* lineStart = content.lines[lineNum - 1].data();
        offsetInBytes = static_cast<uint64_t>(lineStart - fileStart) + linePosInBytes - 1;

        // The linePosInBytes is 1-based.
        uint64_t linePosOffsetInUTF16 = 0;
        DAWN_TRY_ASSIGN(linePosOffsetInUTF16, CountUTF16CodeUnitsFromUTF8String(
                                                  std::string_view(lineStart, linePosInBytes - 1)));
        linePosInUTF16 = linePosOffsetInUTF16 + 1;

        // The offset is 0-based.
        uint64_t lineStartToFileStartOffsetInUTF16 = 0;
        DAWN_TRY_ASSIGN(lineStartToFileStartOffsetInUTF16,
                        CountUTF16CodeUnitsFromUTF8String(std::string_view(
                            fileStart, static_cast<uint64_t>(lineStart - fileStart))));
        offsetInUTF16 = lineStartToFileStartOffsetInUTF16 + linePosInUTF16 - 1;

        // If the range has a valid start but the end is not specified, clamp it to the start.
        uint64_t endLineNum = diagnostic.source.range.end.line;
        uint64_t endLineCol = diagnostic.source.range.end.column;
        if (endLineNum == 0 || endLineCol == 0) {
            endLineNum = lineNum;
            endLineCol = linePosInBytes;
        }

        const char* endLineStart = content.lines[endLineNum - 1].data();
        uint64_t endOffsetInBytes =
            static_cast<uint64_t>(endLineStart - fileStart) + endLineCol - 1;
        // The length of the message is the difference between the starting offset and the
        // ending offset. Negative ranges aren't allowed.
        DAWN_ASSERT(endOffsetInBytes >= offsetInBytes);
        lengthInBytes = endOffsetInBytes - offsetInBytes;
        DAWN_TRY_ASSIGN(lengthInUTF16, CountUTF16CodeUnitsFromUTF8String(std::string_view(
                                           fileStart + offsetInBytes, lengthInBytes)));
    }

    std::string plainMessage = diagnostic.message.Plain();

    AddMessage({{
        .message = plainMessage,
        .type = TintSeverityToMessageType(diagnostic.severity),
        .lineNum = lineNum,
        .linePosInBytes = linePosInBytes,
        .offsetInBytes = offsetInBytes,
        .lengthInBytes = lengthInBytes,
        .linePosInUTF16 = linePosInUTF16,
        .offsetInUTF16 = offsetInUTF16,
        .lengthInUTF16 = lengthInUTF16,
    }});

    return {};
}

void ParsedCompilationMessages::AddMessage(CompilationMessageContent&& message) {
    messages.push_back(message);
}

MaybeError ParsedCompilationMessages::AddMessages(const tint::diag::List& diagnostics) {
    for (const auto& diag : diagnostics) {
        DAWN_TRY(AddMessage(diag));
    }

    AddFormattedTintMessages(diagnostics);

    return {};
}

void ParsedCompilationMessages::AddFormattedTintMessages(const tint::diag::List& diagnostics) {
    tint::diag::List messageList;
    size_t warningCount = 0;
    size_t errorCount = 0;
    for (auto& diag : diagnostics) {
        switch (diag.severity) {
            case tint::diag::Severity::Error: {
                errorCount++;
                messageList.Add(diag);
                break;
            }
            case tint::diag::Severity::Warning: {
                warningCount++;
                messageList.Add(diag);
                break;
            }
            case tint::diag::Severity::Note: {
                messageList.Add(diag);
                break;
            }
            default:
                break;
        }
    }
    if (errorCount == 0 && warningCount == 0) {
        return;
    }
    tint::diag::Formatter::Style style;
    style.print_newline_at_end = false;
    std::ostringstream t;
    if (errorCount > 0) {
        t << errorCount << " error(s) ";
        if (warningCount > 0) {
            t << "and ";
        }
    }
    if (warningCount > 0) {
        t << warningCount << " warning(s) ";
    }
    t << "generated while compiling the shader:\n"
      << tint::diag::Formatter{style}.Format(messageList).Plain();
    formattedTintMessages.push_back(t.str());
}

OwnedCompilationMessages::OwnedCompilationMessages(
    ParsedCompilationMessages&& parsedCompilationMessagges)
    : mMessageContents(parsedCompilationMessagges), mCompilationInfo() {
    // Reserve the size of messages to avoid reallocations, so that we can use
    // the raw pointer when adding each message.
    mMessagesList.reserve(mMessageContents.messages.size());
    mUtf16Messages.reserve(mMessageContents.messages.size());

    // Build the CompilationMessage and DawnCompilationMessageUtf16 object for each message.
    for (auto& m : mMessageContents.messages) {
        if (m.type == wgpu::CompilationMessageType::Error ||
            m.type == wgpu::CompilationMessageType::Warning) {
            mHasWarningsOrErrors = true;
        }

        DawnCompilationMessageUtf16& utf16 = mUtf16Messages.emplace_back();
        utf16.linePos = m.linePosInUTF16;
        utf16.offset = m.offsetInUTF16;
        utf16.length = m.lengthInUTF16;

        CompilationMessage& message = mMessagesList.emplace_back();
        message.message = ToOutputStringView(m.message);
        message.type = m.type;
        message.lineNum = m.lineNum;
        message.linePos = m.linePosInBytes;
        message.offset = m.offsetInBytes;
        message.length = m.lengthInBytes;
        // Points to the created DawnCompilationMessageUtf16, pointers would keep valid since
        // mUtf16Messages has been reserved to the size of messages.
        message.nextInChain = &utf16;
    }

    mCompilationInfo.messageCount = mMessagesList.size();
    mCompilationInfo.messages = mMessagesList.data();
}

const CompilationInfo* OwnedCompilationMessages::GetCompilationInfo() const {
    return &mCompilationInfo;
}

const std::vector<std::string>& OwnedCompilationMessages::GetFormattedTintMessages() const {
    return mMessageContents.formattedTintMessages;
}

bool OwnedCompilationMessages::HasWarningsOrErrors() const {
    return mHasWarningsOrErrors;
}

}  // namespace dawn::native
