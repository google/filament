// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_WEAKREFSUPPORT_H_
#define SRC_DAWN_COMMON_WEAKREFSUPPORT_H_

#include "dawn/common/Compiler.h"
#include "dawn/common/Ref.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

template <typename T>
class WeakRef;

namespace detail {

// Indirection layer to provide external ref-counting for WeakRefs.
class WeakRefData : public RefCounted {
  public:
    explicit WeakRefData(RefCounted* value);
    void Invalidate();

    // Tries to return a valid Ref to `mValue` if it's internal refcount is not already 0. If the
    // internal refcount has already reached 0, returns nullptr instead.
    Ref<RefCounted> TryGetRef();

    // Returns the raw pointer to the RefCounted. In general, this is an unsafe operation because
    // the RefCounted can become invalid after being retrieved.
    RefCounted* UnsafeGet() const;

  private:
    std::mutex mMutex;
    raw_ptr<RefCounted> mValue = nullptr;
};

// Interface base class used to enable compile-time verification of WeakRefSupport functions.
class WeakRefSupportBase {
  protected:
    explicit WeakRefSupportBase(Ref<detail::WeakRefData> data);
    virtual ~WeakRefSupportBase();

  private:
    template <typename T>
    friend class ::dawn::WeakRef;

    Ref<detail::WeakRefData> mData;
};

}  // namespace detail

// Class that should be extended to enable WeakRefs for the type.
template <typename T>
class WeakRefSupport : public detail::WeakRefSupportBase {
  public:
#if DAWN_COMPILER_IS(CLANG)
    // Note that the static cast below fails CFI/UBSAN builds due to the cast. The cast itself is
    // safe so we suppress the failure. See the following link regarding the cast:
    // https://stackoverflow.com/questions/73172193/can-you-static-cast-this-to-a-derived-class-in-a-base-class-constructor-then-u,
    DAWN_NO_SANITIZE("cfi-derived-cast")
    DAWN_NO_SANITIZE("vptr")
#endif
    WeakRefSupport()
        : WeakRefSupportBase(AcquireRef(new detail::WeakRefData(static_cast<T*>(this)))) {}
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_WEAKREFSUPPORT_H_
