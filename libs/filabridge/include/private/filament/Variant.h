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

#ifndef TNT_FILAMENT_VARIANT_H
#define TNT_FILAMENT_VARIANT_H

#include <stdint.h>
#include <cstddef>

#include <utils/bitset.h>

namespace filament {
    static constexpr size_t VARIANT_BITS = 7;
    static constexpr size_t VARIANT_COUNT = 1 << VARIANT_BITS;

    using VariantList = utils::bitset<uint64_t, VARIANT_COUNT / 64>;

    // IMPORTANT: update filterVariant() when adding more variants
    // Also be sure to update formatVariantString inside CommonWriter.cpp
    struct Variant {
        Variant() noexcept = default;
        constexpr explicit Variant(uint8_t key) noexcept : key(key) { }


        // DIR: Directional Lighting
        // DYN: Dynamic Lighting
        // SRE: Shadow Receiver
        // SKN: Skinning
        // DEP: Depth only
        // FOG: Fog
        // PCK: Picking (depth variant only)
        // VSM: Variance shadow maps
        //
        //   X: either 1 or 0
        //
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        // Variant              |  0  | VSM | FOG | DEP | SKN | SRE | DYN | DIR |   128
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //                                    PCK
        //
        // Standard variants:
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //                      |  0  | VSM | FOG |  0  | SKN | SRE | DYN | DIR |    64 - 24 = 40
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //      Vertex shader            0     0     0     X     X     X     X
        //    Fragment shader            X     X     0     0     X     X     X
        //           Reserved            X     X     0     X     1     0     0      [ -8]
        //           Reserved            1     X     0     X     0     X     X      [-16]
        //
        // Depth variants:
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //                      |  0  | VSM | PCK |  1  | SKN |  0  |  0  |  0  |   8 - 2 = 6
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //       Vertex depth            X     0     1     X     0     0     0
        //     Fragment depth            X     X     1     0     0     0     0
        //           Reserved            1     1     1     X     0     0     0     [ -2]
        //
        // 46 variants used, 82 reserved (128 - 46)
        //
        // note: a valid variant can be neither a valid vertex nor a valid fragment variant
        //       (e.g.: FOG|SKN variants), the proper bits are filtered appropriately,
        //       see filterVariantVertex(), filterVariantFragment().

        uint8_t key = 0;

        // when adding more bits, update FRenderer::CommandKey::draw::materialVariant as needed
        // when adding more bits, update VARIANT_COUNT
        static constexpr uint8_t DIR   = 0x01; // directional light present, per frame/world position
        static constexpr uint8_t DYN   = 0x02; // point, spot or area present, per frame/world position
        static constexpr uint8_t SRE   = 0x04; // receives shadows, per renderable
        static constexpr uint8_t SKN   = 0x08; // GPU skinning and/or morphing
        static constexpr uint8_t DEP   = 0x10; // depth only variants
        static constexpr uint8_t FOG   = 0x20; // fog (standard)
        static constexpr uint8_t PCK   = 0x20; // picking (depth)
        static constexpr uint8_t VSM   = 0x40; // variance shadow maps

        static constexpr uint8_t STANDARD_MASK      = DEP;
        static constexpr uint8_t STANDARD_VARIANT   = 0u;

        // the depth variant deactivates all variants that make no sense when writing the depth
        // only -- essentially, all fragment-only variants.
        static constexpr uint8_t DEPTH_MASK         = DEP | SRE | DYN | DIR;
        static constexpr uint8_t DEPTH_VARIANT      = DEP;

        // this mask filters out the lighting variants
        static constexpr uint8_t UNLIT_MASK         = SKN | FOG;

        // returns raw variant bits
        inline bool hasDirectionalLighting() const noexcept { return key & DIR; }
        inline bool hasDynamicLighting() const noexcept     { return key & DYN; }
        inline bool hasShadowReceiver() const noexcept      { return key & SRE; }
        inline bool hasSkinningOrMorphing() const noexcept  { return key & SKN; }
        inline bool hasDepth() const noexcept               { return key & DEP; }
        inline bool hasFog() const noexcept                 { return key & FOG; }
        inline bool hasPicking() const noexcept             { return key & PCK; }
        inline bool hasVsm() const noexcept                 { return key & VSM; }

        inline void setDirectionalLighting(bool v) noexcept { set(v, DIR); }
        inline void setDynamicLighting(bool v) noexcept     { set(v, DYN); }
        inline void setShadowReceiver(bool v) noexcept      { set(v, SRE); }
        inline void setSkinning(bool v) noexcept            { set(v, SKN); }
        inline void setFog(bool v) noexcept                 { set(v, FOG); }
        inline void setPicking(bool v) noexcept             { set(v, PCK); }
        inline void setVsm(bool v) noexcept                 { set(v, VSM); }

