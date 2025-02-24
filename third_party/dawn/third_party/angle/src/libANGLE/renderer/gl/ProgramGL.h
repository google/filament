//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.h: Defines the class interface for ProgramGL.

#ifndef LIBANGLE_RENDERER_GL_PROGRAMGL_H_
#define LIBANGLE_RENDERER_GL_PROGRAMGL_H_

#include <string>
#include <vector>

#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/gl/ProgramExecutableGL.h"

namespace angle
{
struct FeaturesGL;
}  // namespace angle

namespace rx
{

class FunctionsGL;
class RendererGL;
class StateManagerGL;

class ProgramGL : public ProgramImpl
{
  public:
    ProgramGL(const gl::ProgramState &data,
              const FunctionsGL *functions,
              const angle::FeaturesGL &features,
              StateManagerGL *stateManager,
              const std::shared_ptr<RendererGL> &renderer);
    ~ProgramGL() override;

    void destroy(const gl::Context *context) override;

    angle::Result load(const gl::Context *context,
                       gl::BinaryInputStream *stream,
                       std::shared_ptr<LinkTask> *loadTaskOut,
                       egl::CacheGetResult *resultOut) override;
    void save(const gl::Context *context, gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;
    void setSeparable(bool separable) override;

    void prepareForLink(const gl::ShaderMap<ShaderImpl *> &shaders) override;
    angle::Result link(const gl::Context *contextImpl,
                       std::shared_ptr<LinkTask> *linkTaskOut) override;
    GLboolean validate(const gl::Caps &caps) override;

    void markUnusedUniformLocations(std::vector<gl::VariableLocation> *uniformLocations,
                                    std::vector<gl::SamplerBinding> *samplerBindings,
                                    std::vector<gl::ImageBinding> *imageBindings) override;

    ANGLE_INLINE GLuint getProgramID() const { return mProgramID; }

    void onUniformBlockBinding(gl::UniformBlockIndex uniformBlockIndex) override;

    const ProgramExecutableGL *getExecutable() const
    {
        return GetImplAs<ProgramExecutableGL>(&mState.getExecutable());
    }
    ProgramExecutableGL *getExecutable()
    {
        return GetImplAs<ProgramExecutableGL>(&mState.getExecutable());
    }

  private:
    class LinkTaskGL;
    class PostLinkGL;

    friend class LinkTaskGL;
    friend class PostLinkGL;

    angle::Result linkJobImpl(const gl::Extensions &extensions);
    angle::Result postLinkJobImpl(const gl::ProgramLinkedResources &resources);

    bool checkLinkStatus();

    bool getUniformBlockSize(const std::string &blockName,
                             const std::string &blockMappedName,
                             size_t *sizeOut) const;
    bool getUniformBlockMemberInfo(const std::string &memberUniformName,
                                   const std::string &memberUniformMappedName,
                                   sh::BlockMemberInfo *memberInfoOut) const;
    bool getShaderStorageBlockMemberInfo(const std::string &memberName,
                                         const std::string &memberMappedName,
                                         sh::BlockMemberInfo *memberInfoOut) const;
    bool getShaderStorageBlockSize(const std::string &blockName,
                                   const std::string &blockMappedName,
                                   size_t *sizeOut) const;
    void getAtomicCounterBufferSizeMap(std::map<int, unsigned int> *sizeMapOut) const;

    void linkResources(const gl::ProgramLinkedResources &resources);

    const FunctionsGL *mFunctions;
    const angle::FeaturesGL &mFeatures;
    StateManagerGL *mStateManager;

    gl::ShaderMap<GLuint> mAttachedShaders;

    GLuint mProgramID;

    std::shared_ptr<RendererGL> mRenderer;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_PROGRAMGL_H_
