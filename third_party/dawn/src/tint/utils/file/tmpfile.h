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

#ifndef SRC_TINT_UTILS_FILE_TMPFILE_H_
#define SRC_TINT_UTILS_FILE_TMPFILE_H_

#include <string>

#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// TmpFile constructs a temporary file that can be written to, and is
/// automatically deleted on destruction.
class TmpFile {
  public:
    /// Constructor.
    /// Creates a new temporary file which can be written to.
    /// The temporary file will be automatically deleted on destruction.
    /// @param extension optional file extension to use with the file. The file
    /// have no extension by default.
    explicit TmpFile(std::string extension = "");

    /// Destructor.
    /// Deletes the temporary file.
    ~TmpFile();

    /// @return true if the temporary file was successfully created.
    explicit operator bool() { return !path_.empty(); }

    /// @return the path to the temporary file
    std::string Path() const { return path_; }

    /// Opens the temporary file and appends |size| bytes from |data| to the end
    /// of the temporary file. The temporary file is closed again before
    /// returning, allowing other processes to open the file on operating systems
    /// that require exclusive ownership of opened files.
    /// @param data the data to write to the end of the file
    /// @param size the number of bytes to write from data
    /// @returns true on success, otherwise false
    bool Append(const void* data, size_t size) const;

    /// Appends the argument to the end of the file.
    /// @param data the data to write to the end of the file
    /// @return a reference to this TmpFile
    template <typename T>
    inline TmpFile& operator<<(T&& data) {
        StringStream ss;
        ss << data;
        std::string str = ss.str();
        Append(str.data(), str.size());
        return *this;
    }

  private:
    TmpFile(const TmpFile&) = delete;
    TmpFile& operator=(const TmpFile&) = delete;

    std::string path_;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_FILE_TMPFILE_H_
