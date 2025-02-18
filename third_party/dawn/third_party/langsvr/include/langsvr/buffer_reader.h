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

#ifndef LANGSVR_BUFFER_READER_H_
#define LANGSVR_BUFFER_READER_H_

#include "langsvr/reader.h"

namespace langsvr {

/// BufferReader is an implementation of the Reader interface backed by a buffer.
/// The buffer being read is not copied, and must be kept alive while the BufferReader is in use.
class BufferReader final : public Reader {
  public:
    // Destructor
    ~BufferReader() override;

    /// Constructor
    /// @param string the string to read from
    explicit BufferReader(std::string_view string)
        : data_(reinterpret_cast<const std::byte*>(string.data())),
          bytes_remaining_(string.length()) {}

    /// @copydoc Reader::Read
    size_t Read(std::byte* out, size_t count) override;

  private:
    /// The data to read from
    const std::byte* data_ = nullptr;

    /// The number of bytes remaining
    size_t bytes_remaining_ = 0;
};

}  // namespace langsvr

#endif  // LANGSVR_BUFFER_READER_H_
