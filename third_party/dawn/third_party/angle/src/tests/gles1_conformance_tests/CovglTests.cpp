//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CovglTests.cpp:
//   GLES1 conformance covgl tests.
//   Function prototypes taken from tproto.h and turned into gtest tests using a macro.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(GL_OES_VERSION_1_1) && defined(GL_VERSION_ES_CM_1_1) && defined(GL_VERSION_ES_CL_1_1)
#    if GL_VERSION_ES_CM_1_1 || GL_VERSION_ES_CL_1_1
#        define GL_OES_VERSION_1_1
#    endif
#endif

// ES 1.0
extern void StateReset(void);
extern void ProbeError(void (*)(void));
extern GLboolean tkCheckExtension(const char *name);

extern void CallActiveTexture(void);
extern void CallAlphaFunc(void);
extern void CallBlendFunc(void);
extern void CallBindDeleteTexture(void);
extern void CallClear(void);
extern void CallClearColor(void);
extern void CallClearDepth(void);
extern void CallClearStencil(void);
extern void CallClientActiveTexture(void);
extern void CallColor(void);
extern void CallColorMask(void);
extern void CallColorPointer(void);
extern void CallCompressedTexImage2D(void);
extern void CallCompressedTexSubImage2D(void);
extern void CallCopyTexImage2D(void);
extern void CallCopyTexSubImage2D(void);
extern void CallCullFace(void);
extern void CallDepthFunc(void);
extern void CallDepthMask(void);
extern void CallDepthRange(void);
extern void CallDrawArrays(void);
extern void CallDrawElements(void);

#ifdef GL_OES_draw_texture
extern void CallDrawTex(void);
#endif /* GL_OES_draw_texture */

extern void CallEdgeFlag(void);
extern void CallEnableDisable(void);
extern void CallEnableDisableClientState(void);
extern void CallFinish(void);
extern void CallFlush(void);
extern void CallFog(void);
extern void CallFrontFace(void);
extern void CallFrustum(void);
extern void CallGenTextures(void);
extern void CallGet(void);
extern void CallGetError(void);
extern void CallGetString(void);
#ifdef GL_OES_VERSION_1_1
extern void CallGetTexEnv(void);
extern void CallGetLight(void);
extern void CallGetMaterial(void);
extern void CallGetClipPlane(void);
extern void CallGetPointer(void);
#endif /* GL_OES_VERSION_1_1 */

#ifdef GL_OES_VERSION_1_1
extern void CallGetBufferParameter(void);
extern void CallGetTexParameter(void);
#endif /* GL_OES_VERSION_1_1 */

extern void CallHint(void);
extern void CallLight(void);
extern void CallLightModel(void);
extern void CallLineWidth(void);
extern void CallLoadIdentity(void);
extern void CallLoadMatrix(void);
extern void CallLogicOp(void);
extern void CallMaterial(void);
extern void CallMatrixMode(void);
extern void CallMultiTexCoord(void);
extern void CallMultMatrix(void);
extern void CallNormal(void);
extern void CallNormalPointer(void);
extern void CallOrtho(void);
extern void CallPixelStore(void);
#ifdef GL_OES_VERSION_1_1
extern void CallPointParameter(void);
#endif /* GL_OES_VERSION_1_1 */
extern void CallPointSize(void);
extern void CallPolygonOffset(void);
extern void CallPopMatrix(void);
extern void CallPushMatrix(void);
extern void CallReadPixels(void);
extern void CallRotate(void);
extern void CallSampleCoverage(void);
extern void CallScale(void);
extern void CallScissor(void);
extern void CallShadeModel(void);
extern void CallStencilFunc(void);
extern void CallStencilMask(void);
extern void CallStencilOp(void);
#ifdef GL_OES_VERSION_1_1
extern void CallIsTexture(void);
extern void CallIsEnabled(void);
#endif /* GL_OES_VERSION_1_1 */
extern void CallTexCoord(void);
extern void CallTexCoordPointer(void);
extern void CallTexEnv(void);
extern void CallTexImage2D(void);
extern void CallTexParameter(void);
extern void CallTexSubImage2D(void);
extern void CallTranslate(void);
extern void CallVertexPointer(void);
extern void CallViewport(void);

