//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramNULL.cpp:
//    Implements the class methods for ProgramNULL.
//

#include "libANGLE/renderer/null/ProgramNULL.h"

#include "common/debug.h"

namespace rx
{
namespace
{
class LinkTaskNULL : public LinkTask
{
  public:
    LinkTaskNULL(const gl::ProgramState *state) : mState(state) {}
    ~LinkTaskNULL() override = default;
    void link(const gl::ProgramLinkedResources &resources,
              const gl::ProgramMergedVaryings &mergedVaryings,
              std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
              std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut) override
    {
        ASSERT(linkSubTasksOut && linkSubTasksOut->empty());
        ASSERT(postLinkSubTasksOut && postLinkSubTasksOut->empty());

        const gl::SharedCompiledShaderState &fragmentShader =
            mState->getAttachedShader(gl::ShaderType::Fragment);
        if (fragmentShader != nullptr)
        {
            resources.pixelLocalStorageLinker.link(fragmentShader->pixelLocalStorageFormats);
        }

        return;
    }
    angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) override
    {
        return angle::Result::Continue;
    }

  private:
    const gl::ProgramState *mState;
};
}  // anonymous namespace

ProgramNULL::ProgramNULL(const gl::ProgramState &state) : ProgramImpl(state) {}

ProgramNULL::~ProgramNULL() {}

angle::Result ProgramNULL::load(const gl::Context *context,
                                gl::BinaryInputStream *stream,
                                std::shared_ptr<LinkTask> *loadTaskOut,
                                egl::CacheGetResult *resultOut)
{
    *loadTaskOut = {};
    *resultOut   = egl::CacheGetResult::Success;
    return angle::Result::Continue;
}

void ProgramNULL::save(const gl::Context *context, gl::BinaryOutputStream *stream) {}

void ProgramNULL::setBinaryRetrievableHint(bool retrievable) {}

void ProgramNULL::setSeparable(bool separable) {}

angle::Result ProgramNULL::link(const gl::Context *contextImpl,
                                std::shared_ptr<LinkTask> *linkTaskOut)
{
    *linkTaskOut = std::shared_ptr<LinkTask>(new LinkTaskNULL(&mState));
    return angle::Result::Continue;
}

GLboolean ProgramNULL::validate(const gl::Caps &caps)
{
    return GL_TRUE;
}

}  // namespace rx
