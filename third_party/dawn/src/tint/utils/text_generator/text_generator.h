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

#ifndef SRC_TINT_UTILS_TEXT_GENERATOR_TEXT_GENERATOR_H_
#define SRC_TINT_UTILS_TEXT_GENERATOR_TEXT_GENERATOR_H_

#include <string>
#include <utility>
#include <vector>

#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// Helper methods for generators which are creating text output
class TextGenerator {
  public:
    /// LineInfo holds a single line of text
    struct LineInfo {
        /// The indentation of the line in blankspace
        uint32_t indent = 0;
        /// The content of the line, without a trailing newline character
        std::string content;
    };

    /// TextBuffer holds a list of lines of text.
    struct TextBuffer {
        // Constructor
        TextBuffer();

        // Destructor
        ~TextBuffer();

        /// IncrementIndent increases the indentation of lines that will be written
        /// to the TextBuffer
        void IncrementIndent();

        /// DecrementIndent decreases the indentation of lines that will be written
        /// to the TextBuffer
        void DecrementIndent();

        /// Appends the line to the end of the TextBuffer
        /// @param line the line to append to the TextBuffer
        void Append(const std::string& line);

        /// Inserts the line to the TextBuffer before the line with index `before`
        /// @param line the line to append to the TextBuffer
        /// @param before the zero-based index of the line to insert the text before
        /// @param indent the indentation to apply to the inserted lines
        void Insert(const std::string& line, size_t before, uint32_t indent);

        /// Appends the lines of `tb` to the end of this TextBuffer
        /// @param tb the TextBuffer to append to the end of this TextBuffer
        void Append(const TextBuffer& tb);

        /// Inserts the lines of `tb` to the TextBuffer before the line with index
        /// `before`
        /// @param tb the TextBuffer to insert into this TextBuffer
        /// @param before the zero-based index of the line to insert the text before
        /// @param indent the indentation to apply to the inserted lines
        void Insert(const TextBuffer& tb, size_t before, uint32_t indent);

        /// @returns the buffer's content as a single string
        /// @param indent additional indentation to apply to each line
        std::string String(uint32_t indent = 0) const;

        /// The current indentation of the TextBuffer. Lines appended to the
        /// TextBuffer will use this indentation.
        uint32_t current_indent = 0;

        /// The lines
        std::vector<LineInfo> lines;
    };
    /// LineWriter is a helper that acts as a string buffer, who's content is
    /// emitted to the TextBuffer as a single line on destruction.
    struct LineWriter {
      public:
        /// Constructor
        /// @param buffer the TextBuffer that the LineWriter will append its
        /// content to on destruction, at the end of the buffer.
        explicit LineWriter(TextBuffer* buffer);

        /// Move constructor
        /// @param rhs the LineWriter to move
        LineWriter(LineWriter&& rhs);
        /// Destructor
        ~LineWriter();

        /// @returns the StringStream
        operator StringStream&() { return os; }

        /// @param rhs the value to write to the line
        /// @returns the StringStream so calls can be chained
        template <typename T>
        StringStream& operator<<(T&& rhs) {
            return os << std::forward<T>(rhs);
        }

      private:
        LineWriter(const LineWriter&) = delete;
        LineWriter& operator=(const LineWriter&) = delete;

        StringStream os;
        TextBuffer* buffer;
    };

    /// Increment the emitter indent level
    void IncrementIndent() { current_buffer_->IncrementIndent(); }
    /// Decrement the emitter indent level
    void DecrementIndent() { current_buffer_->DecrementIndent(); }

    /// @returns a new LineWriter, used for buffering and writing a line to
    /// the end of #current_buffer_.
    LineWriter Line() { return LineWriter(current_buffer_); }
    /// @param buffer the TextBuffer to write the line to
    /// @returns a new LineWriter, used for buffering and writing a line to
    /// the end of `buffer`.
    static LineWriter Line(TextBuffer* buffer) { return LineWriter(buffer); }

    /// @returns the result data
    virtual std::string Result() const { return main_buffer_.String(); }

    /// @returns the list of diagnostics raised by the generator.
    const diag::List& Diagnostics() const { return diagnostics_; }

  protected:
    /// Helper for writing a '(' on construction and a ')' destruction.
    struct ScopedParen {
        /// Constructor
        /// @param stream the StringStream that will be written to
        explicit ScopedParen(StringStream& stream);
        /// Destructor
        ~ScopedParen();

      private:
        ScopedParen(ScopedParen&& rhs) = delete;
        ScopedParen(const ScopedParen&) = delete;
        ScopedParen& operator=(const ScopedParen&) = delete;
        StringStream& s;
    };

    /// Helper for incrementing indentation on construction and decrementing
    /// indentation on destruction.
    struct ScopedIndent {
        /// Constructor
        /// @param buffer the TextBuffer that the ScopedIndent will indent
        explicit ScopedIndent(TextBuffer* buffer);
        /// Constructor
        /// @param generator ScopedIndent will indent the generator's
        /// `current_buffer_`
        explicit ScopedIndent(TextGenerator* generator);
        /// Destructor
        ~ScopedIndent();

      private:
        ScopedIndent(ScopedIndent&& rhs) = delete;
        ScopedIndent(const ScopedIndent&) = delete;
        ScopedIndent& operator=(const ScopedIndent&) = delete;
        TextBuffer* buffer_;
    };

    /// Constructor
    TextGenerator();
    virtual ~TextGenerator();

    /// Diagnostics generated by the generator
    diag::List diagnostics_;
    /// The buffer the TextGenerator is currently appending lines to
    TextBuffer* current_buffer_ = &main_buffer_;

    /// The primary text buffer that the generator will emit
    TextBuffer main_buffer_;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_GENERATOR_TEXT_GENERATOR_H_
