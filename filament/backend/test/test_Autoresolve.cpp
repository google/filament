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

#include "BackendTest.h"

#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

#include <backend/PixelBufferDescriptor.h>

#include <utils/FixedCapacityVector.h>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

TEST_F(BackendTest, AutoresolveDifferingSampleCounts) {
    auto& api = getDriverApi();
    constexpr int kRenderTargetSize = 512;

    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    Shader shader = SharedShaders::makeShader(api, *mCleanup,
            {
                .mVertexType = VertexShaderType::Simple,
                .mFragmentType = FragmentShaderType::SolidColored,
                .mUniformType = ShaderUniformType::Simple,
            });

    TrianglePrimitive triangle(api);
    PipelineState ps = getColorWritePipelineState();
    shader.addProgramToPipelineState(ps);

    auto ubuffer = addCleanup(api.createBufferObject(sizeof(SimpleMaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    shader.bindUniform<SimpleMaterialParams>(api, ubuffer);

    // Create a texture with sample count = 1.
    Handle<HwTexture> texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
            TextureFormat::RGBA8, 1, kRenderTargetSize, kRenderTargetSize, 1,
            TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE));

    // First render pass: render a red triangle to a RenderTarget with 8 samples.
    {
        Handle<HwRenderTarget> renderTarget =
                addCleanup(api.createRenderTarget(TargetBufferFlags::COLOR, kRenderTargetSize,
                        kRenderTargetSize, 8, 1, { { texture } }, {}, {}));

        shader.uploadUniform(api, ubuffer,
                SimpleMaterialParams{
                    .color = float4(1, 0, 0, 1),
                });

        RenderPassParams params = getClearColorRenderPass(float4(0));
        params.viewport = { 0, 0, kRenderTargetSize, kRenderTargetSize };

        RenderFrame frame(api);
        api.beginRenderPass(renderTarget, params);
        ps.primitiveType = PrimitiveType::TRIANGLES;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();
        api.commit(swapChain);
    }

    // Second render pass: render a green triangle to a RenderTarget with 4 samples, attached to
    // the same texture.
    {
        Handle<HwRenderTarget> renderTarget =
                addCleanup(api.createRenderTarget(TargetBufferFlags::COLOR, kRenderTargetSize,
                        kRenderTargetSize, 4, 1, { { texture } }, {}, {}));

        shader.uploadUniform(api, ubuffer,
                SimpleMaterialParams{
                    .color = float4(0, 1, 0, 1),
                });

        RenderPassParams params = getClearColorRenderPass(float4(0));
        params.viewport = { 0, 0, kRenderTargetSize, kRenderTargetSize };

        RenderFrame frame(api);
        api.beginRenderPass(renderTarget, params);
        ps.primitiveType = PrimitiveType::TRIANGLES;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(renderTarget, ScreenshotParams(kRenderTargetSize, kRenderTargetSize,
                                           "AutoresolveDifferingSampleCounts", 1048576));

        api.commit(swapChain);
    }
}

} // namespace test
