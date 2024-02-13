/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "HwRenderPrimitiveFactory.h"

#include <stdlib.h>

namespace filament {

using namespace utils;
using namespace backend;

bool operator<(HwRenderPrimitiveFactory::Key const& lhs,
        HwRenderPrimitiveFactory::Key const& rhs) noexcept {
    if (lhs.vbh == rhs.vbh) {
        if (lhs.ibh == rhs.ibh) {
            return lhs.type < rhs.type;
        } else {
            return lhs.ibh < rhs.ibh;
        }
    } else {
        return lhs.vbh < rhs.vbh;
    }
}

inline bool operator<(HwRenderPrimitiveFactory::Entry const& lhs,
        HwRenderPrimitiveFactory::Entry const& rhs) noexcept {
    return lhs.key < rhs.key;
}

inline bool operator<(HwRenderPrimitiveFactory::Key const& lhs,
        HwRenderPrimitiveFactory::Entry const& rhs) noexcept {
    return lhs < rhs.key;
}

inline bool operator<(HwRenderPrimitiveFactory::Entry const& lhs,
        HwRenderPrimitiveFactory::Key const& rhs) noexcept {
    return lhs.key < rhs;
}

// ------------------------------------------------------------------------------------------------

HwRenderPrimitiveFactory::HwRenderPrimitiveFactory()
        : mArena("HwRenderPrimitiveFactory::mArena", SET_ARENA_SIZE),
          mSet(mArena) {
    mMap.reserve(256);
}

HwRenderPrimitiveFactory::~HwRenderPrimitiveFactory() noexcept = default;

void HwRenderPrimitiveFactory::terminate(DriverApi&) noexcept {
    assert_invariant(mMap.empty());
    assert_invariant(mSet.empty());
}

RenderPrimitiveHandle HwRenderPrimitiveFactory::create(DriverApi& driver,
        VertexBufferHandle vbh, IndexBufferHandle ibh,
        PrimitiveType type) noexcept {

    const Key key = { vbh, ibh, type };

    // see if we already have seen this RenderPrimitive
    auto pos = mSet.find(key);

    // the common case is that we've never seen it (i.e.: no reuse)
    if (UTILS_LIKELY(pos == mSet.end())) {
        // create the backend object
        auto handle = driver.createRenderPrimitive(vbh, ibh, type);
        // insert key/handle in our set with a refcount of 1
        // IMPORTANT: std::set<> doesn't invalidate iterators in insert/erase
        auto [ipos, _] = mSet.insert({ key, handle, 1 });
        // map the handle back to the key/payload
        mMap.insert({ handle.getId(), ipos });
        return handle;
    }
    pos->refs++;
    return pos->handle;
}

void HwRenderPrimitiveFactory::destroy(DriverApi& driver, RenderPrimitiveHandle rph) noexcept {
    // look for this handle in our map
    auto pos = mMap.find(rph.getId());

    // it must be there
    assert_invariant(pos != mMap.end());

    // check the refcount and destroy if needed
    auto ipos = pos->second;
    if (--ipos->refs == 0) {
        mSet.erase(ipos);
        mMap.erase(pos);
        driver.destroyRenderPrimitive(rph);
    }
}

} // namespace filament