#ifdef GL_OES_VERSION_1_1
extern void CallBindDeleteBuffer(void);
extern void CallBufferData(void);
extern void CallBufferSubData(void);
extern void CallGenBuffers(void);
extern void CallIsBuffer(void);
extern void CallPointSizePointerOES(void);
extern void CallClipPlane(void);
#endif /* GL_OES_VERSION_1_1 */

#ifdef GL_OES_matrix_palette
extern void CallCurrentPaletteMatrixOES(void);
extern void CallLoadPaletteFromModelViewMatrixOES(void);
extern void CallMatrixIndexPointerOES(void);
extern void CallWeightPointerOES(void);
#endif /* GL_OES_matrix_palette */

#ifdef GL_OES_query_matrix
extern void CallQueryMatrix(void);
#endif

void ProbeEnumANGLE(void)
{
    ASSERT_GL_NO_ERROR();
}

#ifdef __cplusplus
}

#endif

namespace angle
{
class GLES1CovglTest : public ANGLETest<>
{
  protected:
    GLES1CovglTest()
    {
        setWindowWidth(48);
        setWindowHeight(48);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        StateReset();
        glViewport(0, 0, 48, 48);
        glScissor(0, 0, 48, 48);
    }
};

TEST_P(GLES1CovglTest, Get)
{
    ProbeError(CallGet);
}

TEST_P(GLES1CovglTest, GetError)
{
    ProbeError(CallGetError);
}
TEST_P(GLES1CovglTest, GetString)
{
    ProbeError(CallGetString);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, GetTexEnv)
{
    ProbeError(CallGetTexEnv);
}
TEST_P(GLES1CovglTest, GetLight)
{
    ProbeError(CallGetLight);
}
TEST_P(GLES1CovglTest, GetMaterial)
{
    ProbeError(CallGetMaterial);
}
TEST_P(GLES1CovglTest, GetClipPlane)
{
    ProbeError(CallGetClipPlane);
}
TEST_P(GLES1CovglTest, GetPointer)
{
    ProbeError(CallGetPointer);
}
TEST_P(GLES1CovglTest, GetTexParameter)
{
    ProbeError(CallGetTexParameter);
}
TEST_P(GLES1CovglTest, GetBufferParameter)
{
    ProbeError(CallGetBufferParameter);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, EnableDisable)
{
    ProbeError(CallEnableDisable);
}

TEST_P(GLES1CovglTest, Hint)
{
    ProbeError(CallHint);
}

TEST_P(GLES1CovglTest, Viewport)
{
    ProbeError(CallViewport);
}
TEST_P(GLES1CovglTest, Ortho)
{
    ProbeError(CallOrtho);
}
TEST_P(GLES1CovglTest, Frustum)
{
    ProbeError(CallFrustum);
}
TEST_P(GLES1CovglTest, Scissor)
{
    ProbeError(CallScissor);
}

TEST_P(GLES1CovglTest, LoadIdentity)
{
    ProbeError(CallLoadIdentity);
}
TEST_P(GLES1CovglTest, MatrixMode)
{
    ProbeError(CallMatrixMode);
}
TEST_P(GLES1CovglTest, PushMatrix)
{
    ProbeError(CallPushMatrix);
}
TEST_P(GLES1CovglTest, LoadMatrix)
{
    ProbeError(CallLoadMatrix);
}
TEST_P(GLES1CovglTest, MultMatrix)
{
    ProbeError(CallMultMatrix);
}
TEST_P(GLES1CovglTest, Rotate)
{
    ProbeError(CallRotate);
}
TEST_P(GLES1CovglTest, Scale)
{
    ProbeError(CallScale);
}
TEST_P(GLES1CovglTest, Translate)
{
    ProbeError(CallTranslate);
}
TEST_P(GLES1CovglTest, PopMatrix)
{
    ProbeError(CallPopMatrix);
}

TEST_P(GLES1CovglTest, Clear)
{
    ProbeError(CallClear);
}
TEST_P(GLES1CovglTest, ClearColor)
{
    ProbeError(CallClearColor);
}
TEST_P(GLES1CovglTest, ClearDepth)
{
    ProbeError(CallClearDepth);
}
TEST_P(GLES1CovglTest, ClearStencil)
{
    ProbeError(CallClearStencil);
}

TEST_P(GLES1CovglTest, ColorMask)
{
    ProbeError(CallColorMask);
}
TEST_P(GLES1CovglTest, Color)
{
    ProbeError(CallColor);
}

TEST_P(GLES1CovglTest, Normal)
{
    ProbeError(CallNormal);
}

TEST_P(GLES1CovglTest, AlphaFunc)
{
    ProbeError(CallAlphaFunc);
}
TEST_P(GLES1CovglTest, BlendFunc)
{
    ProbeError(CallBlendFunc);
}
TEST_P(GLES1CovglTest, DepthFunc)
{
    ProbeError(CallDepthFunc);
}
TEST_P(GLES1CovglTest, DepthMask)
{
    ProbeError(CallDepthMask);
}
TEST_P(GLES1CovglTest, DepthRange)
{
    ProbeError(CallDepthRange);
}
TEST_P(GLES1CovglTest, LogicOp)
{
    ProbeError(CallLogicOp);
}
TEST_P(GLES1CovglTest, StencilFunc)
{
    ProbeError(CallStencilFunc);
}
TEST_P(GLES1CovglTest, StencilMask)
{
    ProbeError(CallStencilMask);
}
TEST_P(GLES1CovglTest, StencilOp)
{
    ProbeError(CallStencilOp);
}

TEST_P(GLES1CovglTest, PixelStore)
{
    ProbeError(CallPixelStore);
}
TEST_P(GLES1CovglTest, ReadPixels)
{
    ProbeError(CallReadPixels);
}

TEST_P(GLES1CovglTest, Fog)
{
    ProbeError(CallFog);
}
TEST_P(GLES1CovglTest, LightModel)
{
    ProbeError(CallLightModel);
}
TEST_P(GLES1CovglTest, Light)
{
    ProbeError(CallLight);
}
TEST_P(GLES1CovglTest, Material)
{
    ProbeError(CallMaterial);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, IsTexture)
{
    ProbeError(CallIsTexture);
}
TEST_P(GLES1CovglTest, IsEnabled)
{
    ProbeError(CallIsEnabled);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, TexEnv)
{
    ProbeError(CallTexEnv);
}
TEST_P(GLES1CovglTest, TexParameter)
{
    ProbeError(CallTexParameter);
}
TEST_P(GLES1CovglTest, TexImage2D)
{
    ProbeError(CallTexImage2D);
}
TEST_P(GLES1CovglTest, TexSubImage2D)
{
    ProbeError(CallTexSubImage2D);
}
TEST_P(GLES1CovglTest, GenTextures)
{
    ProbeError(CallGenTextures);
}
TEST_P(GLES1CovglTest, BindDeleteTexture)
{
    ProbeError(CallBindDeleteTexture);
}
TEST_P(GLES1CovglTest, CopyTexImage2D)
{
    ProbeError(CallCopyTexImage2D);
}
TEST_P(GLES1CovglTest, CopyTexSubImage2D)
{
    ProbeError(CallCopyTexSubImage2D);
}
TEST_P(GLES1CovglTest, CompressedTexImage2D)
{
    ProbeError(CallCompressedTexImage2D);
}
TEST_P(GLES1CovglTest, CompressedTexSubImage2D)
{
    ProbeError(CallCompressedTexSubImage2D);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, BindDeleteBuffer)
{
    ProbeError(CallBindDeleteBuffer);
}
TEST_P(GLES1CovglTest, IsBuffer)
{
    ProbeError(CallIsBuffer);
}
TEST_P(GLES1CovglTest, BufferData)
{
    ProbeError(CallBufferData);
}
TEST_P(GLES1CovglTest, BufferSubData)
{
    ProbeError(CallBufferSubData);
}
TEST_P(GLES1CovglTest, GenBuffers)
{
    ProbeError(CallGenBuffers);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, ShadeModel)
{
    ProbeError(CallShadeModel);
}
TEST_P(GLES1CovglTest, PointSize)
{
    ProbeError(CallPointSize);
}
TEST_P(GLES1CovglTest, LineWidth)
{
    ProbeError(CallLineWidth);
}
TEST_P(GLES1CovglTest, CullFace)
{
    ProbeError(CallCullFace);
}
TEST_P(GLES1CovglTest, FrontFace)
{
    ProbeError(CallFrontFace);
}
TEST_P(GLES1CovglTest, PolygonOffset)
{
    ProbeError(CallPolygonOffset);
}

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, PointParameter)
{
    ProbeError(CallPointParameter);
}
#endif /* GL_OES_VERSION_1_1 */

