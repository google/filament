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
        //                      |  0  | VSM | FOG |  0  | SKN | SRE | DYN | DIR |    64 (-24)
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //      Vertex shader            0     0     0     X     X     X     X
        //    Fragment shader            X     X     0     0     X     X     X
        //           Reserved            X     X     0     X     1     0     0
        //           Reserved            1     X     0     X     0     X     X
        //
        // Depth variants:
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //                      |  0  | VSM | PCK |  1  | SKN |  0  |  0  |  0  |     8 (-58)
        //                      +-----+-----+-----+-----+-----+-----+-----+-----+
        //       Vertex depth            X     0     1     X     0     0     0
        //     Fragment depth            X     X     1     0     0     0     0
        //           Reserved            1     1     1     X     0     0     0
        //           Reserved            X     X     1     X     X     X     1
        //           Reserved            X     X     1     X     X     1     X
        //           Reserved            X     X     1     X     1     X     X
        //
        // 46 variants used, 82 reserved
        //

        uint8_t key = 0;

        // when adding more bits, update FRenderer::CommandKey::draw::materialVariant as needed
        // when adding more bits, update VARIANT_COUNT
        static constexpr uint8_t DIRECTIONAL_LIGHTING   = 0x01; // directional light present, per frame/world position
        static constexpr uint8_t DYNAMIC_LIGHTING       = 0x02; // point, spot or area present, per frame/world position
        static constexpr uint8_t SHADOW_RECEIVER        = 0x04; // receives shadows, per renderable
        static constexpr uint8_t SKINNING_OR_MORPHING   = 0x08; // GPU skinning and/or morphing
        static constexpr uint8_t DEPTH                  = 0x10; // depth only variants
        static constexpr uint8_t FOG                    = 0x20; // fog (standard)
        static constexpr uint8_t PICKING                = 0x20; // picking (depth)
        static constexpr uint8_t VSM                    = 0x40; // variance shadow maps

        static constexpr uint8_t DEPTH_MASK = DIRECTIONAL_LIGHTING |
                                              DYNAMIC_LIGHTING |
                                              SHADOW_RECEIVER |
                                              DEPTH;

        // the depth variant deactivates all variants that make no sense when writing the depth
        // only -- essentially, all fragment-only variants.
        static constexpr uint8_t DEPTH_VARIANT = DEPTH;

        // this mask filters out the lighting variants
        static constexpr uint8_t UNLIT_MASK    = SKINNING_OR_MORPHING | FOG;

        // returns raw variant bits
        inline bool hasDirectionalLighting() const noexcept { return key & DIRECTIONAL_LIGHTING; }
        inline bool hasDynamicLighting() const noexcept { return key & DYNAMIC_LIGHTING; }
        inline bool hasShadowReceiver() const noexcept { return key & SHADOW_RECEIVER; }
        inline bool hasSkinningOrMorphing() const noexcept { return key & SKINNING_OR_MORPHING; }
        inline bool hasDepth() const noexcept { return key & DEPTH; }
        inline bool hasFog() const noexcept { return key & FOG; }
        inline bool hasPicking() const noexcept { return key & PICKING; }
        inline bool hasVsm() const noexcept { return key & VSM; }

        inline void setDirectionalLighting(bool v) noexcept { set(v, DIRECTIONAL_LIGHTING); }
        inline void setDynamicLighting(bool v) noexcept { set(v, DYNAMIC_LIGHTING); }
        inline void setShadowReceiver(bool v) noexcept { set(v, SHADOW_RECEIVER); }
        inline void setSkinning(bool v) noexcept { set(v, SKINNING_OR_MORPHING); }
        inline void setFog(bool v) noexcept { set(v, FOG); }
        inline void setPicking(bool v) noexcept { set(v, PICKING); }
        inline void setVsm(bool v) noexcept { set(v, VSM); }

        inline static constexpr bool isValidDepthVariant(uint8_t variantKey) noexcept {
            // VSM and PICKING are mutually exclusive for DEPTH variants
            return (variantKey & DEPTH_MASK) == DEPTH_VARIANT &&
                    variantKey != 0b1110000u &&
                    variantKey != 0b1111000u;
        }

        static constexpr bool isReserved(uint8_t variantKey) noexcept {
            // reserved variants that should just be skipped
            // 1. If the DEPTH bit is set, then it must be a valid depth variant. Otherwise, the
            // variant is reserved.
            // 2. If SRE is set, either DYN or DIR must also be set (it makes no sense to have
            // shadows without lights).
            // 3. If VSM is set, then SRE must be set.
            return ((variantKey & DEPTH) && !isValidDepthVariant(variantKey)) ||
                    (variantKey & 0b0010111u) == 0b0000100u ||
                    (variantKey & 0b1010100u) == 0b1000000u;
        }

        static constexpr uint8_t filterVariantVertex(uint8_t variantKey) noexcept {
            // filter out vertex variants that are not needed. For e.g. fog doesn't affect the
            // vertex shader.
            if (variantKey & DEPTH) {
                // Only VSM and skinning affects the vertex shader's DEPTH variant
                return variantKey & (DEPTH | SKINNING_OR_MORPHING | VSM);
            }
            return variantKey & (
                    DIRECTIONAL_LIGHTING |
                    DYNAMIC_LIGHTING |
                    SHADOW_RECEIVER |
                    SKINNING_OR_MORPHING);
        }

        static constexpr uint8_t filterVariantFragment(uint8_t variantKey) noexcept {
            // filter out fragment variants that are not needed. For e.g. skinning doesn't
            // affect the fragment shader.
            if (variantKey & DEPTH) {
                // Only VSM & PICKING affects the fragment shader's DEPTH variant
                return variantKey & (DEPTH | VSM | PICKING);
            }
            return variantKey & (
                    DIRECTIONAL_LIGHTING |
                    DYNAMIC_LIGHTING |
                    SHADOW_RECEIVER |
                    FOG |
                    VSM);
        }

        static constexpr uint8_t filterVariant(uint8_t variantKey, bool isLit) noexcept {
            // special case for depth variant
            if (isValidDepthVariant(variantKey)) {
                return variantKey;
            }
            // when the shading mode is unlit, remove all the lighting variants
            return isLit ? variantKey : (variantKey & UNLIT_MASK);
        }

    private:
        inline void set(bool v, uint8_t mask) noexcept {
            key = (key & ~mask) | (v ? mask : uint8_t(0));
        }
    };

namespace details {
// this ensures at compile time that we are correctly triaging the reserved variants
constexpr inline size_t valid_variant_count() noexcept {
    size_t count = 0;
    for (uint8_t i = 0; i < VARIANT_COUNT; i++) {
        if (!Variant::isReserved(i)) { count++; }
    }
    return count;
}
static_assert(valid_variant_count() == 46);
} // namespace details

} // namespace filament

#endif
