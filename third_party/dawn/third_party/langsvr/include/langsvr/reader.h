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

#ifndef LANGSVR_READER_H_
#define LANGSVR_READER_H_

#include <cstdint>
#include <string>

#include "langsvr/result.h"

namespace langsvr {

/// A binary stream reader interface
class Reader {
  public:
    /// Destructor
    virtual ~Reader();

    /// Read reads bytes from the stream, blocking until there are @p count bytes available, or the
    /// end of the stream has been reached.
    /// @param out a pointer to the byte buffer that will be filled with the read data. Must be at
    /// least @p count size.
    /// @param count the number of bytes to read. Must be greater than 0.
    /// @returns the number of bytes read from the stream. If Read() returns less than @p count,
    /// then the end of the stream has been reached.
    virtual size_t Read(std::byte* out, size_t count) = 0;

    /// Reads a string of @p len bytes from the stream.
    /// If there are too few bytes remaining in the stream, then a failure is returned.
    /// @param len the length of the returned string in bytes
    /// @return the deserialized string
    Result<std::string> String(size_t len) {
        static_assert(sizeof(std::byte) == sizeof(char), "length needs calculation");
        std::string out;
        out.resize(len);
        if (size_t n = Read(reinterpret_cast<std::byte*>(out.data()), len); n != len) {
            return Failure{"EOF"};
        }
        return out;
    }
};

}  // namespace langsvr

#endif  // LANGSVR_READER_H_
