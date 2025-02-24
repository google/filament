//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ConformanceTests.cpp:
//   GLES1 conformance tests.
//   Function prototypes taken from tproto.h and turned into gtest tests using a macro.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#ifdef __cplusplus
extern "C" {
#endif

// ES 1.0
extern long AmbLightExec(void);
extern long AmbMatExec(void);
extern long AmbSceneExec(void);
extern long APFuncExec(void);
extern long AtnConstExec(void);
extern long AtnPosExec(void);
extern long BClearExec(void);
extern long BColorExec(void);
extern long BCornerExec(void);
extern long BlendExec(void);
extern long ClipExec(void);
extern long ColRampExec(void);
extern long CopyTexExec(void);
extern long DifLightExec(void);
extern long DifMatExec(void);
extern long DifMatNormExec(void);
extern long DifMatPosExec(void);
extern long DitherExec(void);
extern long DivZeroExec(void);
extern long EmitMatExec(void);
extern long FogExpExec(void);
extern long FogLinExec(void);
extern long LineAntiAliasExec(void);
extern long LineHVExec(void);
extern long LineRasterExec(void);
extern long LogicOpExec(void);
extern long MipExec(void);
extern long MipLevelsExec(void);
extern long MipLinExec(void);
extern long MipSelectExec(void);
extern long MaskExec(void);
extern long MatrixStackExec(void);
extern long MultiTexExec(void);
extern long MustPassExec(void);
extern long PackedPixelsExec(void);
extern long PointAntiAliasExec(void);
extern long PointRasterExec(void);
extern long PolyCullExec(void);
extern long ReadFormatExec(void);
extern long RescaleNormalExec(void);
extern long ScissorExec(void);
extern long SPClearExec(void);
extern long SPCornerExec(void);
extern long SpecExpExec(void);
extern long SpecExpNormExec(void);
extern long SpecLightExec(void);
extern long SpecMatExec(void);
extern long SpecNormExec(void);
extern long SPFuncExec(void);
extern long SPOpExec(void);
extern long SpotPosExec(void);
extern long SpotExpPosExec(void);
extern long SpotExpDirExec(void);
extern long TexDecalExec(void);
extern long TexPaletExec(void);
extern long TextureEdgeClampExec(void);
extern long TriRasterExec(void);
extern long TriTileExec(void);
extern long VertexOrderExec(void);
extern long ViewportClampExec(void);
extern long XFormExec(void);
extern long XFormMixExec(void);
extern long XFormNormalExec(void);
extern long XFormViewportExec(void);
extern long XFormHomogenousExec(void);
extern long ZBClearExec(void);
extern long ZBFuncExec(void);

// GL_OES_draw_texture
extern long DrawTexExec(void);

// GL_OES_query_matrix
extern long MatrixQueryExec(void);

// ES 1.1
extern long BufferObjectExec(void);
extern long PointSizeArrayExec(void);
extern long PointSpriteExec(void);
extern long UserClipExec(void);
extern long MatrixGetTestExec(void);
extern long GetsExec(void);
extern long TexCombineExec(void);

// GL_OES_matrix_palette
extern long MatrixPaletteExec(void);

// Test driver setup
void BufferSetup(void);
void EpsilonSetup(void);
void StateSetup(void);

#define CONFORMANCE_TEST_ERROR (-1)

#include "conform.h"

#ifdef __cplusplus
}

#endif
namespace angle
{
class GLES1ConformanceTest : public ANGLETest<>
{
  protected:
    GLES1ConformanceTest()
    {
        setWindowWidth(WINDSIZEX);
        setWindowHeight(WINDSIZEY);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        BufferSetup();
        EpsilonSetup();
        StateSetup();

        machine = {};
        // Default parameters taken from shell.c.  Verbosity is increased so test failures come with
        // information.
        machine.randSeed       = 1;
        machine.verboseLevel   = 2;
        machine.stateCheckFlag = GL_TRUE;
    }
};

TEST_P(GLES1ConformanceTest, AmbLight)
{
    // Flaky timeouts due to slow test. http://anglebug.com/42263787
    ANGLE_SKIP_TEST_IF(IsVulkan());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, AmbLightExec());
}

TEST_P(GLES1ConformanceTest, AmbMat)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, AmbMatExec());
}

TEST_P(GLES1ConformanceTest, AmbScene)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, AmbSceneExec());
}

TEST_P(GLES1ConformanceTest, APFunc)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, APFuncExec());
}

TEST_P(GLES1ConformanceTest, AtnConst)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, AtnConstExec());
}

TEST_P(GLES1ConformanceTest, AtnPos)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, AtnPosExec());
}

