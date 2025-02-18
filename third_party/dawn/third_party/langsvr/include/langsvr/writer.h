// Copyright 2024 The langsvr Authors
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

#ifndef LANGSVR_WRITER_H_
#define LANGSVR_WRITER_H_

#include <cstddef>
#include <string>

#include "langsvr/result.h"

namespace langsvr {

/// A binary stream writer interface
class Writer {
  public:
    /// Destructor
    virtual ~Writer();

    /// Write writes bytes to the stream, blocking until the write has finished.
    /// @param in the byte data to write to the stream
    /// @param count the number of bytes to writer. Must be greater than 0
    /// @returns the result of the write
    virtual Result<SuccessType> Write(const std::byte* in, size_t count) = 0;

    /// Writes a string of @p len bytes from the stream.
    /// @param value the string to write
    /// @returns the result of the write
    Result<SuccessType> String(std::string_view value) {
        static_assert(sizeof(std::byte) == sizeof(char), "length needs calculation");
        return Write(reinterpret_cast<const std::byte*>(value.data()), value.length());
    }
};

}  // namespace langsvr

#endif  // LANGSVR_WRITER_H_