        inline static constexpr bool isValidDepthVariant(uint8_t variantKey) noexcept {
            // Can't have VSM and PICKING together with DEPTH variants
            constexpr uint8_t RESERVED_MASK  = VSM | PCK | DEP | SRE | DYN | DIR;
            constexpr uint8_t RESERVED_VALUE = VSM | PCK | DEP;
            return ((variantKey & DEPTH_MASK) == DEPTH_VARIANT) &&
                   ((variantKey & RESERVED_MASK) != RESERVED_VALUE);
       }

        inline static constexpr bool isValidStandardVariant(uint8_t variantKey) noexcept {
            // can't have shadow receiver if we don't have any lighting
            constexpr uint8_t RESERVED0_MASK  = SRE | DYN | DIR;
            constexpr uint8_t RESERVED0_VALUE = SRE;
            // can't have VSM without shadow receiver
            constexpr uint8_t RESERVED1_MASK  = VSM | SRE;
            constexpr uint8_t RESERVED1_VALUE = VSM;
            return ((variantKey & STANDARD_MASK) == STANDARD_VARIANT) &&
                   ((variantKey & RESERVED0_MASK) != RESERVED0_VALUE) &&
                   ((variantKey & RESERVED1_MASK) != RESERVED1_VALUE);
        }

        static constexpr bool isReserved(uint8_t variantKey) noexcept {
            return !isValidStandardVariant(variantKey) && !isValidDepthVariant(variantKey);
        }

        static constexpr uint8_t filterVariantVertex(uint8_t variantKey) noexcept {
            // filter out vertex variants that are not needed. For e.g. fog doesn't affect the
            // vertex shader.
            if ((variantKey & STANDARD_MASK) == STANDARD_VARIANT) {
                return variantKey & (SKN | SRE | DYN | DIR);
            }
            if ((variantKey & DEPTH_MASK) == DEPTH_VARIANT) {
                // Only VSM and skinning affects the vertex shader's DEPTH variant
                return variantKey & (VSM | SKN | DEP);
            }
            return 0;
        }

        static constexpr uint8_t filterVariantFragment(uint8_t variantKey) noexcept {
            // filter out fragment variants that are not needed. For e.g. skinning doesn't
            // affect the fragment shader.
            if ((variantKey & STANDARD_MASK) == STANDARD_VARIANT) {
                return variantKey & (VSM | FOG | SRE | DYN | DIR);
            }
            if ((variantKey & DEPTH_MASK) == DEPTH_VARIANT) {
                // Only VSM & PICKING affects the fragment shader's DEPTH variant
                return variantKey & (VSM | PCK | DEP);
            }
            return 0;
        }

        static constexpr uint8_t filterVariant(uint8_t variantKey, bool isLit) noexcept {
            // special case for depth variant
            if (isValidDepthVariant(variantKey)) {
                return variantKey;
            }
            if (!isLit) {
                // when the shading mode is unlit, remove all the lighting variants
                return variantKey & UNLIT_MASK;
            }
            // if shadow receiver is disabled, turn off VSM
            if (!(variantKey & SRE)) {
                return variantKey & ~VSM;
            }
            return variantKey;
        }

    private:
        inline void set(bool v, uint8_t mask) noexcept {
            key = (key & ~mask) | (v ? mask : uint8_t(0));
        }
    };

namespace details {

// compile time sanity-check tests

constexpr inline bool reserved_is_not_valid() noexcept {
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        bool is_valid = Variant::isValidDepthVariant(i) ||
                Variant::isValidStandardVariant(i);
        bool is_reserved = Variant::isReserved(i);
        if (is_valid == is_reserved) {
            return false;
        }
    }
    return true;
}

constexpr inline size_t reserved_variant_count() noexcept {
    size_t count = 0;
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        if (Variant::isReserved(i)) { count++; }
    }
    return count;
}

constexpr inline size_t valid_variant_count() noexcept {
    size_t count = 0;
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        if (Variant::isValidDepthVariant(i) ||
            Variant::isValidStandardVariant(i)) {
            count++;
        }
    }
    return count;
}

constexpr inline size_t vertex_variant_count() noexcept {
    size_t count = 0;
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        if (Variant::isValidDepthVariant(i) ||
            Variant::isValidStandardVariant(i)) {
            if (Variant::filterVariantVertex(i) == i) {
                count++;
            }
        }
    }
    return count;
}

constexpr inline size_t fragment_variant_count() noexcept {
    size_t count = 0;
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        if (Variant::isValidDepthVariant(i) ||
            Variant::isValidStandardVariant(i)) {
            if (Variant::filterVariantFragment(i) == i) {
                count++;
            }
        }
    }
    return count;
}

static_assert(reserved_is_not_valid());
static_assert(reserved_variant_count() == 82);
static_assert(valid_variant_count()    == 46);
static_assert(vertex_variant_count()   == 16 - (2 + 0) + 4 - 0);    // 18
static_assert(fragment_variant_count() == 32 - (4 + 8) + 4 - 1);    // 25

} // namespace details
} // namespace filament

#endif
