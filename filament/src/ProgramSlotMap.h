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

#ifndef TNT_FILAMENT_PROGRAMSLOTMAP_H
#define TNT_FILAMENT_PROGRAMSLOTMAP_H

#include "DynamicSpecConstKey.h"

#include <private/filament/Variant.h>

#include <filament/MaterialEnums.h>

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace filament {

// Maps a (Variant, DynamicSpecConstKey) pair to a slot of a LocalProgramCache.
//
// Only the combinations a material can request get a slot: each of the material's variants and
// depth variants, combined with the specialization keys DynamicSpecConstKey::getValidKeys()
// allows for it. This keeps the cache dense, and adding a specialization constant bit only grows
// it by the combinations that actually use that bit, instead of doubling it.
//
// The mapping is a single lookup in a [variant][specKey] table (~2 KiB, shared by all the
// materials of the same kind), so it stays as cheap as concatenating the key bits.
class ProgramSlotMap {
public:
    using Index = uint32_t;

    // Slot maps are immutable and shared by all the materials with the same domain and lighting.
    static ProgramSlotMap const& get(MaterialDomain materialDomain, bool isVariantLit) noexcept;

    ProgramSlotMap(utils::Slice<const Variant> variants,
            utils::Slice<const Variant> depthVariants, MaterialDomain materialDomain,
            bool isVariantLit) noexcept;

    ProgramSlotMap(ProgramSlotMap const&) = delete;
    ProgramSlotMap& operator=(ProgramSlotMap const&) = delete;

    uint32_t size() const noexcept { return mSize; }

    // `specKey` must be filtered (see MaterialDefinition::filterProgramSpecKey()), panics if
    // there's no slot. Depth variants ignore `specKey`.
    Index map(Variant const variant, DynamicSpecConstKey const specKey) const {
        uint16_t const slot = lookup(variant, specKey);
        if (UTILS_VERY_UNLIKELY(slot == NO_SLOT)) {
            panicNoSlot(variant, specKey, mSize);
        }
        return slot;
    }

    bool hasSlot(Variant const variant, DynamicSpecConstKey const specKey) const noexcept {
        return lookup(variant, specKey) != NO_SLOT;
    }

private:
    using Row = std::array<uint16_t, DYNAMIC_SPEC_CONST_KEY_COUNT>;

    static constexpr uint16_t NO_SLOT = 0xFFFF;
    static constexpr uint32_t SPEC_KEY_MASK = DYNAMIC_SPEC_CONST_KEY_COUNT - 1u;
    // Out-of-range variant keys use this empty row.
    static constexpr std::size_t NO_SLOT_ROW = VARIANT_COUNT;

    static_assert(VARIANT_COUNT * DYNAMIC_SPEC_CONST_KEY_COUNT < NO_SLOT,
            "slots must fit in uint16_t");

    uint16_t lookup(Variant const variant, DynamicSpecConstKey const specKey) const noexcept {
        std::size_t const row = variant.key < NO_SLOT_ROW ? variant.key : NO_SLOT_ROW;
        return mSlots[row][specKey.key & SPEC_KEY_MASK];
    }

    // Must not return, or map() would return NO_SLOT.
    static void panicNoSlot(Variant variant, DynamicSpecConstKey specKey, uint32_t size)
            UTILS_NORETURN UTILS_NOINLINE UTILS_COLD;

    std::array<Row, VARIANT_COUNT + 1> mSlots{};
    uint32_t mSize = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_PROGRAMSLOTMAP_H
