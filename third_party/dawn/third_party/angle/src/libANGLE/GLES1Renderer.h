//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GLES1Renderer.h: Defines GLES1 emulation rendering operations on top of a GLES3
// context. Used by Context.h.

#ifndef LIBANGLE_GLES1_RENDERER_H_
#define LIBANGLE_GLES1_RENDERER_H_

#include "GLES1State.h"
#include "angle_gl.h"
#include "common/angleutils.h"
#include "common/hash_containers.h"
#include "libANGLE/angletypes.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace gl
{
class Context;
class GLES1State;
class Program;
class State;
class Shader;
class ShaderProgramManager;

enum class GLES1StateEnables : uint64_t
{
    Lighting                       = 0,
    Fog                            = 1,
    ClipPlanes                     = 2,
    DrawTexture                    = 3,
    PointRasterization             = 4,
    PointSprite                    = 5,
    RescaleNormal                  = 6,
    Normalize                      = 7,
    AlphaTest                      = 8,
    ShadeModelFlat                 = 9,
    ColorMaterial                  = 10,
    LightModelTwoSided             = 11,
    LogicOpThroughFramebufferFetch = 12,

    InvalidEnum = 13,
    EnumCount   = 13,
};

constexpr int kClipPlaneCount = 6;
constexpr int kTexUnitCount   = 4;
constexpr int kLightCount     = 8;

using GLES1StateEnabledBitSet = angle::PackedEnumBitSet<GLES1StateEnables, uint64_t>;

struct GLES1ShaderState
{
    GLES1ShaderState();
    ~GLES1ShaderState();
    GLES1ShaderState(const GLES1ShaderState &other);

    size_t hash() const;

    GLES1StateEnabledBitSet mGLES1StateEnabled;

    using BoolLightArray     = bool[kLightCount];
    using BoolTexArray       = bool[kTexUnitCount];
    using BoolClipPlaneArray = bool[kClipPlaneCount];
    using UintTexArray       = uint16_t[kTexUnitCount];

    BoolTexArray tex2DEnables   = {false, false, false, false};
    BoolTexArray texCubeEnables = {false, false, false, false};

    UintTexArray tex2DFormats = {GL_RGBA, GL_RGBA, GL_RGBA, GL_RGBA};

    UintTexArray texEnvModes          = {};
    UintTexArray texCombineRgbs       = {};
    UintTexArray texCombineAlphas     = {};
    UintTexArray texCombineSrc0Rgbs   = {};
    UintTexArray texCombineSrc0Alphas = {};
    UintTexArray texCombineSrc1Rgbs   = {};
    UintTexArray texCombineSrc1Alphas = {};
    UintTexArray texCombineSrc2Rgbs   = {};
    UintTexArray texCombineSrc2Alphas = {};
    UintTexArray texCombineOp0Rgbs    = {};
    UintTexArray texCombineOp0Alphas  = {};
    UintTexArray texCombineOp1Rgbs    = {};
    UintTexArray texCombineOp1Alphas  = {};
    UintTexArray texCombineOp2Rgbs    = {};
    UintTexArray texCombineOp2Alphas  = {};

    BoolTexArray pointSpriteCoordReplaces = {};

    BoolLightArray lightEnables = {};

    BoolClipPlaneArray clipPlaneEnables = {};

    AlphaTestFunc alphaTestFunc = {};

    FogMode fogMode = {};
};

bool operator==(const GLES1ShaderState &a, const GLES1ShaderState &b);
bool operator!=(const GLES1ShaderState &a, const GLES1ShaderState &b);

}  // namespace gl

namespace std
{
template <>
struct hash<gl::GLES1ShaderState>
{
    size_t operator()(const gl::GLES1ShaderState &key) const { return key.hash(); }
};
}  // namespace std

namespace gl
{

class GLES1Renderer final : angle::NonCopyable
{
  public:
    GLES1Renderer();
    ~GLES1Renderer();

    void onDestroy(Context *context, State *state);