TEST_P(GLES1ConformanceTest, BClear)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, BClearExec());
}

TEST_P(GLES1ConformanceTest, BColor)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, BColorExec());
}

TEST_P(GLES1ConformanceTest, BCorner)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, BCornerExec());
}

TEST_P(GLES1ConformanceTest, Blend)
{
    // Slow test, takes over 20 seconds in some configs. http://anglebug.com/42263732
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsWindows());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, BlendExec());
}

TEST_P(GLES1ConformanceTest, Clip)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ClipExec());
}

TEST_P(GLES1ConformanceTest, ColRamp)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ColRampExec());
}

TEST_P(GLES1ConformanceTest, CopyTex)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, CopyTexExec());
}

TEST_P(GLES1ConformanceTest, DifLight)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DifLightExec());
}

TEST_P(GLES1ConformanceTest, DifMat)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DifMatExec());
}

TEST_P(GLES1ConformanceTest, DifMatNorm)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DifMatNormExec());
}

TEST_P(GLES1ConformanceTest, DifMatPos)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DifMatPosExec());
}

TEST_P(GLES1ConformanceTest, Dither)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DitherExec());
}

TEST_P(GLES1ConformanceTest, DivZero)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DivZeroExec());
}

TEST_P(GLES1ConformanceTest, EmitMat)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, EmitMatExec());
}

TEST_P(GLES1ConformanceTest, FogExp)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, FogExpExec());
}

TEST_P(GLES1ConformanceTest, FogLin)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, FogLinExec());
}

TEST_P(GLES1ConformanceTest, LineAntiAlias)
{
    ANGLE_SKIP_TEST_IF(true);
    ASSERT_NE(CONFORMANCE_TEST_ERROR, LineAntiAliasExec());
}

TEST_P(GLES1ConformanceTest, LineHV)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, LineHVExec());
}

TEST_P(GLES1ConformanceTest, LineRaster)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, LineRasterExec());
}

TEST_P(GLES1ConformanceTest, LogicOp)
{
    // Only supported if logicOp or framebuffer fetch is supported by the backend.
    //
    // - Desktop GL: has logicOp support
    // - GLES: has framebuffer fetch support
    // - Vulkan: has logicOp support on desktop, and framebuffer fetch support otherwise.
    //    * Non-coherent framebuffer fetch is disabled on Qualcomm due to app bugs, and coherent is
    //      not supported.
    ANGLE_SKIP_TEST_IF(!IsOpenGL() && !IsVulkan());
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsQualcomm());

    ASSERT_NE(CONFORMANCE_TEST_ERROR, LogicOpExec());
}

TEST_P(GLES1ConformanceTest, Mip)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MipExec());
}

TEST_P(GLES1ConformanceTest, MipLevels)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MipLevelsExec());
}

TEST_P(GLES1ConformanceTest, MipLin)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MipLinExec());
}

TEST_P(GLES1ConformanceTest, MipSelect)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MipSelectExec());
}

TEST_P(GLES1ConformanceTest, Mask)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MaskExec());
}

TEST_P(GLES1ConformanceTest, MatrixStack)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MatrixStackExec());
}

TEST_P(GLES1ConformanceTest, MultiTex)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MultiTexExec());
}

TEST_P(GLES1ConformanceTest, MustPass)
{
    ANGLE_SKIP_TEST_IF(true);
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MustPassExec());
}

TEST_P(GLES1ConformanceTest, PackedPixels)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, PackedPixelsExec());
}

TEST_P(GLES1ConformanceTest, PointAntiAlias)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, PointAntiAliasExec());
}

TEST_P(GLES1ConformanceTest, PointRaster)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, PointRasterExec());
}

TEST_P(GLES1ConformanceTest, PolyCull)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, PolyCullExec());
}

TEST_P(GLES1ConformanceTest, ReadFormat)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ReadFormatExec());
}

TEST_P(GLES1ConformanceTest, RescaleNormal)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, RescaleNormalExec());
}

TEST_P(GLES1ConformanceTest, Scissor)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ScissorExec());
}

TEST_P(GLES1ConformanceTest, SPClear)
{
    // http://anglebug.com/42266142
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsVulkan());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SPClearExec());
}

TEST_P(GLES1ConformanceTest, SPCorner)
{
    // http://anglebug.com/42266142
    ANGLE_SKIP_TEST_IF(IsQualcomm());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SPCornerExec());
}

TEST_P(GLES1ConformanceTest, SpecExp)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpecExpExec());
}

TEST_P(GLES1ConformanceTest, SpecExpNorm)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpecExpNormExec());
}

TEST_P(GLES1ConformanceTest, SpecLight)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpecLightExec());
}