TEST_P(GLES1CovglTest, Flush)
{
    ProbeError(CallFlush);
}
TEST_P(GLES1CovglTest, Finish)
{
    ProbeError(CallFinish);
}

TEST_P(GLES1CovglTest, ColorPointer)
{
    ProbeError(CallColorPointer);
}
TEST_P(GLES1CovglTest, DrawArrays)
{
    ProbeError(CallDrawArrays);
}
TEST_P(GLES1CovglTest, DrawElements)
{
    ProbeError(CallDrawElements);
}
#ifdef GL_OES_draw_texture
TEST_P(GLES1CovglTest, DrawTex)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_draw_texture"));

    ProbeError(CallDrawTex);
}
#endif /*GL_OES_draw_texture */
TEST_P(GLES1CovglTest, NormalPointer)
{
    ProbeError(CallNormalPointer);
}
TEST_P(GLES1CovglTest, TexCoordPointer)
{
    ProbeError(CallTexCoordPointer);
}
TEST_P(GLES1CovglTest, VertexPointer)
{
    ProbeError(CallVertexPointer);
}
#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, PointSizePointerOES)
{
    ProbeError(CallPointSizePointerOES);
}
#endif
TEST_P(GLES1CovglTest, EnableDisableClientState)
{
    ProbeError(CallEnableDisableClientState);
}

