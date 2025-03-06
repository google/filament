// Copyright 2022 The Dawn & Tint Authors
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

// turbo-cov is a minimal re-implementation of LLVM's llvm-cov, that emits just
// the per segment coverage in a binary stream. This avoids the overhead of
// encoding to JSON.

#include <cstdio>
#include <utility>

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ProfileData/Coverage/CoverageMapping.h"
#include "llvm/ProfileData/InstrProfReader.h"
#include "llvm/Support/VirtualFileSystem.h"

#if defined(_MSC_VER)
#include <fcntl.h>  // _O_BINARY
#include <io.h>     // _setmode
#endif

namespace {

template <typename T>
void emit(T v) {
    fwrite(&v, sizeof(v), 1, stdout);
}

void emit(const llvm::StringRef& str) {
    uint64_t len = str.size();
    emit<uint32_t>(len);
    fwrite(str.data(), len, 1, stdout);
}

}  // namespace

int main(int argc, const char** argv) {
#if defined(_MSC_VER)
    // Change stdin & stdout from text mode to binary mode.
    // This ensures sequences of \r\n are not changed to \n.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    if (argc < 3) {
        fprintf(stderr, "turbo-cov <exe> <profdata>\n");
        return 1;
    }

    auto exe = argv[1];
    auto profdata = argv[2];
    auto filesystem = llvm::vfs::getRealFileSystem();
    auto res = llvm::coverage::CoverageMapping::load({exe}, profdata, *filesystem);
    if (auto E = res.takeError()) {
        fprintf(stderr, "failed to load executable '%s': %s\n", exe,
                llvm::toString(std::move(E)).c_str());
        return 1;
    }

    auto coverage = std::move(res.get());
    if (!coverage) {
        fprintf(stderr, "failed to load coverage information\n");
        return 1;
    }

    if (auto mismatched = coverage->getMismatchedCount()) {
        fprintf(stderr, "%d functions have mismatched data\n", static_cast<int>(mismatched));
        return 1;
    }

    // uint32 num_files
    //   file[0]
    //     uint32 filename.length
    //     <data> filename.data
    //     uint32 num_segments
    //       file[0].segment[0]
    //         uint32 line
    //         uint32 col
    //         uint32 count
    //         uint8  hasCount
    //       file[0].segment[1]
    //         ...
    //   file[1]
    //     ...

    auto files = coverage->getUniqueSourceFiles();
    emit<uint32_t>(files.size());
    for (auto& file : files) {
        emit(file);
        auto fileCoverage = coverage->getCoverageForFile(file);
        emit<uint32_t>(fileCoverage.end() - fileCoverage.begin());
        for (auto& segment : fileCoverage) {
            emit<uint32_t>(segment.Line);
            emit<uint32_t>(segment.Col);
            emit<uint32_t>(segment.Count);
            emit<uint8_t>(segment.HasCount ? 1 : 0);
        }
    }

    return 0;
}
