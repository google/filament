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

#ifndef TNT_FILAMENT_BACKEND_WEBGPU_WASM_POLYFILL_H
#define TNT_FILAMENT_BACKEND_WEBGPU_WASM_POLYFILL_H

#if defined(__EMSCRIPTEN__)

#include <webgpu/webgpu_cpp.h>
#include <cstdint>

namespace wgpu {

struct Extent2D {
    uint32_t width = 0;
    uint32_t height = 0;
    operator Extent3D() const { return {width, height, 1}; }
};

struct Origin2D {
    uint32_t x = 0;
    uint32_t y = 0;
    operator Origin3D() const { return {x, y, 0}; }
};

// b/508270158
enum class ComponentSwizzle : uint32_t {
    Undefined = 0,
    Zero = 1,
    One = 2,
    R = 3,
    G = 4,
    B = 5,
    A = 6,
};

struct TextureComponentSwizzle {
    ComponentSwizzle r = ComponentSwizzle::Undefined;
    ComponentSwizzle g = ComponentSwizzle::Undefined;
    ComponentSwizzle b = ComponentSwizzle::Undefined;
    ComponentSwizzle a = ComponentSwizzle::Undefined;
};

struct DawnTogglesDescriptor : public ChainedStruct {
    uint32_t enabledToggleCount = 0;
    const char* const* enabledToggles = nullptr;
};

struct DawnAdapterPropertiesPowerPreference : public ChainedStructOut {
    PowerPreference powerPreference = PowerPreference::Undefined;
};

} // namespace wgpu

#endif // __EMSCRIPTEN__

#endif // TNT_FILAMENT_BACKEND_WEBGPU_WASM_POLYFILL_H
