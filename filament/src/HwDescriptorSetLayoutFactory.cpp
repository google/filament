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

#include "HwDescriptorSetLayoutFactory.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <private/backend/DriverApi.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Hash.h>
#include <utils/Log.h>

#include <algorithm>
#include <utility>

#include <stdint.h>
#include <stdlib.h>

namespace filament {

using namespace utils;
using namespace backend;

size_t HwDescriptorSetLayoutFactory::Parameters::hash() const noexcept {
    return utils::hash::murmurSlow(
            reinterpret_cast<uint8_t const *>(dsl.bindings.data()),
            dsl.bindings.size() * sizeof(backend::DescriptorSetLayoutBinding),
            42);
}

bool operator==(HwDescriptorSetLayoutFactory::Parameters const& lhs,
        HwDescriptorSetLayoutFactory::Parameters const& rhs) noexcept {
    return (lhs.dsl.bindings.size() == rhs.dsl.bindings.size()) &&
           std::equal(
                   lhs.dsl.bindings.begin(), lhs.dsl.bindings.end(),
                   rhs.dsl.bindings.begin());
}

// ------------------------------------------------------------------------------------------------

HwDescriptorSetLayoutFactory::HwDescriptorSetLayoutFactory()
        : mArena("HwDescriptorSetLayoutFactory::mArena", SET_ARENA_SIZE),
          mBimap(mArena) {
    mBimap.reserve(256);
}

HwDescriptorSetLayoutFactory::~HwDescriptorSetLayoutFactory() noexcept = default;

void HwDescriptorSetLayoutFactory::terminate(DriverApi&) noexcept {
    assert_invariant(mBimap.empty());
}

auto HwDescriptorSetLayoutFactory::create(DriverApi& driver,
        backend::DescriptorSetLayout dsl) noexcept -> Handle {

    std::sort(dsl.bindings.begin(), dsl.bindings.end(),
            [](auto&& lhs, auto&& rhs) {
        return lhs.binding < rhs.binding;
    });

    // see if we already have seen this RenderPrimitive
    Key const key({ dsl });
    auto pos = mBimap.find(key);

    // the common case is that we've never seen it (i.e.: no reuse)
    if (UTILS_LIKELY(pos == mBimap.end())) {
        auto handle = driver.createDescriptorSetLayout(std::move(dsl));
        mBimap.insert(key, { handle });
        return handle;
    }

    ++(pos->first.pKey->refs);

    return pos->second.handle;
}

void HwDescriptorSetLayoutFactory::destroy(DriverApi& driver, Handle handle) noexcept {
    // look for this handle in our map
    auto pos = mBimap.find(Value{ handle });
    if (--pos->second.pKey->refs == 0) {
        mBimap.erase(pos);
        driver.destroyDescriptorSetLayout(handle);
    }
}

} // namespace filament
