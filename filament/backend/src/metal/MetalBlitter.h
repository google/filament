/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_METALBLITTER_H
#define TNT_METALBLITTER_H

#include <Metal/Metal.h>

#include <backend/DriverEnums.h>

#include <tsl/robin_map.h>
#include <utils/Hash.h>

namespace filament::backend {

struct MetalContext;

class MetalBlitter {

public:

    explicit MetalBlitter(MetalContext& context) noexcept;

    enum class FormatType { NONE, COLOR, DEPTH, STENCIL, DEPTH_STENCIL };
    static FormatType getFormatType(TextureFormat format) noexcept {
        bool const depth = isDepthFormat(format);
        bool const stencil = isStencilFormat(format);
        if (depth && stencil) return FormatType::DEPTH_STENCIL;
        if (depth) return FormatType::DEPTH;
        if (stencil) return FormatType::STENCIL;
        return FormatType::COLOR;
    }

    struct BlitArgs {
        struct Attachment {
            using Type = FormatType;
            id<MTLTexture> texture = nil;
            MTLRegion region = {};
            uint8_t level = 0;
            uint32_t slice = 0;      // must be 0 on source attachment
            Type type = Type::NONE;
        };

        // Valid source formats:       2D, 2DArray, 2DMultisample, 3D
        // Valid destination formats:  2D, 2DArray, 3D, Cube
        Attachment source;
        Attachment destination;
        SamplerMagFilter filter;

        bool blitColor() const {
            return source.type == Attachment::Type::COLOR &&
                   destination.type == Attachment::Type::COLOR;
        }

        bool blitDepth() const {
            return source.type == Attachment::Type::DEPTH &&
                   destination.type == Attachment::Type::DEPTH;
        }

        bool destinationIsFullAttachment() const {
            return destination.texture.width == destination.region.size.width &&
                   destination.texture.height == destination.region.size.height;
        }
    };

    void blit(id<MTLCommandBuffer> cmdBuffer, const BlitArgs& args, const char* label);

    /**
     * Free resources. Should be called at least once per process when no further calls to blit will
     * occur.
     */
    void shutdown() noexcept;

private:

    static void setupAttachment(MTLRenderPassAttachmentDescriptor* descriptor,
            const BlitArgs& args, uint32_t depthPlane);

    struct BlitFunctionKey {
        bool blitColor;
        bool blitDepth;
        bool msaaColorSource;
        bool msaaDepthSource;
        bool sources3D;

        char padding[3];

        bool isValid() const noexcept {
            // MSAA 3D textures do not exist.
            bool const hasMsaa = msaaColorSource || msaaDepthSource;
            return !(hasMsaa && sources3D);
        }

        bool operator==(const BlitFunctionKey& rhs) const noexcept {
            return blitColor == rhs.blitColor &&
                   blitDepth == rhs.blitDepth &&
                   msaaColorSource == rhs.msaaColorSource &&
                   msaaDepthSource == rhs.msaaDepthSource &&
                   sources3D == rhs.sources3D;
        }

        BlitFunctionKey() {
            std::memset(this, 0, sizeof(BlitFunctionKey));
        }
    };

    static bool blitFastPath(id<MTLCommandBuffer> cmdBuffer,
            const BlitArgs& args, const char* label);

    void blitSlowPath(id<MTLCommandBuffer> cmdBuffer,
            const BlitArgs& args, const char* label);

    void blitDepthPlane(id<MTLCommandBuffer> cmdBuffer, bool blitColor, bool blitDepth,
            const BlitArgs& args, uint32_t depthPlaneSource, uint32_t depthPlaneDest,
            const char* label);

    id<MTLFunction> compileFragmentFunction(BlitFunctionKey key) const;
    id<MTLFunction> getBlitVertexFunction();
    id<MTLFunction> getBlitFragmentFunction(BlitFunctionKey key);

    MetalContext& mContext;

    using HashFn = utils::hash::MurmurHashFn<BlitFunctionKey>;
    using Function = id<MTLFunction>;
    tsl::robin_map<BlitFunctionKey, Function, HashFn> mBlitFunctions;

    id<MTLFunction> mVertexFunction = nil;
};

} // namespace filament::backend


#endif //TNT_METALBLITTER_H
