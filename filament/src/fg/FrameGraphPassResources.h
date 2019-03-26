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

#ifndef TNT_FILAMENT_FRAMEGRAPHPASSRESOURCES_H
#define TNT_FILAMENT_FRAMEGRAPHPASSRESOURCES_H

#include "FrameGraphResource.h"

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

namespace filament {

namespace fg {
struct PassNode;
} // namespace fg

class FrameGraph;

class FrameGraphPassResources {
public:

    struct RenderTargetInfo {
        backend::Handle<backend::HwRenderTarget> target;
        backend::RenderPassParams params;
    };

    backend::Handle<backend::HwTexture> getTexture(FrameGraphResource r) const noexcept;

    RenderTargetInfo getRenderTarget(FrameGraphResource r) const noexcept;

    FrameGraphResource::Descriptor const& getDescriptor(FrameGraphResource r) const noexcept;

private:
    friend class FrameGraph;
    explicit FrameGraphPassResources(FrameGraph& fg, fg::PassNode const& pass) noexcept;
    FrameGraph& mFrameGraph;
    fg::PassNode const& mPass;
};

} // namespace filament

#endif //TNT_FILAMENT_FRAMEGRAPHPASSRESOURCES_H