    angle::Result prepareForDraw(PrimitiveMode mode,
                                 Context *context,
                                 State *glState,
                                 GLES1State *gles1State);

    static int VertexArrayIndex(ClientVertexArrayType type, const GLES1State &gles1);
    static ClientVertexArrayType VertexArrayType(int attribIndex);
    static int TexCoordArrayIndex(unsigned int unit);

    void drawTexture(Context *context,
                     State *glState,
                     GLES1State *gles1State,
                     float x,
                     float y,
                     float z,
                     float width,
                     float height);

  private:
    using Mat4Uniform = float[16];
    using Vec4Uniform = float[4];
    using Vec3Uniform = float[3];

    Shader *getShader(ShaderProgramID handle) const;
    Program *getProgram(ShaderProgramID handle) const;

    angle::Result compileShader(Context *context,
                                ShaderType shaderType,
                                const char *src,
                                ShaderProgramID *shaderOut);
    angle::Result linkProgram(Context *context,
                              State *glState,
                              ShaderProgramID vshader,
                              ShaderProgramID fshader,
                              const angle::HashMap<GLint, std::string> &attribLocs,
                              ShaderProgramID *programOut);
    angle::Result initializeRendererProgram(Context *context,
                                            State *glState,
                                            GLES1State *gles1State);

