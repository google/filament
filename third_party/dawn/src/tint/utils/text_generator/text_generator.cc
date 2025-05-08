// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/utils/text_generator/text_generator.h"

#include <algorithm>

#include "src/tint/utils/ice/ice.h"

namespace tint {

TextGenerator::TextGenerator() = default;

TextGenerator::~TextGenerator() = default;

TextGenerator::LineWriter::LineWriter(TextBuffer* buf) : buffer(buf) {}

TextGenerator::LineWriter::LineWriter(LineWriter&& other) {
    buffer = other.buffer;
    other.buffer = nullptr;
}

TextGenerator::LineWriter::~LineWriter() {
    if (buffer) {
        buffer->Append(os.str());
    }
}

TextGenerator::TextBuffer::TextBuffer() = default;

TextGenerator::TextBuffer::~TextBuffer() = default;

void TextGenerator::TextBuffer::IncrementIndent() {
    current_indent += 2;
}

void TextGenerator::TextBuffer::DecrementIndent() {
    current_indent = std::max(2u, current_indent) - 2u;
}

void TextGenerator::TextBuffer::Append(const std::string& line) {
    lines.emplace_back(LineInfo{current_indent, line});
}

void TextGenerator::TextBuffer::Insert(const std::string& line, size_t before, uint32_t indent) {
    if (DAWN_UNLIKELY(before > lines.size())) {
        TINT_ICE() << "TextBuffer::Insert() called with before > lines.size()\n"
                   << "  before:" << before << "\n"
                   << "  lines.size(): " << lines.size();
    }
    using DT = decltype(lines)::difference_type;
    lines.insert(lines.begin() + static_cast<DT>(before), LineInfo{indent, line});
}

void TextGenerator::TextBuffer::Append(const TextBuffer& tb) {
    for (auto& line : tb.lines) {
        // TODO(crbug.com/tint/2222): inefficient, consider optimizing
        lines.emplace_back(LineInfo{current_indent + line.indent, line.content});
    }
}

void TextGenerator::TextBuffer::Insert(const TextBuffer& tb, size_t before, uint32_t indent) {
    if (DAWN_UNLIKELY(before > lines.size())) {
        TINT_ICE() << "TextBuffer::Insert() called with before > lines.size()\n"
                   << "  before:" << before << "\n"
                   << "  lines.size(): " << lines.size();
    }
    size_t idx = 0;
    for (auto& line : tb.lines) {
        // TODO(crbug.com/tint/2222): inefficient, consider optimizing
        using DT = decltype(lines)::difference_type;
        lines.insert(lines.begin() + static_cast<DT>(before + idx),
                     LineInfo{indent + line.indent, line.content});
        idx++;
    }
}

std::string TextGenerator::TextBuffer::String(uint32_t indent /* = 0 */) const {
    StringStream ss;
    for (auto& line : lines) {
        if (!line.content.empty()) {
            for (uint32_t i = 0; i < indent + line.indent; i++) {
                ss << " ";
            }
            ss << line.content;
        }
        ss << "\n";
    }
    return ss.str();
}

TextGenerator::ScopedParen::ScopedParen(StringStream& stream) : s(stream) {
    s << "(";
}

TextGenerator::ScopedParen::~ScopedParen() {
    s << ")";
}

TextGenerator::ScopedIndent::ScopedIndent(TextGenerator* generator)
    : ScopedIndent(generator->current_buffer_) {}

TextGenerator::ScopedIndent::ScopedIndent(TextBuffer* buffer) : buffer_(buffer) {
    buffer_->IncrementIndent();
}
TextGenerator::ScopedIndent::~ScopedIndent() {
    buffer_->DecrementIndent();
}

}  // namespace tint
