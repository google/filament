/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMAT_FLATENNER_H
#define TNT_FILAMAT_FLATENNER_H

#include <utils/Panic.h>

#include <map>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <string.h>

using namespace utils;

namespace filamat {

class Flattener {
public:
    explicit Flattener(uint8_t* dst) : mCursor(dst), mStart(dst){}

    static Flattener& getDryRunner() {
        static Flattener dryRunner = Flattener(nullptr);
        dryRunner.mStart = nullptr;
        dryRunner.mCursor = nullptr;
        dryRunner.mOffsetPlaceholders.clear();
        dryRunner.mSizePlaceholders.clear();
        dryRunner.mValuePlaceholders.clear();
        return dryRunner;
    }

    bool isDryRunner() { return mStart == nullptr;}

    size_t getBytesWritten() {
        return mCursor - mStart;
    }

    void writeBool(bool b) {
        if (mStart != nullptr) {
            mCursor[0] = static_cast<uint8_t>(b);
        }
        mCursor += 1;
    }

    void writeUint8(uint8_t i) {
        if (mStart != nullptr) {
            mCursor[0] = i;
        }
        mCursor += 1;
    }

    void writeUint16(uint16_t i) {
        if (mStart != nullptr) {
            mCursor[0] = static_cast<uint8_t>( i        & 0xff);
            mCursor[1] = static_cast<uint8_t>((i >> 8)  & 0xff);
        }
        mCursor += 2;
    }

    void writeUint32(uint32_t i) {
        if (mStart != nullptr) {
            mCursor[0] = static_cast<uint8_t>( i        & 0xff);
            mCursor[1] = static_cast<uint8_t>((i >> 8)  & 0xff);
            mCursor[2] = static_cast<uint8_t>((i >> 16) & 0xff);
            mCursor[3] = static_cast<uint8_t>( i >> 24);
        }
        mCursor += 4;
    }

    void writeUint64(uint64_t i) {
        if (mStart != nullptr) {
            mCursor[0] = static_cast<uint8_t>( i        & 0xff);
            mCursor[1] = static_cast<uint8_t>((i >> 8)  & 0xff);
            mCursor[2] = static_cast<uint8_t>((i >> 16) & 0xff);
            mCursor[3] = static_cast<uint8_t>((i >> 24) & 0xff);
            mCursor[4] = static_cast<uint8_t>((i >> 32) & 0xff);
            mCursor[5] = static_cast<uint8_t>((i >> 40) & 0xff);
            mCursor[6] = static_cast<uint8_t>((i >> 48) & 0xff);
            mCursor[7] = static_cast<uint8_t>((i >> 56) & 0xff);
        }
        mCursor += 8;
    }

    void writeString(const char* str) {
        size_t len = strlen(str);
        if (mStart != nullptr) {
            strcpy(reinterpret_cast<char*>(mCursor), str);
        }
        mCursor += len + 1;
    }

    void writeString(std::string_view str) {
        size_t len = str.length();
        if (mStart != nullptr) {
            memcpy(reinterpret_cast<char*>(mCursor), str.data(), len);
            mCursor[len] = 0;
        }
        mCursor += len + 1;
    }

    void writeBlob(const char* blob, size_t nbytes) {
        writeUint64(nbytes);
        if (mStart != nullptr) {
            memcpy(reinterpret_cast<char*>(mCursor), blob, nbytes);
        }
        mCursor += nbytes;
    }

    void writeSizePlaceholder() {
        mSizePlaceholders.push_back(mCursor);
        if (mStart != nullptr) {
            mCursor[0] = 0;
            mCursor[1] = 0;
            mCursor[2] = 0;
            mCursor[3] = 0;
        }
        mCursor += 4;
    }

    // This writes 0 to 7 (inclusive) zeroes, and the subsequent write is guaranteed to be on a
    // 8-byte boundary. Note that the reader must perform a similar calculation to figure out
    // how many bytes to skip.
    void writeAlignmentPadding() {
        const intptr_t offset = mCursor - mStart;
        const uint8_t padSize = (8 - (offset % 8)) % 8;
        for (uint8_t i = 0; i < padSize; i++) {
            writeUint8(0);
        }
        assert_invariant(0 == ((mCursor - mStart) % 8));
    }

    uint32_t writeSize() {
        assert(mSizePlaceholders.size() > 0);

        uint8_t* dst = mSizePlaceholders.back();
        mSizePlaceholders.pop_back();
        // -4 to account for the 4 bytes we are about to write.
        uint32_t size = static_cast<uint32_t>(mCursor - dst - 4);
        if (mStart != nullptr) {
            dst[0] = static_cast<uint8_t>( size        & 0xff);
            dst[1] = static_cast<uint8_t>((size >>  8) & 0xff);
            dst[2] = static_cast<uint8_t>((size >> 16) & 0xff);
            dst[3] = static_cast<uint8_t>((size >> 24));
        }
        return size;
    }

    void writeOffsetplaceholder(size_t index) {
        mOffsetPlaceholders.insert(std::pair<size_t, uint8_t*>(index, mCursor));
        if (mStart != nullptr) {
            mCursor[0] = 0x0;
            mCursor[1] = 0x0;
            mCursor[2] = 0x0;
            mCursor[3] = 0x0;
        }
        mCursor += 4;
    }

    void writeOffsets(uint32_t forIndex) {
        if (mStart == nullptr) {
            return;
        }

        for(auto pair : mOffsetPlaceholders) {
            size_t index = pair.first;
            if (index != forIndex) {
                continue;
            }
            uint8_t* dst = pair.second;
            size_t offset = mCursor - mOffsetsBase;
            if (offset > UINT32_MAX) {
                slog.e << "Unable to write offset greater than UINT32_MAX." << io::endl;
                exit(0);
            }
            dst[0] = static_cast<uint8_t>( offset & 0xff);
            dst[1] = static_cast<uint8_t>((offset >> 8) & 0xff);
            dst[2] = static_cast<uint8_t>((offset >> 16) & 0xff);
            dst[3] = static_cast<uint8_t>((offset >> 24));
        }
    }

    void writeValuePlaceholder() {
        mValuePlaceholders.push_back(mCursor);
        if (mStart != nullptr) {
            mCursor[0] = 0;
            mCursor[1] = 0;
            mCursor[2] = 0;
            mCursor[3] = 0;
        }
        mCursor += 4;
    }

    void writePlaceHoldValue(size_t v) {
        assert(mValuePlaceholders.size() > 0);

        if (v > UINT32_MAX) {
            slog.e << "Unable to write value greater than UINT32_MAX." << io::endl;
            exit(0);
        }

        uint8_t* dst = mValuePlaceholders.back();
        mValuePlaceholders.pop_back();
        if (mStart != nullptr) {
            dst[0] = static_cast<uint8_t>( v        & 0xff);
            dst[1] = static_cast<uint8_t>((v >>  8) & 0xff);
            dst[2] = static_cast<uint8_t>((v >> 16) & 0xff);
            dst[3] = static_cast<uint8_t>((v >> 24));
        }
    }

    void resetOffsets() {
        mOffsetPlaceholders.clear();
    }

    void markOffsetBase() {
        mOffsetsBase = mCursor;
    }

private:
    uint8_t* mCursor;
    uint8_t* mStart;
    std::vector<uint8_t*> mSizePlaceholders;
    std::multimap<size_t, uint8_t*> mOffsetPlaceholders;
    std::vector<uint8_t*> mValuePlaceholders;
    uint8_t* mOffsetsBase = nullptr;
};

} // namespace filamat
#endif // TNT_FILAMAT_FLATENNER_H
