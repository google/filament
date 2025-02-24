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

#ifndef SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_
#define SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_

#include "dawn/common/WeakRefSupport.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

template <typename RefCountedT>
class ContentLessObjectCache;

namespace detail {

// Placeholding base class for cacheable types to enable easier compile-time verifications.
class ContentLessObjectCacheableBase {};

}  // namespace detail

// Classes need to extend this type if they want to be cacheable via the ContentLessObjectCache. It
// is also assumed that the type already extends RefCounted in some way. Otherwise, this helper
// class does not work.
template <typename RefCountedT>
class ContentLessObjectCacheable : public detail::ContentLessObjectCacheableBase,
                                   public WeakRefSupport<RefCountedT> {
  public:
    // Currently, any cacheables should call Uncache in their DeleteThis override. This is important
    // because otherwise, the objects may be leaked in the internal set.
    void Uncache() {
        if (mCache != nullptr) {
            // Note that Erase sets mCache to nullptr. We do it in Erase instead of doing it here in
            // case users call Erase somewhere else before the Uncache call.
            mCache->Erase(static_cast<RefCountedT*>(this));
        }
    }

  protected:
    // The dtor asserts that the cache isn't set to ensure that we were Uncache-d or never cached.
    ~ContentLessObjectCacheable() override { DAWN_ASSERT(mCache == nullptr); }

  private:
    friend class ContentLessObjectCache<RefCountedT>;

    // Pointer to the owning cache if we were inserted at any point. This is set via the
    // Insert/Erase functions on the cache.
    raw_ptr<ContentLessObjectCache<RefCountedT>> mCache = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_
