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

namespace filament {
namespace backend {
namespace metal {

struct MetalContext;

class MetalBlitter {

public:

    explicit MetalBlitter(MetalContext& context) noexcept;

    struct BlitArgs {
        struct Attachment {
            id<MTLTexture> color = nil;
            id<MTLTexture> depth = nil;
            MTLRegion region;
            uint8_t level = 0;
        };

        Attachment source, destination;
        SamplerMagFilter filter;

        bool colorDestinationIsFullAttachment() const {
            return destination.color.width == destination.region.size.width &&
                   destination.color.height == destination.region.size.height;
        }

        bool depthDestinationIsFullAttachment() const {
            return destination.depth.width == destination.region.size.width &&
                   destination.depth.height == destination.region.size.height;
        }
    };

    void blit(const BlitArgs& args);

    /**
     * Free global resources. Should be called at least once per process when no further calls to
     * blit will occur.
     */
    static void shutdown() noexcept;

private:

    static void setupColorAttachment(const BlitArgs& args, MTLRenderPassDescriptor* descriptor);
    static void setupDepthAttachment(const BlitArgs& args, MTLRenderPassDescriptor* descriptor);
    void ensureFunctions();

    MetalContext& mContext;

};

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_METALBLITTER_H
