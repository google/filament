/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "ProgramCacheLayout.h"

#include "DynamicSpecConstKey.h"
#include "MaterialDefinition.h"

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <utils/debug.h>
#include <utils/Panic.h>
#include <utils/Slice.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace filament {

ProgramCacheLayout const& ProgramCacheLayout::get(MaterialDomain const materialDomain,
        bool const isVariantLit) noexcept {
    auto const make = [](MaterialDomain const domain, bool const lit) {
        return ProgramCacheLayout(MaterialDefinition::getVariantsFor(domain, lit),
                MaterialDefinition::getDepthVariantsFor(domain), domain, lit);
    };
    switch (materialDomain) {
        case MaterialDomain::SURFACE:
            if (isVariantLit) {
                static ProgramCacheLayout const sLayout = make(MaterialDomain::SURFACE, true);
                return sLayout;
            } else {
                static ProgramCacheLayout const sLayout = make(MaterialDomain::SURFACE, false);
                return sLayout;
            }
        case MaterialDomain::POST_PROCESS: {
            // Post-process programs never use specialization keys, lighting doesn't matter.
            static ProgramCacheLayout const sLayout = make(MaterialDomain::POST_PROCESS, false);
            return sLayout;
        }
        case MaterialDomain::COMPUTE: {
            // TODO: implement MaterialDomain::COMPUTE
            static constexpr Variant COMPUTE_VARIANTS[] = { Variant{ 0 } };
            static ProgramCacheLayout const sLayout({ COMPUTE_VARIANTS, 1 }, {},
                    MaterialDomain::COMPUTE, false);
            return sLayout;
        }
    }
    // Unreachable, but some compilers don't know.
    static ProgramCacheLayout const sEmpty({}, {}, MaterialDomain::COMPUTE, false);
    return sEmpty;
}

ProgramCacheLayout::ProgramCacheLayout(utils::Slice<const Variant> const variants,
        utils::Slice<const Variant> const depthVariants, MaterialDomain const materialDomain,
        bool const isVariantLit) noexcept {
    for (Row& row : mSlots) {
        row.fill(NO_SLOT);
    }

    auto const add = [&](Variant const variant) {
        assert_invariant(variant.key < VARIANT_COUNT);
        Row& row = mSlots[variant.key];
        if (row[0] != NO_SLOT) {
            return; // already in the layout (specKey 0 is always valid)
        }

        DynamicSpecConstKey::ValidKeys const keys =
                DynamicSpecConstKey::getValidKeys(variant, materialDomain, isVariantLit);
        assert_invariant(keys.size > 0);

        if (materialDomain == MaterialDomain::SURFACE && Variant::isValidDepthVariant(variant)) {
            // Depth variants ignore the specialization key: all keys share one slot.
            row.fill(uint16_t(mSize++));
        } else {
            for (DynamicSpecConstKey const key : keys) {
                row[key.key] = uint16_t(mSize++);
            }
        }
    };

    for (Variant const variant : variants) {
        add(variant);
    }
    for (Variant const variant : depthVariants) {
        add(variant);
    }
}

void ProgramCacheLayout::panicNoSlot(Variant const variant, DynamicSpecConstKey const specKey,
        uint32_t const size) {
    FILAMENT_CHECK_POSTCONDITION(false)
            << "Program cache has no slot for variant=" << uint32_t(variant.key)
            << ", specKey=" << uint32_t(specKey.key) << ", size=" << size;
    std::abort(); // in case postcondition failures aren't fatal
}

} // namespace filament
