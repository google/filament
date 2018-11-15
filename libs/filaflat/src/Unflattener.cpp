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

#include <filaflat/Unflattener.h>

namespace filaflat {

bool Unflattener::read(utils::CString* s) noexcept {
    const uint8_t* start = mCursor;
    while (mCursor < mEnd && *mCursor != '\0') {
        mCursor++;
    }
    bool overflowed = mCursor >= mEnd;
    if (!overflowed) {
        mCursor++;
        *s = utils::CString{ (const char*)start, (utils::CString::size_type)(mCursor - start) };
    }
    return !overflowed;
}

bool Unflattener::read(const char** blob, size_t* size) noexcept {
    uint64_t nbytes;
    if (!read(&nbytes)) {
        return false;
    }
    const uint8_t* start = mCursor;
    mCursor += nbytes;
    bool overflowed = mCursor > mEnd;
    if (!overflowed) {
        *blob = (const char*) start;
        *size = nbytes;
    }
    return !overflowed;
}

bool Unflattener::read(const char** s) noexcept {
    const uint8_t* start = mCursor;
    while (mCursor < mEnd && *mCursor != '\0') {
        mCursor++;
    }
    bool overflowed = mCursor >= mEnd;
    if (!overflowed) {
        mCursor++;
    }
    *s = (char*) start;
    return !overflowed;
}

}
