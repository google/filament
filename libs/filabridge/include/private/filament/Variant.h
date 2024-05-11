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

#ifndef TNT_FILABRIDGE_VARIANT_H
#define TNT_FILABRIDGE_VARIANT_H

#include <filament/MaterialEnums.h>

#include <utils/bitset.h>
#include <utils/Slice.h>

#include <stdint.h>
#include <stddef.h>

namespace filament {
static constexpr size_t VARIANT_BITS = 8;
static constexpr size_t VARIANT_COUNT = 1 << VARIANT_BITS;

using VariantList = utils::bitset<uint64_t, VARIANT_COUNT / 64>;

// IMPORTANT: update filterVariant() when adding more variants
// Also be sure to update formatVariantString inside CommonWriter.cpp
struct Variant {
    using type_t = uint8_t;

    Variant() noexcept = default;
    Variant(Variant const& rhs) noexcept = default;
    Variant& operator=(Variant const& rhs) noexcept = default;
    constexpr explicit Variant(type_t key) noexcept : key(key) { }


    // DIR: Directional Lighting
    // DYN: Dynamic Lighting
    // SRE: Shadow Receiver
    // SKN: Skinning
    // DEP: Depth only
    // FOG: Fog
    // PCK: Picking (depth variant only)
    // VSM: Variance shadow maps
    // STE: Instanced stereo rendering
    //
    //   X: either 1 or 0
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    // Variant              | STE | VSM | FOG | DEP | SKN | SRE | DYN | DIR |   256
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    //                                    PCK
    //
    // Standard variants:
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    //                      | STE | VSM | FOG |  0  | SKN | SRE | DYN | DIR |    128 - 44 = 84
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    //      Vertex shader      X     0     0     0     X     X     X     X
    //    Fragment shader      0     X     X     0     0     X     X     X
    //       Fragment SSR      0     1     0     0     0     1     0     0
    //           Reserved      X     1     1     0     X     1     0     0      [ -4]
    //           Reserved      X     0     X     0     X     1     0     0      [ -8]
    //           Reserved      X     1     X     0     X     0     X     X      [-32]
    //
    // Depth variants:
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    //                      | STE | VSM | PCK |  1  | SKN |  0  |  0  |  0  |   16 - 4 = 12
    //                      +-----+-----+-----+-----+-----+-----+-----+-----+
    //       Vertex depth      X     X     0     1     X     0     0     0
    //     Fragment depth      0     X     X     1     0     0     0     0
    //           Reserved      X     1     1     1     X     0     0     0     [  -4]
    //
    // 96 variants used, 160 reserved (256 - 96)
    //
    // note: a valid variant can be neither a valid vertex nor a valid fragment variant
    //       (e.g.: FOG|SKN variants), the proper bits are filtered appropriately,
    //       see filterVariantVertex(), filterVariantFragment().

    type_t key = 0u;

    // when adding more bits, update FRenderer::CommandKey::draw::materialVariant as needed
    // when adding more bits, update VARIANT_COUNT
    static constexpr type_t DIR   = 0x01; // directional light present, per frame/world position
    static constexpr type_t DYN   = 0x02; // point, spot or area present, per frame/world position
    static constexpr type_t SRE   = 0x04; // receives shadows, per renderable
    static constexpr type_t SKN   = 0x08; // GPU skinning and/or morphing
    static constexpr type_t DEP   = 0x10; // depth only variants
    static constexpr type_t FOG   = 0x20; // fog (standard)
    static constexpr type_t PCK   = 0x20; // picking (depth)
    static constexpr type_t VSM   = 0x40; // variance shadow maps
    static constexpr type_t STE   = 0x80; // instanced stereo

    // special variants (variants that use the reserved space)
    static constexpr type_t SPECIAL_SSR   = VSM | SRE; // screen-space reflections variant

    static constexpr type_t STANDARD_MASK      = DEP;
    static constexpr type_t STANDARD_VARIANT   = 0u;

    // the depth variant deactivates all variants that make no sense when writing the depth
    // only -- essentially, all fragment-only variants.
    static constexpr type_t DEPTH_MASK         = DEP | SRE | DYN | DIR;
    static constexpr type_t DEPTH_VARIANT      = DEP;

    // this mask filters out the lighting variants
    static constexpr type_t UNLIT_MASK         = STE | SKN | FOG;

    // returns raw variant bits
    inline bool hasDirectionalLighting() const noexcept { return key & DIR; }
    inline bool hasDynamicLighting() const noexcept     { return key & DYN; }
    inline bool hasSkinningOrMorphing() const noexcept  { return key & SKN; }
    inline bool hasStereo() const noexcept              { return key & STE; }

    inline void setDirectionalLighting(bool v) noexcept { set(v, DIR); }
    inline void setDynamicLighting(bool v) noexcept     { set(v, DYN); }
    inline void setShadowReceiver(bool v) noexcept      { set(v, SRE); }
    inline void setSkinning(bool v) noexcept            { set(v, SKN); }
    inline void setFog(bool v) noexcept                 { set(v, FOG); }
    inline void setPicking(bool v) noexcept             { set(v, PCK); }
    inline void setVsm(bool v) noexcept                 { set(v, VSM); }
    inline void setStereo(bool v) noexcept              { set(v, STE); }

    inline static constexpr bool isValidDepthVariant(Variant variant) noexcept {
        // Can't have VSM and PICKING together with DEPTH variants
        constexpr type_t RESERVED_MASK  = VSM | PCK | DEP | SRE | DYN | DIR;
        constexpr type_t RESERVED_VALUE = VSM | PCK | DEP;
        return ((variant.key & DEPTH_MASK) == DEPTH_VARIANT) &&
               ((variant.key & RESERVED_MASK) != RESERVED_VALUE);
   }

