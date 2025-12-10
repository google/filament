/*
* Copyright (C) 2025 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "CompressedStringChunk.h"

#include <zstd.h>

namespace filamat {

namespace {
int toZstdCompressionLevel(CompressedStringChunk::CompressionLevel compressionLevel) {
    switch (compressionLevel) {
        case CompressedStringChunk::CompressionLevel::MIN:
            return ZSTD_minCLevel();
        case CompressedStringChunk::CompressionLevel::MAX:
            return ZSTD_maxCLevel();
        case CompressedStringChunk::CompressionLevel::DEFAULT:
            return ZSTD_defaultCLevel();
    }
}
} // namespace

void CompressedStringChunk::flatten(filamat::Flattener& f) {
    const size_t bufferBound = ZSTD_compressBound(mString.size());
    std::vector<std::uint8_t> compressed(bufferBound);

    const size_t compressedSize = ZSTD_compress(
            compressed.data(), compressed.size(),
            mString.data(), mString.size(),
            toZstdCompressionLevel(mCompressionLevel));

    if (ZSTD_isError(compressedSize)) {
        utils::slog.e << "Error compressing the input string." << utils::io::endl;
        return;
    }

    f.writeBlob((const char*) compressed.data(), compressedSize);
}

} // namespace filamat