TEST_P(GLES1ConformanceTest, SpecMat)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpecMatExec());
}

TEST_P(GLES1ConformanceTest, SpecNorm)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpecNormExec());
}

TEST_P(GLES1ConformanceTest, SPFunc)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SPFuncExec());
}

TEST_P(GLES1ConformanceTest, SPOp)
{
    // http://anglebug.com/42266142
    ANGLE_SKIP_TEST_IF(IsQualcomm());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SPOpExec());
}

TEST_P(GLES1ConformanceTest, SpotPos)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpotPosExec());
}

TEST_P(GLES1ConformanceTest, SpotExpPos)
{
    // Fails on UHD 770 driver 31.0.101.5333 http://anglebug.com/352085732
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsDesktopOpenGL());

    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpotExpPosExec());
}

TEST_P(GLES1ConformanceTest, SpotExpDir)
{
    // Fails on UHD 770 driver 31.0.101.5333 http://anglebug.com/352085732
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsDesktopOpenGL());

    ASSERT_NE(CONFORMANCE_TEST_ERROR, SpotExpDirExec());
}

TEST_P(GLES1ConformanceTest, TexDecal)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TexDecalExec());
}

TEST_P(GLES1ConformanceTest, TexPalet)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TexPaletExec());
}

TEST_P(GLES1ConformanceTest, TextureEdgeClamp)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TextureEdgeClampExec());
}

TEST_P(GLES1ConformanceTest, TriRaster)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TriRasterExec());
}

TEST_P(GLES1ConformanceTest, TriTile)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TriTileExec());
}

TEST_P(GLES1ConformanceTest, VertexOrder)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, VertexOrderExec());
}

TEST_P(GLES1ConformanceTest, ViewportClamp)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ViewportClampExec());
}

TEST_P(GLES1ConformanceTest, XForm)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, XFormExec());
}

TEST_P(GLES1ConformanceTest, XFormMix)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, XFormMixExec());
}

TEST_P(GLES1ConformanceTest, XFormNormal)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, XFormNormalExec());
}

TEST_P(GLES1ConformanceTest, XFormViewport)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, XFormViewportExec());
}

TEST_P(GLES1ConformanceTest, XFormHomogenous)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, XFormHomogenousExec());
}

TEST_P(GLES1ConformanceTest, ZBClear)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ZBClearExec());
}

TEST_P(GLES1ConformanceTest, ZBFunc)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, ZBFuncExec());
}

TEST_P(GLES1ConformanceTest, DrawTex)
{
    // Fails on UHD 770 driver 31.0.101.5333 http://anglebug.com/352085732
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsDesktopOpenGL());

    ASSERT_TRUE(IsGLExtensionEnabled("GL_OES_draw_texture"));
    ASSERT_NE(CONFORMANCE_TEST_ERROR, DrawTexExec());
}

TEST_P(GLES1ConformanceTest, MatrixQuery)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MatrixQueryExec());
}

TEST_P(GLES1ConformanceTest, BufferObject)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, BufferObjectExec());
}

TEST_P(GLES1ConformanceTest, PointSizeArray)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, PointSizeArrayExec());
}

TEST_P(GLES1ConformanceTest, PointSprite)
{
    // http://anglebug.com/42265148
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());

    ASSERT_NE(CONFORMANCE_TEST_ERROR, PointSpriteExec());
}

TEST_P(GLES1ConformanceTest, UserClip)
{
    // Fails on UHD 770 driver 31.0.101.5333 http://anglebug.com/352085732
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsDesktopOpenGL());

    // "2.11 Clipping" describes the complementarity criterion, where a
    // primitive drawn once with a particular clip plane and again with the
    // negated version of the clip plane must not overdraw for pixels where the
    // plane equation evaluates exactly to zero; that is, we would need to
    // detect previously drawn fragments from one clip plane that lie exactly
    // on the half space boundary, and avoid drawing them if the same primitive
    // is issued next draw with a negated version of the clip plane.
    ASSERT_NE(CONFORMANCE_TEST_ERROR, UserClipExec());
}

TEST_P(GLES1ConformanceTest, MatrixGetTest)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MatrixGetTestExec());
}

TEST_P(GLES1ConformanceTest, Gets)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, GetsExec());
}

TEST_P(GLES1ConformanceTest, TexCombine)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, TexCombineExec());
}

TEST_P(GLES1ConformanceTest, MatrixPalette)
{
    ASSERT_NE(CONFORMANCE_TEST_ERROR, MatrixPaletteExec());
}

ANGLE_INSTANTIATE_TEST(GLES1ConformanceTest, ES1_OPENGL(), ES1_VULKAN());
}  // namespace angle