    inline static constexpr bool isValidStandardVariant(Variant variant) noexcept {
        // can't have shadow receiver if we don't have any lighting
        constexpr type_t RESERVED0_MASK  = VSM | FOG | SRE | DYN | DIR;
        constexpr type_t RESERVED0_VALUE = VSM | FOG | SRE;

        // can't have shadow receiver if we don't have any lighting
        constexpr type_t RESERVED1_MASK  = VSM | SRE | DYN | DIR;
        constexpr type_t RESERVED1_VALUE = SRE;

        // can't have VSM without shadow receiver
        constexpr type_t RESERVED2_MASK  = VSM | SRE;
        constexpr type_t RESERVED2_VALUE = VSM;

        return ((variant.key & STANDARD_MASK) == STANDARD_VARIANT) &&
               ((variant.key & RESERVED0_MASK) != RESERVED0_VALUE) &&
               ((variant.key & RESERVED1_MASK) != RESERVED1_VALUE) &&
               ((variant.key & RESERVED2_MASK) != RESERVED2_VALUE);
    }

    inline static constexpr bool isVertexVariant(Variant variant) noexcept {
        return filterVariantVertex(variant) == variant;
    }

    inline static constexpr bool isFragmentVariant(Variant variant) noexcept {
        return filterVariantFragment(variant) == variant;
    }

    static constexpr bool isReserved(Variant variant) noexcept {
        return !isValid(variant);
    }

    static constexpr bool isValid(Variant variant) noexcept {
        return isValidStandardVariant(variant) || isValidDepthVariant(variant);
    }

    inline static constexpr bool isSSRVariant(Variant variant) noexcept {
        return (variant.key & (STE | VSM | DEP | SRE | DYN | DIR)) == (VSM | SRE);
    }

    inline static constexpr bool isVSMVariant(Variant variant) noexcept {
        return !isSSRVariant(variant) && ((variant.key & VSM) == VSM);
    }

    inline static constexpr bool isShadowReceiverVariant(Variant variant) noexcept {
        return !isSSRVariant(variant) && ((variant.key & SRE) == SRE);
    }

    inline static constexpr bool isFogVariant(Variant variant) noexcept {
        return (variant.key & (FOG | DEP)) == FOG;
    }

    inline static constexpr bool isPickingVariant(Variant variant) noexcept {
        return (variant.key & (PCK | DEP)) == (PCK | DEP);
    }

    inline static constexpr bool isStereoVariant(Variant variant) noexcept {
        return (variant.key & STE) == STE;
    }

    static constexpr Variant filterVariantVertex(Variant variant) noexcept {
        // filter out vertex variants that are not needed. For e.g. fog doesn't affect the
        // vertex shader.
        if ((variant.key & STANDARD_MASK) == STANDARD_VARIANT) {
            if (isSSRVariant(variant)) {
                variant.key &= ~(VSM | SRE);
            }
            return variant & (STE | SKN | SRE | DYN | DIR);
        }
        if ((variant.key & DEPTH_MASK) == DEPTH_VARIANT) {
            // Only VSM, skinning, and stereo affect the vertex shader's DEPTH variant
            return variant & (STE | VSM | SKN | DEP);
        }
        return {};
    }

    static constexpr Variant filterVariantFragment(Variant variant) noexcept {
        // filter out fragment variants that are not needed. For e.g. skinning doesn't
        // affect the fragment shader.
        if ((variant.key & STANDARD_MASK) == STANDARD_VARIANT) {
            return variant & (VSM | FOG | SRE | DYN | DIR);
        }
        if ((variant.key & DEPTH_MASK) == DEPTH_VARIANT) {
            // Only VSM & PICKING affects the fragment shader's DEPTH variant
            return variant & (VSM | PCK | DEP);
        }
        return {};
    }

    static constexpr Variant filterVariant(Variant variant, bool isLit) noexcept {
        // special case for depth variant
        if (isValidDepthVariant(variant)) {
            if (!isLit) {
                // if we're unlit, we never need the VSM variant
                return variant & ~VSM;
            }
            return variant;
        }
        if (isSSRVariant(variant)) {
            return variant;
        }
        if (!isLit) {
            // when the shading mode is unlit, remove all the lighting variants
            return variant & UNLIT_MASK;
        }
        // if shadow receiver is disabled, turn off VSM
        if (!(variant.key & SRE)) {
            return variant & ~VSM;
        }
        return variant;
    }

    constexpr bool operator==(Variant rhs) const noexcept {
        return key == rhs.key;
    }

    constexpr bool operator!=(Variant rhs) const noexcept {
        return key != rhs.key;
    }

    constexpr Variant operator & (type_t rhs) const noexcept {
        return Variant(key & rhs);
    }

    static Variant filterUserVariant(
            Variant variant, UserVariantFilterMask filterMask) noexcept;

private:
    void set(bool v, type_t mask) noexcept {
        key = (key & ~mask) | (v ? mask : type_t(0));
    }
};

namespace VariantUtils {
// list of lit variants
utils::Slice<Variant> getLitVariants() noexcept UTILS_PURE;
// list of unlit variants
utils::Slice<Variant> getUnlitVariants() noexcept UTILS_PURE;
}

} // namespace filament

#endif // TNT_FILABRIDGE_VARIANT_H