    void setUniform1i(Context *context,
                      ProgramExecutable *executable,
                      UniformLocation location,
                      GLint value);
    void setUniform1ui(ProgramExecutable *executable, UniformLocation location, GLuint value);
    void setUniform1iv(Context *context,
                       ProgramExecutable *executable,
                       UniformLocation location,
                       GLint count,
                       const GLint *value);
    void setUniformMatrix4fv(ProgramExecutable *executable,
                             UniformLocation location,
                             GLint count,
                             GLboolean transpose,
                             const GLfloat *value);
    void setUniform4fv(ProgramExecutable *executable,

                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform3fv(ProgramExecutable *executable,

                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform2fv(ProgramExecutable *executable,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);
    void setUniform1f(ProgramExecutable *executable, UniformLocation location, GLfloat value);
    void setUniform1fv(ProgramExecutable *executable,
                       UniformLocation location,
                       GLint count,
                       const GLfloat *value);

    void setAttributesEnabled(Context *context,
                              State *glState,
                              GLES1State *gles1State,
                              AttributesMask mask);

    static constexpr int kVertexAttribIndex           = 0;
    static constexpr int kNormalAttribIndex           = 1;
    static constexpr int kColorAttribIndex            = 2;
    static constexpr int kPointSizeAttribIndex        = 3;
    static constexpr int kTextureCoordAttribIndexBase = 4;

    bool mRendererProgramInitialized;
    ShaderProgramManager *mShaderPrograms;

    GLES1ShaderState mShaderState = {};

    const char *getShaderBool(GLES1StateEnables state);
    void addShaderDefine(std::stringstream &outStream,
                         GLES1StateEnables state,
                         const char *enableString);
    void addShaderUint(std::stringstream &outStream, const char *name, uint16_t value);
    void addShaderUintTexArray(std::stringstream &outStream,
                               const char *texString,
                               GLES1ShaderState::UintTexArray &texState);
    void addShaderBoolTexArray(std::stringstream &outStream,
                               const char *texString,
                               GLES1ShaderState::BoolTexArray &texState);
    void addShaderBoolLightArray(std::stringstream &outStream,
                                 const char *name,
                                 GLES1ShaderState::BoolLightArray &value);
    void addShaderBoolClipPlaneArray(std::stringstream &outStream,
                                     const char *name,
                                     GLES1ShaderState::BoolClipPlaneArray &value);
    void addVertexShaderDefs(std::stringstream &outStream);
    void addFragmentShaderDefs(std::stringstream &outStream);

    struct GLES1ProgramState
    {
        ShaderProgramID program;

        UniformLocation projMatrixLoc;
        UniformLocation modelviewMatrixLoc;
        UniformLocation textureMatrixLoc;
        UniformLocation modelviewInvTrLoc;

        // Texturing
        std::array<UniformLocation, kTexUnitCount> tex2DSamplerLocs;
        std::array<UniformLocation, kTexUnitCount> texCubeSamplerLocs;

        UniformLocation textureEnvColorLoc;
        UniformLocation rgbScaleLoc;
        UniformLocation alphaScaleLoc;

        // Alpha test
        UniformLocation alphaTestRefLoc;

        // Shading, materials, and lighting
        UniformLocation materialAmbientLoc;
        UniformLocation materialDiffuseLoc;
        UniformLocation materialSpecularLoc;
        UniformLocation materialEmissiveLoc;
        UniformLocation materialSpecularExponentLoc;

        UniformLocation lightModelSceneAmbientLoc;

        UniformLocation lightAmbientsLoc;
        UniformLocation lightDiffusesLoc;
        UniformLocation lightSpecularsLoc;
        UniformLocation lightPositionsLoc;
        UniformLocation lightDirectionsLoc;
        UniformLocation lightSpotlightExponentsLoc;
        UniformLocation lightSpotlightCutoffAnglesLoc;
        UniformLocation lightAttenuationConstsLoc;
        UniformLocation lightAttenuationLinearsLoc;
        UniformLocation lightAttenuationQuadraticsLoc;

        // Fog
        UniformLocation fogDensityLoc;
        UniformLocation fogStartLoc;
        UniformLocation fogEndLoc;
        UniformLocation fogColorLoc;

        // Clip planes
        UniformLocation clipPlanesLoc;

        // Logic op
        UniformLocation logicOpLoc;

        // Point rasterization
        UniformLocation pointSizeMinLoc;
        UniformLocation pointSizeMaxLoc;
        UniformLocation pointDistanceAttenuationLoc;

        // Draw texture
        UniformLocation drawTextureCoordsLoc;
        UniformLocation drawTextureDimsLoc;
        UniformLocation drawTextureNormalizedCropRectLoc;
    };

    struct GLES1UniformBuffers
    {
        std::array<Mat4Uniform, kTexUnitCount> textureMatrices;

        std::array<Vec4Uniform, kTexUnitCount> texEnvColors;
        std::array<GLfloat, kTexUnitCount> texEnvRgbScales;
        std::array<GLfloat, kTexUnitCount> texEnvAlphaScales;

        // Lighting
        std::array<Vec4Uniform, kLightCount> lightAmbients;
        std::array<Vec4Uniform, kLightCount> lightDiffuses;
        std::array<Vec4Uniform, kLightCount> lightSpeculars;
        std::array<Vec4Uniform, kLightCount> lightPositions;
        std::array<Vec3Uniform, kLightCount> lightDirections;
        std::array<GLfloat, kLightCount> spotlightExponents;
        std::array<GLfloat, kLightCount> spotlightCutoffAngles;
        std::array<GLfloat, kLightCount> attenuationConsts;
        std::array<GLfloat, kLightCount> attenuationLinears;
        std::array<GLfloat, kLightCount> attenuationQuadratics;

        // Clip planes
        std::array<Vec4Uniform, kClipPlaneCount> clipPlanes;

        // Texture crop rectangles
        std::array<Vec4Uniform, kTexUnitCount> texCropRects;
    };

    struct GLES1UberShaderState
    {
        GLES1UniformBuffers uniformBuffers;
        GLES1ProgramState programState;
    };

    GLES1UberShaderState &getUberShaderState()
    {
        ASSERT(mUberShaderState.find(mShaderState) != mUberShaderState.end());
        return mUberShaderState[mShaderState];
    }

    angle::HashMap<GLES1ShaderState, GLES1UberShaderState> mUberShaderState;

    bool mDrawTextureEnabled      = false;
    GLfloat mDrawTextureCoords[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    GLfloat mDrawTextureDims[2]   = {0.0f, 0.0f};
};

}  // namespace gl

#endif  // LIBANGLE_GLES1_RENDERER_H_