TEST_P(GLES1CovglTest, ActiveTexture)
{
    ProbeError(CallActiveTexture);
}
TEST_P(GLES1CovglTest, ClientActiveTexture)
{
    ProbeError(CallClientActiveTexture);
}
TEST_P(GLES1CovglTest, MultiTexCoord)
{
    ProbeError(CallMultiTexCoord);
}

TEST_P(GLES1CovglTest, SampleCoverage)
{
    ProbeError(CallSampleCoverage);
}

#ifdef GL_OES_query_matrix
TEST_P(GLES1CovglTest, QueryMatrix)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_query_matrix"));
    ProbeError(CallQueryMatrix);
}
#endif

#ifdef GL_OES_matrix_palette
TEST_P(GLES1CovglTest, CurrentPaletteMatrixOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    ProbeError(CallCurrentPaletteMatrixOES);
}
TEST_P(GLES1CovglTest, LoadPaletteFromModelViewMatrixOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    ProbeError(CallLoadPaletteFromModelViewMatrixOES);
}
TEST_P(GLES1CovglTest, MatrixIndexPointerOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    ProbeError(CallMatrixIndexPointerOES);
}
TEST_P(GLES1CovglTest, WeightPointerOES)
{
    ANGLE_SKIP_TEST_IF(!tkCheckExtension("GL_OES_matrix_palette"));
    ProbeError(CallWeightPointerOES);
}
#endif

#ifdef GL_OES_VERSION_1_1
TEST_P(GLES1CovglTest, ClipPlane)
{
    ProbeError(CallClipPlane);
}
#endif

ANGLE_INSTANTIATE_TEST(GLES1CovglTest, ES1_OPENGL(), ES1_VULKAN());
}  // namespace angle
