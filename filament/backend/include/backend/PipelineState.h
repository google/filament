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

#ifndef TNT_FILAMENT_BACKEND_PIPELINESTATE_H
#define TNT_FILAMENT_BACKEND_PIPELINESTATE_H

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/ostream.h>

#include <stdint.h>

namespace filament::backend {

//! \privatesection

struct PipelineState {
    Handle<HwProgram> program;
    RasterState rasterState;
    StencilState stencilState;
    PolygonOffset polygonOffset;
    Viewport scissor{ 0, 0, (uint32_t)INT32_MAX, (uint32_t)INT32_MAX };
};

} // namespace filament::backend

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const filament::backend::PipelineState& ps);
#endif

#endif //TNT_FILAMENT_BACKEND_PIPELINESTATE_H
