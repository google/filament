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

// GEN_BUILD:CONDITION(tint_build_is_linux || tint_build_is_mac)

#include "src/tint/utils/file/tmpfile.h"

#include <unistd.h>
#include <limits>

#include "src/tint/utils/ice/ice.h"

namespace tint {

namespace {

std::string TmpFilePath(std::string ext) {
    char const* dir = getenv("TMPDIR");
    if (dir == nullptr) {
        dir = "/tmp";
    }

    // mkstemps requires an `int` for the file extension name but STL represents
    // size_t. Pre-C++20 there the behavior for unsigned-to-signed conversion
    // (when the source value exceeds the representable range) is implementation
    // defined. While such a large file extension is unlikely in practice, we
    // enforce this here at runtime.
    TINT_ASSERT(ext.length() <= static_cast<size_t>(std::numeric_limits<int>::max()));
    std::string name = std::string(dir) + "/tint_XXXXXX" + ext;
    int file = mkstemps(&name[0], static_cast<int>(ext.length()));
    if (file != -1) {
        close(file);
        return name;
    }
    return "";
}

}  // namespace

TmpFile::TmpFile(std::string extension) : path_(TmpFilePath(std::move(extension))) {}

TmpFile::~TmpFile() {
    if (!path_.empty()) {
        remove(path_.c_str());
    }
}

bool TmpFile::Append(const void* data, size_t size) const {
    if (auto* file = fopen(path_.c_str(), "ab")) {
        fwrite(data, size, 1, file);
        fclose(file);
        return true;
    }
    return false;
}

}  // namespace tint
