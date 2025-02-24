//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.h: Defines the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#ifndef LIBANGLE_PROGRAMPIPELINE_H_
#define LIBANGLE_PROGRAMPIPELINE_H_

#include <memory>

#include "common/angleutils.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class GLImplFactory;
class ProgramPipelineImpl;
}  // namespace rx

namespace gl
{
class Context;
class ProgramPipeline;

class ProgramPipelineState final : angle::NonCopyable
{
  public:
    ProgramPipelineState(rx::GLImplFactory *factory);
    ~ProgramPipelineState();

    const std::string &getLabel() const;

    ProgramExecutable &getExecutable() const
    {
        ASSERT(mExecutable);
        return *mExecutable;
    }

    const SharedProgramExecutable &getSharedExecutable() const
    {
        ASSERT(mExecutable);
        return mExecutable;
    }

    void activeShaderProgram(Program *shaderProgram);
    void useProgramStages(const Context *context,
                          const gl::ShaderBitSet &shaderTypes,
                          Program *shaderProgram,
                          std::vector<angle::ObserverBinding> *programObserverBindings,
                          std::vector<angle::ObserverBinding> *programExecutableObserverBindings);

    Program *getActiveShaderProgram() { return mActiveShaderProgram; }

    GLboolean isValid() const { return mValid; }

    const Program *getShaderProgram(ShaderType shaderType) const { return mPrograms[shaderType]; }
    const SharedProgramExecutable &getShaderProgramExecutable(ShaderType shaderType) const
    {
        return mExecutable->mPPOProgramExecutables[shaderType];
    }

    bool usesShaderProgram(ShaderProgramID program) const;

    void updateExecutableTextures();

    void updateExecutableSpecConstUsageBits();

  private:
    SharedProgramExecutable makeNewExecutable(
        rx::GLImplFactory *factory,
        ShaderMap<SharedProgramExecutable> &&ppoProgramExecutables);
    void useProgramStage(const Context *context,
                         ShaderType shaderType,
                         Program *shaderProgram,
                         angle::ObserverBinding *programObserverBinding,
                         angle::ObserverBinding *programExecutableObserverBinding);
    void destroyDiscardedExecutables(const Context *context);

    friend class ProgramPipeline;

    std::string mLabel;

    // The active shader program
    Program *mActiveShaderProgram;
    // The shader programs for each stage.
    ShaderMap<Program *> mPrograms;

    // Mapping from program's UBOs into the program executable's UBOs.
    ShaderMap<ProgramUniformBlockArray<GLuint>> mUniformBlockMap;

    // A list of executables to be garbage collected.  This is populated as the pipeline is
    // notified about program relinks, but cannot immediately destroy the old executables due to
    // lack of access to context.
    std::vector<SharedProgramExecutable> mProgramExecutablesToDiscard;

    GLboolean mValid;

    InfoLog mInfoLog;

    SharedProgramExecutable mExecutable;

    bool mIsLinked;
};

class ProgramPipeline final : public RefCountObject<ProgramPipelineID>,
                              public LabeledObject,
                              public angle::ObserverInterface,
                              public angle::Subject
{
  public:
    ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle);
    ~ProgramPipeline() override;

    void onDestroy(const Context *context) override;

    angle::Result setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    const ProgramPipelineState &getState() const { return mState; }
    ProgramPipelineState &getState() { return mState; }

    ProgramExecutable &getExecutable() const { return mState.getExecutable(); }
    const SharedProgramExecutable &getSharedExecutable() const
    {
        return mState.getSharedExecutable();
    }

    rx::ProgramPipelineImpl *getImplementation() const;

    Program *getActiveShaderProgram() { return mState.getActiveShaderProgram(); }
    void activeShaderProgram(Program *shaderProgram);
    Program *getLinkedActiveShaderProgram(const Context *context)
    {
        Program *program = mState.getActiveShaderProgram();
        if (program)
        {
            program->resolveLink(context);
        }
        return program;
    }

    angle::Result useProgramStages(const Context *context,
                                   GLbitfield stages,
                                   Program *shaderProgram);

    const Program *getShaderProgram(ShaderType shaderType) const
    {
        return mState.getShaderProgram(shaderType);
    }
    const SharedProgramExecutable &getShaderProgramExecutable(ShaderType shaderType) const
    {
        return mState.getShaderProgramExecutable(shaderType);
    }

    void resetIsLinked() { mState.mIsLinked = false; }
    angle::Result link(const gl::Context *context);

    InfoLog &getInfoLog() { return mState.mInfoLog; }
    int getInfoLogLength() const;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const;

    // Ensure program pipeline is linked. Inlined to make sure its overhead is as low as possible.
    void resolveLink(const Context *context)
    {
        if (mState.mIsLinked)
        {
            // Already linked, nothing to do.
            return;
        }

        resolveAttachedPrograms(context);
        angle::Result linkResult = link(context);
        if (linkResult != angle::Result::Continue)
        {
            // If the link failed then log a warning, swallow the error and move on.
            WARN() << "ProgramPipeline link failed" << std::endl;
        }
        return;
    }
    void resolveAttachedPrograms(const Context *context);

    void validate(const gl::Context *context);
    GLboolean isValid() const { return mState.isValid(); }
    bool isLinked() const { return mState.mIsLinked; }

    // ObserverInterface implementation.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

  private:
    bool linkVaryings();
    void updateLinkedShaderStages();
    void updateExecutableAttributes();
    void updateTransformFeedbackMembers();
    void updateShaderStorageBlocks();
    void updateImageBindings();
    void updateExecutableGeometryProperties();
    void updateExecutableTessellationProperties();
    void updateFragmentInoutRangeAndEnablesPerSampleShading();
    void updateLinkedVaryings();
    void updateExecutable();

    std::unique_ptr<rx::ProgramPipelineImpl> mProgramPipelineImpl;

    ProgramPipelineState mState;

    std::vector<angle::ObserverBinding> mProgramObserverBindings;
    std::vector<angle::ObserverBinding> mProgramExecutableObserverBindings;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAMPIPELINE_H_
