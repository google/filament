/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_POSTPROCESS_MANAGER_H
#define TNT_FILAMENT_POSTPROCESS_MANAGER_H

#include "RenderTargetPool.h"

#include "UniformBuffer.h"

#include "driver/DriverApiForward.h"
#include "driver/Handle.h"

#include <filament/Viewport.h>

#include <filament/driver/DriverEnums.h>

#include <vector>

namespace filament {

namespace details {
class FEngine;
class FView;
} // namespace details

class PostProcessManager {
public:
    void init(details::FEngine& engine) noexcept;
    void terminate(driver::DriverApi& driver) noexcept;
    void setSource(uint32_t viewportWidth, uint32_t viewportHeight,
            const RenderTargetPool::Target* pos) const noexcept;

    // start() is a scam, it does nothing
    void start() noexcept { }

    // a fullscreen pass, using the given format as target and writing into the specified program
    void pass(driver::TextureFormat format, Handle<HwProgram> program) noexcept;

    // a blit pass, using the given format as target
    void blit(driver::TextureFormat format = driver::TextureFormat::RGBA8) noexcept;

    void finish(driver::TargetBufferFlags discarded,
            Handle<HwRenderTarget> viewRenderTarget,
            Viewport const& vp,
            RenderTargetPool::Target const* linearTarget,
            Viewport const& svp);


private:
    details::FEngine* mEngine = nullptr;

    struct Command {
        Handle<HwProgram> program = {};
        driver::TextureFormat format;
    };

    std::vector<Command> mCommands;

    // we need only one of these
    mutable UniformBuffer mPostProcessUb;
    Handle<HwSamplerBuffer> mPostProcessSbh;
    Handle<HwUniformBuffer> mPostProcessUbh;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
