//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3D11FormatTablesTest:
//   Tests to validate our D3D11 support tables match hardware support.
//

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/dxgi_support_table.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace
{

class D3D11FormatTablesTest : public ANGLETest<>
{};

// Hack the angle!
rx::Context11 *HackANGLE(EGLDisplay dpy, EGLContext ctx)
{
    egl::Display *display   = static_cast<egl::Display *>(dpy);
    gl::ContextID contextID = {static_cast<GLuint>(reinterpret_cast<uintptr_t>(ctx))};
    gl::Context *context    = display->getContext(contextID);
    return rx::GetImplAs<rx::Context11>(context);
}

// This test enumerates all GL formats - for each, it queries the D3D support for
// using it as a texture, a render target, and sampling from it in the shader. It
// checks this against our speed-optimized baked tables, and validates they would
// give the same result.
TEST_P(D3D11FormatTablesTest, TestFormatSupport)
{
    ASSERT_EQ(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, GetParam().getRenderer());

    rx::Context11 *context11 =
        HackANGLE(getEGLWindow()->getDisplay(), getEGLWindow()->getContext());
    rx::Renderer11 *renderer = context11->getRenderer();
    const auto &textureCaps  = renderer->getNativeTextureCaps();

    ID3D11Device *device = renderer->getDevice();

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const rx::d3d11::Format &formatInfo =
            rx::d3d11::Format::Get(internalFormat, renderer->getRenderer11DeviceCaps());
        const auto &textureInfo = textureCaps.get(internalFormat);

        // Bits for texturing
        const gl::InternalFormat &internalFormatInfo =
            gl::GetSizedInternalFormatInfo(internalFormat);

        UINT texSupportMask = D3D11_FORMAT_SUPPORT_TEXTURE2D;
        if (internalFormatInfo.depthBits == 0 && internalFormatInfo.stencilBits == 0)
        {
            texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURECUBE;
            if (GetParam().majorVersion > 2)
            {
                texSupportMask |= D3D11_FORMAT_SUPPORT_TEXTURE3D;
            }
        }

        UINT texSupport  = 0;
        bool texSuccess  = SUCCEEDED(device->CheckFormatSupport(formatInfo.texFormat, &texSupport));
        bool textureable = texSuccess && ((texSupport & texSupportMask) == texSupportMask);
        EXPECT_EQ(textureable, textureInfo.texturable) << " for " << gl::FmtHex(internalFormat);

        // Bits for mipmap auto-gen.
        bool expectedMipGen = texSuccess && ((texSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0);
        auto featureLevel   = renderer->getRenderer11DeviceCaps().featureLevel;
        const auto &dxgiSupport = rx::d3d11::GetDXGISupport(formatInfo.texFormat, featureLevel);
        bool actualMipGen =
            ((dxgiSupport.alwaysSupportedFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN) != 0);
        EXPECT_EQ(0u, dxgiSupport.optionallySupportedFlags & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN)
            << " for " << gl::FmtHex(internalFormat);
        EXPECT_EQ(expectedMipGen, actualMipGen) << " for " << gl::FmtHex(internalFormat);

        // Bits for filtering
        UINT filterSupport = 0;
        bool filterSuccess =
            SUCCEEDED(device->CheckFormatSupport(formatInfo.srvFormat, &filterSupport));
        bool filterable =
            filterSuccess && ((filterSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) != 0);
        EXPECT_EQ(filterable, textureInfo.filterable) << " for " << gl::FmtHex(internalFormat);

        // Bits for renderable
        bool renderable          = false;
        UINT renderSupport       = 0u;
        DXGI_FORMAT renderFormat = DXGI_FORMAT_UNKNOWN;
        if (internalFormatInfo.depthBits > 0 || internalFormatInfo.stencilBits > 0)
        {
            renderFormat = formatInfo.dsvFormat;
            bool depthSuccess =
                SUCCEEDED(device->CheckFormatSupport(formatInfo.dsvFormat, &renderSupport));
            renderable =
                depthSuccess && ((renderSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL) != 0);
            if (renderable)
            {
                EXPECT_NE(DXGI_FORMAT_UNKNOWN, formatInfo.dsvFormat)
                    << " for " << gl::FmtHex(internalFormat);
            }
        }
        else
        {
            renderFormat = formatInfo.rtvFormat;
            bool rtSuccess =
                SUCCEEDED(device->CheckFormatSupport(formatInfo.rtvFormat, &renderSupport));
            renderable = rtSuccess && ((renderSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET) != 0);
            if (renderable)
            {
                EXPECT_NE(DXGI_FORMAT_UNKNOWN, formatInfo.rtvFormat)
                    << " for " << gl::FmtHex(internalFormat);
            }
        }
        EXPECT_EQ(renderable, textureInfo.textureAttachment)
            << " for " << gl::FmtHex(internalFormat);
        EXPECT_EQ(renderable, textureInfo.renderbuffer) << " for " << gl::FmtHex(internalFormat);
        if (!textureInfo.sampleCounts.empty())
        {
            EXPECT_TRUE(renderable) << " for " << gl::FmtHex(internalFormat);
        }

        // Multisample counts
        if (renderable)
        {
            if ((renderSupport & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET) != 0)
            {
                EXPECT_TRUE(!textureInfo.sampleCounts.empty());
                for (unsigned int sampleCount = 1;
                     sampleCount <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; sampleCount *= 2)
                {
                    UINT qualityCount    = 0;
                    bool sampleSuccess   = SUCCEEDED(device->CheckMultisampleQualityLevels(
                          renderFormat, sampleCount, &qualityCount));
                    GLuint expectedCount = (!sampleSuccess || qualityCount == 0) ? 0 : 1;
                    EXPECT_EQ(expectedCount, textureInfo.sampleCounts.count(sampleCount))
                        << " for " << gl::FmtHex(internalFormat);
                }
            }
            else
            {
                EXPECT_TRUE(textureInfo.sampleCounts.empty())
                    << " for " << gl::FmtHex(internalFormat);
            }
        }
    }
}

// This test validates that all DXGI_FORMATs can be potentially resized without crashes.
TEST_P(D3D11FormatTablesTest, TestFormatMakeValidSize)
{
    rx::Context11 *context11 =
        HackANGLE(getEGLWindow()->getDisplay(), getEGLWindow()->getContext());
    rx::Renderer11 *renderer = context11->getRenderer();

    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const rx::d3d11::Format &formatInfo =
            rx::d3d11::Format::Get(internalFormat, renderer->getRenderer11DeviceCaps());

        std::array<bool, 2> isImages = {false, true};
        for (auto &image : isImages)
        {
            int reqWidth  = 32;
            int reqHeight = 32;
            int level     = 0;

            rx::d3d11::MakeValidSize(image, formatInfo.texFormat, &reqWidth, &reqHeight, &level);
        }
    }
}

ANGLE_INSTANTIATE_TEST(D3D11FormatTablesTest,
                       ES2_D3D11_FL10_0(),
                       ES2_D3D11_FL10_1(),
                       ES2_D3D11_FL11_0());

}  // anonymous namespace
