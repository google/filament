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

#ifndef UTILS_CALLSTACK_H
#define UTILS_CALLSTACK_H

#include <stddef.h>
#include <stdint.h>
#include <typeinfo>

#include <utils/CString.h>
#include <utils/compiler.h>

namespace utils {
namespace io {
class ostream;
}

/**
 * CallStack captures the current's thread call stack.
 */
class CallStack {
public:
    /**
     * Creates an empty call stack
     * @see CallStack::capture()
     */
    CallStack() = default;
    CallStack(const CallStack&) = default;
    ~CallStack() = default;

    /**
     * A convenience method to create and capture the stack trace in one go.
     * @param ignore number frames to ignore at the top of the stack.
     * @return A CallStack object
     */
    static CallStack unwind(size_t ignore = 0) noexcept;

    /**
     * Capture the current thread's stack and replaces the existing one if any.
     * @param ignore number frames to ignore at the top of the stack.
     */
    void update(size_t ignore = 0) noexcept;

    /**
     * Get the number of stack frames this object has recorded.
     * @return How many stack frames are accessible through operator[]
     */
    size_t getFrameCount() const noexcept;

    /**
     * Return the program-counter of each stack frame captured
     * @param index of the frame between 0 and getFrameCount()-1
     * @return the program-counter of the stack-frame recorded at index \p index
     * @throw std::out_of_range if the index is out of range
     */
    intptr_t operator [](size_t index) const;

   /** Demangles a C++ type name */
    static CString demangleTypeName(const char* mangled);

    template<typename T>
    static CString typeName() {
#if UTILS_HAS_RTTI
        return demangleTypeName(typeid(T).name());
#else
        return CString("<no-rtti>");
#endif
    }

    /**
     * Outputs a CallStack into a stream.
     * This will print, when possible, the demangled names of functions corresponding to the
     * program-counter recorded.
     */
    friend io::ostream& operator <<(io::ostream& stream, const CallStack& callstack);

    bool operator <(const CallStack& rhs) const;

    bool operator >(const CallStack& rhs) const {
        return rhs < *this;
    }

    bool operator !=(const CallStack& rhs) const {
        return *this < rhs || rhs < *this;
    }

    bool operator >=(const CallStack& rhs) const {
        return !operator <(rhs);
    }

    bool operator <=(const CallStack& rhs) const {
        return !operator >(rhs);
    }

    bool operator ==(const CallStack& rhs) const {
        return !operator !=(rhs);
    }

private:
    void update_gcc(size_t ignore) noexcept;

    static CString demangle(const char* mangled);

    static constexpr size_t NUM_FRAMES = 20;

    struct StackFrameInfo {
        intptr_t pc;
    };

    size_t m_frame_count = 0;
    StackFrameInfo m_stack[NUM_FRAMES];
};

} // namespace utils

#endif // UTILS_CALLSTACK_H
