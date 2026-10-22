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

#include <utils/compiler.h>
#include <utils/CString.h>

#include <cstddef>
#include <cstdint>

namespace filaflat {

using namespace utils;

bool Unflattener::read(const char** blob, size_t* size) noexcept {
    uint64_t nbytes;
    if (!read(&nbytes)) {
        return false;
    }

    // Prevent integer/pointer wrap-around by verifying boundaries
    if (willOverflow(nbytes)) {
        return false;
    }

    const uint8_t* start = mCursor;
    mCursor += nbytes;

    *blob = reinterpret_cast<const char*>(start);
    *size = nbytes;
    
    return true;
}

bool Unflattener::read(CString* const s) noexcept {
    const uint8_t* const start = mCursor;
    const uint8_t* const last = mEnd;
    const uint8_t* curr = start;

    while (curr < last && *curr != '\0') {
        curr++;
    }

    // A securely read CString must be explicitly null-terminated inside the chunk.
    bool const overflowed = curr == last;
    if (UTILS_LIKELY(!overflowed)) {
        *s = CString{ reinterpret_cast<const char*>(start), CString::size_type(curr - start) };
        curr++;
    }

    mCursor = curr;
    return !overflowed;
}

bool Unflattener::read(const char** const s) noexcept {
    const uint8_t* const start = mCursor;
    const uint8_t* const last = mEnd;
    const uint8_t* curr = start;

    while (curr < last && *curr != '\0') {
        curr++;
    }

    // Explicitly reject strings that terminate abruptly at chunk boundaries without '\0'
    bool const overflowed = curr == last;
    if (UTILS_LIKELY(!overflowed)) {
        *s = reinterpret_cast<char const*>(start);
        curr++;
    }

    mCursor = curr;
    return !overflowed;
}

}
