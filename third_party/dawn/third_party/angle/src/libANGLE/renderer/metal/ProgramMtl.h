
//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramMtl.h:
//    Defines the class interface for ProgramMtl, implementing ProgramImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_PROGRAMMTL_H_
#define LIBANGLE_RENDERER_METAL_PROGRAMMTL_H_

#import <Metal/Metal.h>

#include <array>

#include "common/Optional.h"
#include "common/utilities.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/metal/ProgramExecutableMtl.h"
#include "libANGLE/renderer/metal/ShaderMtl.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"

namespace rx
{
#define SHADER_ENTRY_NAME @"main0"
class ContextMtl;

class ProgramMtl : public ProgramImpl
{
  public:
    ProgramMtl(const gl::ProgramState &state);
    ~ProgramMtl() override;

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

    const ProgramExecutableMtl *getExecutable() const
    {
        return mtl::GetImpl(&mState.getExecutable());
    }
    ProgramExecutableMtl *getExecutable() { return mtl::GetImpl(&mState.getExecutable()); }

  private:
    class LinkTaskMtl;
    class LoadTaskMtl;

    friend class LinkTaskMtl;

    angle::Result linkJobImpl(mtl::Context *context,
                              const gl::ProgramLinkedResources &resources,
                              std::vector<std::shared_ptr<LinkSubTask>> *subTasksOut);

    void linkResources(const gl::ProgramLinkedResources &resources);
    angle::Result compileMslShaderLibs(mtl::Context *context,
                                       std::vector<std::shared_ptr<LinkSubTask>> *subTasksOut);

    gl::ShaderMap<SharedCompiledShaderStateMtl> mAttachedShaders;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_PROGRAMMTL_H_ */
