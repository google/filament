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

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <private/backend/DriverApi.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Hash.h>

#include <stdlib.h>

namespace filament {

using namespace utils;
using namespace backend;

size_t HwRenderPrimitiveFactory::KeyHash::operator()(
        HwRenderPrimitiveFactory::Key const& p) const noexcept {
    return utils::hash::combine(
            p.params.vbh.getId(),
            utils::hash::combine(p.params.ibh.getId(), (size_t)p.params.type));
}

bool operator==(HwRenderPrimitiveFactory::Key const& lhs,
        HwRenderPrimitiveFactory::Key const& rhs) noexcept {
    return lhs.params.vbh == rhs.params.vbh &&
           lhs.params.ibh == rhs.params.ibh &&
           lhs.params.type == rhs.params.type;
}

// ------------------------------------------------------------------------------------------------

HwRenderPrimitiveFactory::HwRenderPrimitiveFactory()
        : mArena("HwRenderPrimitiveFactory::mArena", SET_ARENA_SIZE),
          mBimap(mArena) {
    mBimap.reserve(256);
}

HwRenderPrimitiveFactory::~HwRenderPrimitiveFactory() noexcept = default;

void HwRenderPrimitiveFactory::terminate(DriverApi&) noexcept {
    assert_invariant(mBimap.empty());
}

auto HwRenderPrimitiveFactory::create(DriverApi& driver,
        backend::VertexBufferHandle vbh,
        backend::IndexBufferHandle ibh,
        backend::PrimitiveType type) noexcept -> Handle {

    // see if we already have seen this RenderPrimitive
    Key const key{{ vbh, ibh, type }, 1 };
    auto pos = mBimap.find(key);

    // the common case is that we've never seen it (i.e.: no reuse)
    if (UTILS_LIKELY(pos == mBimap.end())) {
        auto handle = driver.createRenderPrimitive(vbh, ibh, type);
        mBimap.insert(key, { handle });
        return handle;
    }

    pos->first.pKey->refs++;
    return pos->second.handle;
}

void HwRenderPrimitiveFactory::destroy(DriverApi& driver, Handle handle) noexcept {
    // look for this handle in our map
    auto pos = mBimap.find(Value{ handle });
    if (--pos->second.pKey->refs == 0) {
        mBimap.erase(pos);
        driver.destroyRenderPrimitive(handle);
    }
}

} // namespace filament
