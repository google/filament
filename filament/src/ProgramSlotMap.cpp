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

#include "ProgramSlotMap.h"

#include "DynamicSpecConstKey.h"

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <utils/debug.h>
#include <utils/Panic.h>
#include <utils/Slice.h>

#include <cstdint>

namespace filament {

using namespace utils;

ProgramSlotMap::ProgramSlotMap() noexcept {
    for (Row& row : mSlots) {
        row.fill(NO_SLOT);
    }
}

ProgramSlotMap::ProgramSlotMap(Slice<const Variant> const variants,
        Slice<const Variant> const depthVariants, MaterialDomain const materialDomain,
        bool const isVariantLit) noexcept
        : ProgramSlotMap() {
    auto const add = [&](Variant const variant) {
        assert_invariant(variant.key < VARIANT_COUNT);
        Row& row = mSlots[variant.key];
        if (row[0] != NO_SLOT) {
            return; // already in the slot map (specKey 0 is always valid)
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

void ProgramSlotMap::panicNoSlot(Variant const variant, DynamicSpecConstKey const specKey) const {
    FILAMENT_CHECK_POSTCONDITION(false)
            << "Program cache has no slot for variant=" << uint32_t(variant.key)
            << ", specKey=" << uint32_t(specKey.key) << ", size=" << mSize;
}

} // namespace filament
