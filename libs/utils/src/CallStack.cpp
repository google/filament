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
#include <utils/CString.h>
#include <utils/compiler.h>
#include <utils/ostream.h>

#include <algorithm>
#include <memory>
#include <cstdlib>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if !defined(__ANDROID__) && !defined(WIN32) && !defined(__EMSCRIPTEN__)
#include <execinfo.h>
#define HAS_EXECINFO 1
#else
#define HAS_EXECINFO 0
#endif

// execinfo.h is available as of Android 33
#if defined(__ANDROID__) && (__ANDROID_API__ >= 33)
#include <execinfo.h>
#undef HAS_EXECINFO
#define HAS_EXECINFO 1
#endif

#if !defined(WIN32) && !defined(__EMSCRIPTEN__)
#include <dlfcn.h>
#define HAS_DLADDR 1
#else
#define HAS_DLADDR 0
#endif

#if !defined(NDEBUG) && !defined(WIN32)
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

intptr_t CallStack::operator[](size_t index) const {
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

void CallStack::update_gcc(size_t UTILS_UNUSED ignore) noexcept {
    // reset the object
    ssize_t size = 0;

#if HAS_EXECINFO
    void* array[NUM_FRAMES];
    size = ::backtrace(array, NUM_FRAMES);
    size -= ignore;
    for (ssize_t i = 0; i < size; i++) {
        m_stack[i].pc = intptr_t(array[ignore + i]);
    }
    size--; // the last one seems to always be 0x0
#endif

    // update how many frames we have
    m_frame_count = size_t(std::max(ssize_t(0), size));
}

bool CallStack::operator<(const CallStack& rhs) const {
    if (m_frame_count != rhs.m_frame_count) {
        return m_frame_count < rhs.m_frame_count;
    }
    return memcmp(m_stack, rhs.m_stack, m_frame_count * sizeof(StackFrameInfo)) < 0;
}

// ------------------------------------------------------------------------------------------------

CString CallStack::demangle(const char* mangled) {
#if !defined(NDEBUG) && !defined(WIN32)
    size_t len;
    int status;
    std::unique_ptr<char, FreeDeleter> const demangled(abi::__cxa_demangle(mangled, nullptr, &len, &status));
    if (!status && demangled) {
        // success
        return CString(demangled.get());
    }
    // failed to demangle string or parsing error:  return input
#endif
    return CString(mangled);
}


CString CallStack::demangleTypeName(const char* mangled) {
    return demangle(mangled);
}

// ------------------------------------------------------------------------------------------------

io::ostream& operator<<(io::ostream& stream, CallStack const& UTILS_UNUSED callstack) {
#if HAS_EXECINFO
    size_t const size = callstack.getFrameCount();
    char buf[1024];
    for (size_t i = 0; i < size; i++) {
        Dl_info info;
        void* pc = (void*)callstack[i];
#if HAS_DLADDR
        if (::dladdr(pc, &info)) {
            char const* exe = strrchr(info.dli_fname, '/');
            snprintf(buf, sizeof(buf), "#%u\t%-31s %*p %s + %zd\n",
                    unsigned(i),
                    exe ? exe + 1 : info.dli_fname,
                    int(2 + sizeof(void*)*2),
                    pc,
                    CallStack::demangle(info.dli_sname).c_str(),
                    (char *)callstack[i] - (char *)info.dli_saddr);
            stream << buf;
        } else
#endif
        {
            char** symbols = ::backtrace_symbols(&pc, 1);
            stream << "#" << i << "\t" << symbols[0] << "\n";
            free((void*)symbols);
        }
    }
    stream << io::endl;
#endif
    return stream;
}

} // namespace utils
