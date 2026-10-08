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

#include "BackendTest.h"
#include "Lifetimes.h"
#include "Shader.h"
#include "SharedShaders.h"
#include "Skip.h"
#include "TrianglePrimitive.h"
#include "Workarounds.h"

#include <backend/PixelBufferDescriptor.h>

#include <math/vec2.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

namespace {

using Pixels = std::vector<uint8_t>;

constexpr uint32_t kSize = 256;

class ViewportClearedTest : public BackendTest {
protected:
    void SetUp() override {
        auto& api = getDriverApi();
        mSwapChain = addCleanup(createSwapChain());
        api.makeCurrent(mSwapChain, mSwapChain);
        mShader.emplace(SharedShaders::makeShader(api, *mCleanup, ShaderRequest{
                .mVertexType = VertexShaderType::Noop,
                .mFragmentType = FragmentShaderType::White,
                .mUniformType = ShaderUniformType::None,
        }));
        // covers the whole viewport
        mCover.emplace(api);
        float2 const vertices[3] = { { -1, -1 }, { 3, -1 }, { -1, 3 } };
        mCover->updateVertices(vertices);
        mPipelineState = getColorWritePipelineState();
        mShader->addProgramToPipelineState(mPipelineState);
        mPipelineState.primitiveType = PrimitiveType::TRIANGLES;
        mPipelineState.vertexBufferInfo = mCover->getVertexBufferInfo();
    }

    RenderTargetHandle createRenderTarget() {
        auto& api = getDriverApi();
        TextureHandle const texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::RGBA8, 1, kSize, kSize, 1,
                TextureUsage::COLOR_ATTACHMENT TEXTURE_USAGE_READ_PIXELS));
        return addCleanup(api.createRenderTarget(TargetBufferFlags::COLOR, kSize, kSize, 1, 0,
                { { texture } }, {}, {}));
    }

    // Runs one render pass, which paints its viewport white if `paint`, and reads back `rt`.
    Pixels render(RenderTargetHandle rt, RenderPassParams const& params, bool paint) {
        auto& api = getDriverApi();
        Pixels pixels(kSize * kSize * 4, 0);
        {
            RenderFrame frame(api);
            api.beginRenderPass(rt, params);
            if (paint) {
                api.bindPipeline(mPipelineState);
                api.bindRenderPrimitive(mCover->getRenderPrimitive());
                api.draw2(0, 3, 1);
            }
            api.endRenderPass();
            api.readPixels(rt, 0, 0, kSize, kSize, PixelBufferDescriptor(pixels.data(),
                    pixels.size(), PixelDataFormat::RGBA, PixelDataType::UBYTE));
            api.commit(mSwapChain);
        }
        flushAndWait();
        return pixels;
    }

    SwapChainHandle mSwapChain;
    std::optional<Shader> mShader;
    std::optional<TrianglePrimitive> mCover;
    PipelineState mPipelineState;
};

} // namespace

// The viewport is cleared to blue rather than to what it holds, so that the clear shows: it must
// cover exactly the viewport, and leave everything outside of it as it was.
TEST_F(ViewportClearedTest, ClearsOnlyTheViewport) {
    SKIP_IF_NOT(Backend::VULKAN, "only Vulkan acts on viewportCleared");

    RenderPassParams paint = getClearColorDepthRenderPass({ 1, 0, 0, 1 });
    uint8_t const blue[4] = { 0, 0, 255, 255 };
    // the top right quarter, and one that hangs over the bottom and right edges
    for (Viewport const viewport : { Viewport{ kSize / 2, kSize / 2, kSize / 2, kSize / 2 },
            Viewport{ kSize / 2, -int32_t(kSize / 4), kSize, kSize } }) {
        // white inside the viewport, red outside
        paint.viewport = viewport;
        Pixels const inside = render(createRenderTarget(), paint, true);

        // white in the bottom left quarter, red elsewhere
        RenderTargetHandle const rt = createRenderTarget();
        paint.viewport = { 0, 0, kSize / 2, kSize / 2 };
        Pixels const before = render(rt, paint, true);

        RenderPassParams params = {};
        params.flags.viewportCleared = TargetBufferFlags::COLOR;
        params.clearColor = { 0, 0, 1, 1 };
        params.viewport = viewport;
        Pixels const after = render(rt, params, false);

        size_t cleared = 0;
        for (size_t i = 0; i < after.size(); i += 4) {
            bool const isInside = inside[i + 1] == 255;
            uint8_t const* const expected = isInside ? blue : &before[i];
            ASSERT_TRUE(std::equal(expected, expected + 4, &after[i]))
                    << "x=" << (i / 4) % kSize << " y=" << (i / 4) / kSize;
            cleared += isInside;
        }
        // the part of the viewport inside the render target
        EXPECT_EQ(cleared, kSize / 2 * (viewport.bottom < 0 ? kSize * 3 / 4 : kSize / 2));
    }
}

} // namespace test
