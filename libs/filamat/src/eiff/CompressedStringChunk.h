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

#ifndef TNT_COMPRESSEDSTRINGCHUNK_H
#define TNT_COMPRESSEDSTRINGCHUNK_H

#include "Chunk.h"
#include "Flattener.h"

namespace filamat {

class CompressedStringChunk final : public Chunk {
public:
    enum class Compression { NONE, COMPRESSED };

    CompressedStringChunk(ChunkType type, Compression compression, std::string_view string)
            : Chunk(type), mString(string) {}
    ~CompressedStringChunk() = default;

private:
    void flatten(Flattener& f) override;

    Compression mCompression;
    std::string_view mString;
};

} // namespace filamat


#endif // TNT_COMPRESSEDSTRINGCHUNK_H
