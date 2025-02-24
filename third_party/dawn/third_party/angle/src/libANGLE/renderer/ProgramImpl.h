//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramImpl.h: Defines the abstract rx::ProgramImpl class.

#ifndef LIBANGLE_RENDERER_PROGRAMIMPL_H_
#define LIBANGLE_RENDERER_PROGRAMIMPL_H_

#include "common/BinaryStream.h"
#include "common/WorkerThread.h"
#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Program.h"
#include "libANGLE/Shader.h"

#include <functional>
#include <map>

namespace gl
{
class Context;
struct ProgramLinkedResources;
}  // namespace gl

namespace sh
{
struct BlockMemberInfo;
}

namespace rx
{
// The link job is split as such:
//
// - Front-end link
// - Back-end link
// - Independent back-end link subtasks (typically native driver compile jobs)
// - Post-link finalization
//
// Each step depends on the previous.  These steps are executed as such:
//
// 1. Program::link calls into ProgramImpl::link
//   - ProgramImpl::link runs whatever needs the Context, such as releasing resources
//   - ProgramImpl::link returns a LinkTask
// 2. Program::link implements a closure that calls the front-end link and passes the results to
//    the backend's LinkTask.
// 3. The LinkTask potentially returns a set of LinkSubTasks to be scheduled by the worker pool
// 4. Once the link is resolved, the post-link finalization is run
//
// In the above, steps 1 and 4 are done under the share group lock.  Steps 2 and 3 can be done in
// threads or without holding the share group lock if the backend supports it.
class LinkSubTask : public angle::Closure
{
  public:
    ~LinkSubTask() override                                                           = default;
    virtual angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) = 0;
};
class LinkTask
{
  public:
    virtual ~LinkTask() = default;
    // Used for link()
    // Backends should populate only one of linkSubTasksOut or postLinkSubTasksOut.
    virtual void link(const gl::ProgramLinkedResources &resources,
                      const gl::ProgramMergedVaryings &mergedVaryings,
                      std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
                      std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut);
    // Used for load()
    // Backends should populate only one of linkSubTasksOut or postLinkSubTasksOut.
    virtual void load(std::vector<std::shared_ptr<LinkSubTask>> *linkSubTasksOut,
                      std::vector<std::shared_ptr<LinkSubTask>> *postLinkSubTasksOut);
    virtual angle::Result getResult(const gl::Context *context, gl::InfoLog &infoLog) = 0;

    // Used by the GL backend to query whether the driver is linking in parallel internally.
    virtual bool isLinkingInternally();
};

class ProgramImpl : angle::NonCopyable
{
  public:
    ProgramImpl(const gl::ProgramState &state) : mState(state) {}
    virtual ~ProgramImpl() {}
    virtual void destroy(const gl::Context *context) {}

    virtual angle::Result load(const gl::Context *context,
                               gl::BinaryInputStream *stream,
                               std::shared_ptr<LinkTask> *loadTaskOut,
                               egl::CacheGetResult *resultOut)                    = 0;
    virtual void save(const gl::Context *context, gl::BinaryOutputStream *stream) = 0;
    virtual void setBinaryRetrievableHint(bool retrievable)                       = 0;
    virtual void setSeparable(bool separable)                                     = 0;

    virtual void prepareForLink(const gl::ShaderMap<ShaderImpl *> &shaders) {}
    virtual angle::Result link(const gl::Context *context,
                               std::shared_ptr<LinkTask> *linkTaskOut) = 0;
    virtual GLboolean validate(const gl::Caps &caps)                   = 0;

    // Implementation-specific method for ignoring unreferenced uniforms. Some implementations may
    // perform more extensive analysis and ignore some locations that ANGLE doesn't detect as
    // unreferenced. This method is not required to be overriden by a back-end.
    virtual void markUnusedUniformLocations(std::vector<gl::VariableLocation> *uniformLocations,
                                            std::vector<gl::SamplerBinding> *samplerBindings,
                                            std::vector<gl::ImageBinding> *imageBindings)
    {}

    const gl::ProgramState &getState() const { return mState; }

    virtual angle::Result onLabelUpdate(const gl::Context *context);

    // Called when glUniformBlockBinding is called.
    virtual void onUniformBlockBinding(gl::UniformBlockIndex uniformBlockIndex) {}

  protected:
    const gl::ProgramState &mState;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_PROGRAMIMPL_H_
