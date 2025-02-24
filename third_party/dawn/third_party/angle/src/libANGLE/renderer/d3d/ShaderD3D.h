//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderD3D.h: Defines the rx::ShaderD3D class which implements rx::ShaderImpl.

#ifndef LIBANGLE_RENDERER_D3D_SHADERD3D_H_
#define LIBANGLE_RENDERER_D3D_SHADERD3D_H_

#include "libANGLE/renderer/ShaderImpl.h"

#include <map>
#include <memory>

namespace angle
{
struct FeaturesD3D;
}  // namespace angle

namespace gl
{
struct Extensions;
}

namespace rx
{
class DynamicHLSL;
class RendererD3D;
struct D3DUniform;

// Workarounds attached to each shader. Do not need to expose information about these workarounds so
// a simple bool struct suffices.
struct CompilerWorkaroundsD3D
{
    bool skipOptimization = false;

    bool useMaxOptimization = false;

    // IEEE strictness needs to be enabled for NANs to work.
    bool enableIEEEStrictness = false;
};

enum class FragDepthUsage
{
    Unused,
    Any,
    Greater,
    Less
};

struct CompiledShaderStateD3D : angle::NonCopyable
{
    CompiledShaderStateD3D();
    ~CompiledShaderStateD3D();

    bool hasUniform(const std::string &name) const;

    // Query regular uniforms with their name. Query sampler fields of structs with field selection
    // using dot (.) operator.
    unsigned int getUniformRegister(const std::string &uniformName) const;

    unsigned int getUniformBlockRegister(const std::string &blockName) const;
    bool shouldUniformBlockUseStructuredBuffer(const std::string &blockName) const;
    unsigned int getShaderStorageBlockRegister(const std::string &blockName) const;
    bool useImage2DFunction(const std::string &functionName) const;
    const std::set<std::string> &getSlowCompilingUniformBlockSet() const;
    void appendDebugInfo(const std::string &info) { debugInfo += info; }

    void generateWorkarounds(CompilerWorkaroundsD3D *workarounds) const;

    ShShaderOutput compilerOutputType;

    bool usesMultipleRenderTargets;
    bool usesFragColor;
    bool usesFragData;
    bool usesSecondaryColor;
    bool usesFragCoord;
    bool usesFrontFacing;
    bool usesHelperInvocation;
    bool usesPointSize;
    bool usesPointCoord;
    bool usesDepthRange;
    bool usesSampleID;
    bool usesSamplePosition;
    bool usesSampleMaskIn;
    bool usesSampleMask;
    bool hasMultiviewEnabled;
    bool usesVertexID;
    bool usesViewID;
    bool usesDiscardRewriting;
    bool usesNestedBreak;
    bool requiresIEEEStrictCompiling;
    FragDepthUsage fragDepthUsage;
    uint8_t clipDistanceSize;
    uint8_t cullDistanceSize;

    std::string debugInfo;
    std::map<std::string, unsigned int> uniformRegisterMap;
    std::map<std::string, unsigned int> uniformBlockRegisterMap;
    std::map<std::string, bool> uniformBlockUseStructuredBufferMap;
    std::set<std::string> slowCompilingUniformBlockSet;
    std::map<std::string, unsigned int> shaderStorageBlockRegisterMap;
    unsigned int readonlyImage2DRegisterIndex;
    unsigned int image2DRegisterIndex;
    std::set<std::string> usedImage2DFunctionNames;
};
using SharedCompiledShaderStateD3D = std::shared_ptr<CompiledShaderStateD3D>;

class ShaderD3D : public ShaderImpl
{
  public:
    ShaderD3D(const gl::ShaderState &state, RendererD3D *renderer);
    ~ShaderD3D() override;

    std::shared_ptr<ShaderTranslateTask> compile(const gl::Context *context,
                                                 ShCompileOptions *options) override;
    std::shared_ptr<ShaderTranslateTask> load(const gl::Context *context,
                                              gl::BinaryInputStream *stream) override;

    std::string getDebugInfo() const override;

    const SharedCompiledShaderStateD3D &getCompiledState() const { return mCompiledState; }

  private:
    RendererD3D *mRenderer;

    SharedCompiledShaderStateD3D mCompiledState;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_SHADERD3D_H_
