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

#ifndef TNT_FILAFLAT_UNFLATTENER_H
#define TNT_FILAFLAT_UNFLATTENER_H

#include <filaflat/ChunkContainer.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <stdint.h>

namespace filaflat {

// Allow read operation from an Unflattenable. All READ operation MUST go through the Unflattener
// since it checks boundaries before readind. All read operations return values MUST be verified,
// never assume a read will succeed.
class UTILS_PUBLIC Unflattener {
public:
    Unflattener(const uint8_t* src, const uint8_t* end)
            : mSrc(src), mCursor(src), mEnd(end) {
    }

    Unflattener(ChunkContainer const& container, filamat::ChunkType type)
            : mSrc(container.getChunkStart(type)),
              mCursor(mSrc),
              mEnd(container.getChunkEnd(type)) {
    }

    ~Unflattener() = default;

    bool hasData() const noexcept {
        return mCursor < mEnd;
    }

    inline bool willOverflow(size_t size) const noexcept {
        return (mCursor + size) >  mEnd;
    }

    bool read(bool* b) noexcept {
        if (willOverflow(1)) {
            return false;
        }
        *b = mCursor[0];
        mCursor += 1;
        return true;
    }

    bool read(uint8_t* i) noexcept {
        if (willOverflow(1)) {
            return false;
        }
        *i = mCursor[0];
        mCursor += 1;
        return true;
    }

    bool read(uint16_t* i) noexcept {
        if (willOverflow(2)) {
            return false;
        }
        *i = 0;
        *i |= mCursor[0];
        *i |= mCursor[1] << 8;
        mCursor += 2;
        return true;
    }

    bool read(uint32_t* i) noexcept {
        if (willOverflow(4)) {
            return false;
        }
        *i = 0;
        *i |= mCursor[0];
        *i |= mCursor[1] << 8;
        *i |= mCursor[2] << 16;
        *i |= mCursor[3] << 24;
        mCursor += 4;
        return true;
    }

    bool read(uint64_t* i) noexcept {
        if (willOverflow(8)) {
            return false;
        }
        *i = 0;
        *i |= static_cast<int64_t>(mCursor[0]);
        *i |= static_cast<int64_t>(mCursor[1]) << 8;
        *i |= static_cast<int64_t>(mCursor[2]) << 16;
        *i |= static_cast<int64_t>(mCursor[3]) << 24;
        *i |= static_cast<int64_t>(mCursor[4]) << 32;
        *i |= static_cast<int64_t>(mCursor[5]) << 40;
        *i |= static_cast<int64_t>(mCursor[6]) << 48;
        *i |= static_cast<int64_t>(mCursor[7]) << 56;
        mCursor += 8;
        return true;
    }

    bool read(utils::CString* s) noexcept;

    bool read(const char** blob, size_t* size) noexcept;

    bool read(const char** s) noexcept;

    bool read(float* f) noexcept {
        if (willOverflow(4)) {
            return false;
        }
        uint32_t i;
        i = 0;
        i |= mCursor[0];
        i |= mCursor[1] << 8;
        i |= mCursor[2] << 16;
        i |= mCursor[3] << 24;
        *f = reinterpret_cast<float&>(i);
        mCursor += 4;
        return true;
    }

    const uint8_t* getCursor() const noexcept {
        return mCursor;
    }

    void setCursor(const uint8_t* cursor) noexcept {
        if (mSrc <= cursor && cursor < mEnd) {
            mCursor = cursor;
        } else {
            mCursor = mEnd;
        }
    }

private:
    const uint8_t* mSrc;
    const uint8_t* mCursor;
    const uint8_t* mEnd;
};

} //namespace filaflat

#endif // TNT_FILAFLAT_UNFLATTENER_H
