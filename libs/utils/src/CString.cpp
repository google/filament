/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <utils/CString.h>

#include <utils/compiler.h>

#include <algorithm>
#include <memory>

namespace utils {

int StaticString::compare(const StaticString& rhs) const noexcept {
    size_type lhs_size = size();
    size_type rhs_size = rhs.size();
    if (lhs_size < rhs_size) return -1;
    if (lhs_size > rhs_size) return 1;
    return strncmp(data(), rhs.data(), size());
}

// ------------------------------------------------------------------------------------------------

UTILS_NOINLINE
CString::CString(const char* cstr, size_t length) {
    if (length && cstr) {
        assert(length == strlen(cstr));
        Data* p = (Data*)malloc(sizeof(Data) + length + 1);
        p->length = (size_type)length;
        mCStr = (value_type*)(p + 1);
        // we don't use memcpy here to avoid a call to libc, the generated code is pretty good.
        std::uninitialized_copy_n(cstr, length + 1, mCStr);
    }
}

CString::CString(const char* cstr)
        : CString(cstr, size_type(cstr ? strlen(cstr) : 0)) {
}

CString::CString(const CString& rhs)
        : CString(rhs.c_str(), rhs.size()) {
}

CString& CString::operator=(const CString& rhs) {
    if (this != &rhs) {
        auto const p = mData ? mData - 1 : nullptr;
        new(this) CString(rhs);
        free(p);
    }
    return *this;
}

CString& CString::replace(size_type pos, size_type len, const CString& str) noexcept {
    assert(pos <= size());

    len = std::min(len, size() - pos);

    // The new size of the string, after the replacement.
    const size_type newSize = size() - len + str.size();

    // Allocate enough memory to hold the new string.
    Data* p = (Data*) malloc(sizeof(Data) + newSize + 1);
    assert(p);
    p->length = newSize;
    value_type* newStr = (value_type*) (p + 1);

    const value_type* beginning = mCStr;
    const value_type* replacementStart = mCStr + pos;
    const value_type* replacementEnd = replacementStart + len;
    const value_type* end = beginning + size();

    value_type* ptr = newStr;
    ptr = std::uninitialized_copy(beginning, replacementStart, ptr);
    ptr = std::uninitialized_copy_n(str.c_str_safe(), str.length(), ptr);
    ptr = std::uninitialized_copy(replacementEnd, end, ptr);

    // null-terminator
    *ptr = 0;

    std::swap(mCStr, newStr);
    if (newStr) {
        free((Data*) newStr - 1);
    }

    return *this;
}

} // namespace utils
