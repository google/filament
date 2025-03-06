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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_FAIL_STREAM_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_FAIL_STREAM_H_

#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {

/// A FailStream object accumulates values onto a given stream,
/// and can be used to record failure by writing the false value
/// to given a pointer-to-bool.
class FailStream {
  public:
    /// Creates a new fail stream
    /// @param status_ptr where we will write false to indicate failure. Assumed
    /// to be a valid pointer to bool.
    /// @param out output stream where a message should be written to explain
    /// the failure
    FailStream(bool* status_ptr, StringStream* out) : status_ptr_(status_ptr), out_(out) {}
    /// Copy constructor
    /// @param other the fail stream to clone
    FailStream(const FailStream& other) = default;

    /// Converts to a boolean status. A true result indicates success,
    /// and a false result indicates failure.
    /// @returns the status
    operator bool() const { return *status_ptr_; }
    /// Returns the current status value.  This can be more readable
    /// the conversion operator.
    /// @returns the status
    bool status() const { return *status_ptr_; }

    /// Records failure.
    /// @returns a FailStream
    FailStream& Fail() {
        *status_ptr_ = false;
        return *this;
    }

    /// Appends the given value to the message output stream.
    /// @param val the value to write to the output stream.
    /// @returns this object
    template <typename T>
    FailStream& operator<<(const T& val) {
        *out_ << val;
        return *this;
    }

  private:
    bool* status_ptr_;
    StringStream* out_;
};

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_FAIL_STREAM_H_
