// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_ATOMIC_H_
#define SRC_DAWN_COMMON_ATOMIC_H_

#include <atomic>
#include <concepts>
#include <type_traits>

namespace dawn {

// A wrapper around std::atomic that allows specifying default memory orders for load and store
// operations at the type level. The casting operator uses LoadMemoryOrder and the assignment
// operator uses StoreMemoryOrder.
template <typename T,
          std::memory_order LoadMemoryOrder = std::memory_order::seq_cst,
          std::memory_order StoreMemoryOrder = LoadMemoryOrder>
class Atomic {
  public:
    Atomic() = default;
    explicit Atomic(T value) : mValue(value) {}

    Atomic(const Atomic&) = delete;

    Atomic& operator=(const Atomic& src) {
        mValue.store(src.mValue.load(LoadMemoryOrder), StoreMemoryOrder);
        return *this;
    }

    operator T() const { return mValue.load(LoadMemoryOrder); }

    T operator=(T value) {
        mValue.store(value, StoreMemoryOrder);
        return value;
    }

    T operator->() const
        requires std::is_pointer_v<T>
    {
        return mValue.load(LoadMemoryOrder);
    }

  private:
    std::atomic<T> mValue;
};

// Equivalent to C++ stdlib's fetch_max that is only available in C++26 and beyond. This returns the
// value preceding the effects of this function.
template <typename T>
    requires std::totally_ordered<T>
T FetchMax(std::atomic<T>& t, T arg) {
    // Atomically set |t| to |arg| if |arg| > |t|.
    T current = t.load(std::memory_order::acquire);
    while (arg > current && !t.compare_exchange_weak(current, arg, std::memory_order::acq_rel)) {
    }
    return current;
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_RESULT_H_
