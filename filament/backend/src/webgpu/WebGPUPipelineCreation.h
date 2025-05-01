/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUPIPELINECREATION_H
#define TNT_FILAMENT_BACKEND_WEBGPUPIPELINECREATION_H

#include <cstdint>

namespace wgpu {
class Device;
class PipelineLayout;
class RenderPipeline;
enum class TextureFormat : uint32_t;
}// namespace wgpu

namespace filament::backend {

struct PolygonOffset;
enum class PrimitiveType : uint8_t;
struct RasterState;
struct StencilState;

class WGPUVertexBufferInfo;
class WGPUProgram;

[[nodiscard]] wgpu::RenderPipeline createWebGPURenderPipeline(wgpu::Device const&,
        WGPUProgram const&, WGPUVertexBufferInfo const&, wgpu::PipelineLayout const&,
        RasterState const&, StencilState const&, PolygonOffset const&, PrimitiveType,
        wgpu::TextureFormat colorFormat, wgpu::TextureFormat depthFormat);

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUPIPELINECREATION_H
