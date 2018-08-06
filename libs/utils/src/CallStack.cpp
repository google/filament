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

#include <utils/CallStack.h>
#include <utils/Log.h>

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <memory>

// FIXME: Android doesn't have execinfo.h (but has unwind.h)
#if !defined(ANDROID) && !defined(WIN32)
#include <execinfo.h>
#endif

#if !defined(WIN32)
#include <cxxabi.h>
#endif

#include <string.h>

#ifdef __EXCEPTIONS
#include <stdexcept>
#endif

namespace utils {

struct FreeDeleter {
    void operator()(const void* pv) const { free(const_cast<void*>(pv)); }
};

// ------------------------------------------------------------------------------------------------

CallStack CallStack::unwind(size_t ignore) noexcept {
    CallStack stack;
    stack.update(ignore);
    return stack;
}

// ------------------------------------------------------------------------------------------------

intptr_t CallStack::operator [](size_t index) const {
    if (index >= m_frame_count) {
#ifdef __EXCEPTIONS
        throw std::out_of_range("out-of-range access in CallStack::operator[]");
#endif
        std::abort();
    }
    return m_stack[index].pc;
}

size_t CallStack::getFrameCount() const noexcept {
    return m_frame_count;
}

void CallStack::update(size_t ignore) noexcept {
    update_gcc(ignore);
}

void CallStack::update_gcc(size_t ignore) noexcept {
    // reset the object
    ssize_t size = 0;

    void *array[NUM_FRAMES];
#if !defined(ANDROID) && !defined(WIN32)
    size = ::backtrace(array, NUM_FRAMES);
    size -= ignore;
#endif
    for (ssize_t i = 0; i < size; i++) {
        m_stack[i].pc = intptr_t(array[ignore+i]);
    }
    size--; // the last one seems to always be 0x0

    // update how many frames we have
    m_frame_count = size_t(std::max(ssize_t(0), size));
}

bool CallStack::operator <(const CallStack& rhs) const {
    if (m_frame_count != rhs.m_frame_count) {
        return m_frame_count < rhs.m_frame_count;
    }
    return memcmp(m_stack, rhs.m_stack, m_frame_count * sizeof(StackFrameInfo)) < 0;
}

// ------------------------------------------------------------------------------------------------

utils::CString CallStack::demangleTypeName(const char* mangled) {
#if !defined(WIN32)
    size_t len;
    int status;
    std::unique_ptr<char, FreeDeleter> demangled(abi::__cxa_demangle(mangled, nullptr, &len, &status));
    if (!status && demangled) {
        // success
        return CString(demangled.get());
    }
    // failed to demangle string or parsing error:  return input
#endif
    return CString(mangled);
}

// ------------------------------------------------------------------------------------------------

io::ostream& operator<<(io::ostream& stream, const CallStack& callstack) {
#if !defined(ANDROID) && !defined(WIN32)
    size_t size = callstack.getFrameCount();
    for (size_t i = 0; i < size; i++) {
        intptr_t pc = callstack[i];
        std::unique_ptr<char*, FreeDeleter> symbols(::backtrace_symbols((void* const*)&pc, 1));
        char const* const symbol = symbols.get()[0];
        stream << "#" << i << " " << symbol;
        if (i < size - 1) {
            stream << io::endl;
        }
    }
#endif
    return stream;
}

} // namespace utils
