/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "ImageExpectations.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "TrianglePrimitive.h"

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

TEST_F(BackendTest, TestTemplate) {
    constexpr int kRenderTargetSize = 512;

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);
    RenderTargetHandle renderTarget = addCleanup(api.createDefaultRenderTarget());

    Shader shader = SharedShaders::makeShader(api, *mCleanup, ShaderRequest{
            .mVertexType = VertexShaderType::Simple,
            .mFragmentType = FragmentShaderType::SolidColored,
            .mUniformType = ShaderUniformType::Simple,
    });
    TrianglePrimitive triangle(api);

    RenderPassParams params = getClearColorRenderPass();
    params.viewport = getFullViewport();

    PipelineState ps = getColorWritePipelineState();
    shader.addProgramToPipelineState(ps);

    auto ubuffer = addCleanup(api.createBufferObject(sizeof(SimpleMaterialParams),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    shader.uploadUniform(api, ubuffer, SimpleMaterialParams{
            .color = float4(1, 0, 0, 1),
            .scaleMinusOne = float4(0, 0, -0.5, 0),
            .offset = float4(0, 0, 0, 0),
    });
    shader.bindUniform<SimpleMaterialParams>(api, ubuffer);

    {
        RenderFrame frame(api);

        api.beginRenderPass(renderTarget, params);
        ps.primitiveType = PrimitiveType::TRIANGLES;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        api.bindPipeline(ps);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());
        api.draw2(0, 3, 1);
        api.endRenderPass();

        EXPECT_IMAGE(renderTarget,
                ScreenshotParams(kRenderTargetSize, kRenderTargetSize, "TestTemplate", 1048576));

        api.commit(swapChain);
    }
}

} // namespace test
