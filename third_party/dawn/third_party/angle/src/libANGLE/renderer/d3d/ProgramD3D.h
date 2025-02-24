//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramD3D.h: Defines the rx::ProgramD3D class which implements rx::ProgramImpl.

#ifndef LIBANGLE_RENDERER_D3D_PROGRAMD3D_H_
#define LIBANGLE_RENDERER_D3D_PROGRAMD3D_H_

#include <string>
#include <vector>

#include "compiler/translator/hlsl/blocklayoutHLSL.h"
#include "libANGLE/Constants.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/d3d/ProgramExecutableD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/ShaderD3D.h"
#include "platform/autogen/FeaturesD3D_autogen.h"

namespace rx
{
class RendererD3D;

class ProgramD3DMetadata final : angle::NonCopyable
{
  public:
    ProgramD3DMetadata(RendererD3D *renderer,
                       const gl::SharedCompiledShaderState &fragmentShader,
                       const gl::ShaderMap<SharedCompiledShaderStateD3D> &attachedShaders,
                       int shaderVersion);
    ~ProgramD3DMetadata();

    int getRendererMajorShaderModel() const;
    bool usesBroadcast(const gl::Version &clientVersion) const;
    bool usesSecondaryColor() const;
    bool usesPointCoord() const;
    bool usesFragCoord() const;
    bool usesPointSize() const;
    bool usesInsertedPointCoordValue() const;
    bool usesViewScale() const;
    bool hasMultiviewEnabled() const;
    bool usesVertexID() const;
    bool usesViewID() const;
    bool canSelectViewInVertexShader() const;
    bool addsPointCoordToVertexShader() const;
    bool usesTransformFeedbackGLPosition() const;
    bool usesSystemValuePointSize() const;
    bool usesMultipleFragmentOuts() const;
    bool usesCustomOutVars() const;
    bool usesSampleMask() const;
    const gl::SharedCompiledShaderState &getFragmentShader() const;
    FragDepthUsage getFragDepthUsage() const;
    uint8_t getClipDistanceArraySize() const;
    uint8_t getCullDistanceArraySize() const;

  private:
    const int mRendererMajorShaderModel;
    const std::string mShaderModelSuffix;
    const bool mUsesViewScale;
    const bool mCanSelectViewInVertexShader;
    gl::SharedCompiledShaderState mFragmentShader;
    const gl::ShaderMap<SharedCompiledShaderStateD3D> &mAttachedShaders;
    int mShaderVersion;
};

class ProgramD3D : public ProgramImpl
{
  public:
    ProgramD3D(const gl::ProgramState &data, RendererD3D *renderer);
    ~ProgramD3D() override;

    void destroy(const gl::Context *context) override;

    angle::Result load(const gl::Context *context,
                       gl::BinaryInputStream *stream,
                       std::shared_ptr<LinkTask> *loadTaskOut,
                       egl::CacheGetResult *resultOut) override;
    void save(const gl::Context *context, gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;
    void setSeparable(bool separable) override;

    void prepareForLink(const gl::ShaderMap<ShaderImpl *> &shaders) override;
    angle::Result link(const gl::Context *context, std::shared_ptr<LinkTask> *linkTaskOut) override;
    GLboolean validate(const gl::Caps &caps) override;

    const gl::ProgramState &getState() const { return mState; }

    const ProgramExecutableD3D *getExecutable() const
    {
        return GetImplAs<ProgramExecutableD3D>(&mState.getExecutable());
    }
    ProgramExecutableD3D *getExecutable()
    {
        return GetImplAs<ProgramExecutableD3D>(&mState.getExecutable());
    }

  private:
    class GetVertexExecutableTask;
    class GetPixelExecutableTask;
    class GetGeometryExecutableTask;
    class GetComputeExecutableTask;
    class LinkLoadTaskD3D;
    class LinkTaskD3D;
    class LoadTaskD3D;

    friend class LinkTaskD3D;
    friend class LoadTaskD3D;

    angle::Result linkJobImpl(d3d::Context *context,
                              const gl::Caps &caps,
                              const gl::Version &clientVersion,
                              const gl::ProgramLinkedResources &resources,
                              const gl::ProgramMergedVaryings &mergedVaryings);
    const SharedCompiledShaderStateD3D &getAttachedShader(gl::ShaderType shaderType)
    {
        return getExecutable()->mAttachedShaders[shaderType];
    }

    void linkResources(const gl::ProgramLinkedResources &resources);

    RendererD3D *mRenderer;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_PROGRAMD3D_H_
