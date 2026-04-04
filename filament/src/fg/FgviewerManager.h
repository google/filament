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

#ifndef TNT_FILAMENT_FGVIEWERMANAGER_H
#define TNT_FILAMENT_FGVIEWERMANAGER_H

#include <backend/Handle.h>

#include <cstdint>

namespace filament {

class FEngine;
class FView;
class FrameGraph;

class FgviewerManager {
public:
#if FILAMENT_ENABLE_FGVIEWER
    static void handleFgReadbacks(FEngine& engine, FrameGraph& fg,
            backend::Handle<backend::HwRenderTarget> viewTarget, uint32_t viewId);
    static void update(FEngine& engine, FrameGraph& fg, FView const& view);
    static void tick(FEngine& engine);
#else
    static void handleFgReadbacks(FEngine& engine, FrameGraph& fg,
            backend::Handle<backend::HwRenderTarget> viewTarget, uint32_t viewId) {}
    static void update(FEngine& engine, FrameGraph& fg, FView const& view) {}
    static void tick(FEngine& engine) {}
#endif
};

} // namespace filament

#endif // TNT_FILAMENT_FGVIEWERMANAGER_H
