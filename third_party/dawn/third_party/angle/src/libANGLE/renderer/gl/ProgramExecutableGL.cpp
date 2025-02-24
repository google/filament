//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramExecutableGL.cpp: Implementation of ProgramExecutableGL.

#include "libANGLE/renderer/gl/ProgramExecutableGL.h"

#include "common/string_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{
ProgramExecutableGL::ProgramExecutableGL(const gl::ProgramExecutable *executable)
    : ProgramExecutableImpl(executable),
      mHasAppliedTransformFeedbackVaryings(false),
      mClipDistanceEnabledUniformLocation(-1),
      mClipOriginUniformLocation(-1),
      mMultiviewBaseViewLayerIndexUniformLocation(-1),
      mProgramID(0),
      mFunctions(nullptr),
      mStateManager(nullptr)
{}

ProgramExecutableGL::~ProgramExecutableGL() {}

void ProgramExecutableGL::destroy(const gl::Context *context) {}

void ProgramExecutableGL::reset()
{
    mUniformRealLocationMap.clear();
    mUniformBlockRealLocationMap.clear();

    mClipDistanceEnabledUniformLocation         = -1;
    mClipOriginUniformLocation                  = -1;
    mMultiviewBaseViewLayerIndexUniformLocation = -1;
}

void ProgramExecutableGL::postLink(const FunctionsGL *functions,
                                   StateManagerGL *stateManager,
                                   const angle::FeaturesGL &features,
                                   GLuint programID)
{
    // Cache the following so the executable is capable of making the appropriate GL calls without
    // having to involve ProgramGL.
    mProgramID    = programID;
    mFunctions    = functions;
    mStateManager = stateManager;

    // Query the uniform information
    ASSERT(mUniformRealLocationMap.empty());
    const auto &uniformLocations = mExecutable->getUniformLocations();
    const auto &uniforms         = mExecutable->getUniforms();
    mUniformRealLocationMap.resize(uniformLocations.size(), GL_INVALID_INDEX);
    for (size_t uniformLocation = 0; uniformLocation < uniformLocations.size(); uniformLocation++)
    {
        const auto &entry = uniformLocations[uniformLocation];
        if (!entry.used())
        {
            continue;
        }

        // From the GLES 3.0.5 spec:
        // "Locations for sequential array indices are not required to be sequential."
        const gl::LinkedUniform &uniform     = uniforms[entry.index];
        const std::string &uniformMappedName = mExecutable->getUniformMappedNames()[entry.index];
        std::stringstream fullNameStr;
        if (uniform.isArray())
        {
            ASSERT(angle::EndsWith(uniformMappedName, "[0]"));
            fullNameStr << uniformMappedName.substr(0, uniformMappedName.length() - 3);
            fullNameStr << "[" << entry.arrayIndex << "]";
        }
        else
        {
            fullNameStr << uniformMappedName;
        }
        const std::string &fullName = fullNameStr.str();

        GLint realLocation = functions->getUniformLocation(programID, fullName.c_str());
        mUniformRealLocationMap[uniformLocation] = realLocation;
    }

    if (features.emulateClipDistanceState.enabled && mExecutable->hasClipDistance())
    {
        ASSERT(functions->standard == STANDARD_GL_ES);
        mClipDistanceEnabledUniformLocation =
            functions->getUniformLocation(programID, "angle_ClipDistanceEnabled");
        ASSERT(mClipDistanceEnabledUniformLocation != -1);
    }

    if (features.emulateClipOrigin.enabled)
    {
        ASSERT(functions->standard == STANDARD_GL_ES);
        mClipOriginUniformLocation = functions->getUniformLocation(programID, "angle_ClipOrigin");
    }

    if (mExecutable->usesMultiview())
    {
        mMultiviewBaseViewLayerIndexUniformLocation =
            functions->getUniformLocation(programID, "multiviewBaseViewLayerIndex");
        ASSERT(mMultiviewBaseViewLayerIndexUniformLocation != -1);
    }
}

void ProgramExecutableGL::updateEnabledClipDistances(uint8_t enabledClipDistancesPacked) const
{
    ASSERT(mExecutable->hasClipDistance());
    ASSERT(mClipDistanceEnabledUniformLocation != -1);

    ASSERT(mFunctions->programUniform1ui != nullptr);
    mFunctions->programUniform1ui(mProgramID, mClipDistanceEnabledUniformLocation,
                                  enabledClipDistancesPacked);
}

void ProgramExecutableGL::updateEmulatedClipOrigin(gl::ClipOrigin origin) const
{
    if (mClipOriginUniformLocation == -1)
    {
        // A driver may optimize away the uniform when gl_Position.y is always zero.
        return;
    }

    const float originValue = (origin == gl::ClipOrigin::LowerLeft) ? 1.0f : -1.0f;
    if (mFunctions->programUniform1f != nullptr)
    {
        mFunctions->programUniform1f(mProgramID, mClipOriginUniformLocation, originValue);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform1f(mClipOriginUniformLocation, originValue);
    }
}

void ProgramExecutableGL::enableLayeredRenderingPath(int baseViewIndex) const
{
    ASSERT(mExecutable->usesMultiview());
    ASSERT(mMultiviewBaseViewLayerIndexUniformLocation != -1);

    ASSERT(mFunctions->programUniform1i != nullptr);
    mFunctions->programUniform1i(mProgramID, mMultiviewBaseViewLayerIndexUniformLocation,
                                 baseViewIndex);
}

void ProgramExecutableGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->programUniform1fv != nullptr)
    {
        mFunctions->programUniform1fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform1fv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->programUniform2fv != nullptr)
    {
        mFunctions->programUniform2fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform2fv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->programUniform3fv != nullptr)
    {
        mFunctions->programUniform3fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform3fv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->programUniform4fv != nullptr)
    {
        mFunctions->programUniform4fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform4fv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->programUniform1iv != nullptr)
    {
        mFunctions->programUniform1iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform1iv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->programUniform2iv != nullptr)
    {
        mFunctions->programUniform2iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform2iv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->programUniform3iv != nullptr)
    {
        mFunctions->programUniform3iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform3iv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->programUniform4iv != nullptr)
    {
        mFunctions->programUniform4iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform4iv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->programUniform1uiv != nullptr)
    {
        mFunctions->programUniform1uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform1uiv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->programUniform2uiv != nullptr)
    {
        mFunctions->programUniform2uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform2uiv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->programUniform3uiv != nullptr)
    {
        mFunctions->programUniform3uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform3uiv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->programUniform4uiv != nullptr)
    {
        mFunctions->programUniform4uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniform4uiv(uniLoc(location), count, v);
    }
}

void ProgramExecutableGL::setUniformMatrix2fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    if (mFunctions->programUniformMatrix2fv != nullptr)
    {
        mFunctions->programUniformMatrix2fv(mProgramID, uniLoc(location), count, transpose, value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix3fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    if (mFunctions->programUniformMatrix3fv != nullptr)
    {
        mFunctions->programUniformMatrix3fv(mProgramID, uniLoc(location), count, transpose, value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix4fv(GLint location,
                                              GLsizei count,
                                              GLboolean transpose,
                                              const GLfloat *value)
{
    if (mFunctions->programUniformMatrix4fv != nullptr)
    {
        mFunctions->programUniformMatrix4fv(mProgramID, uniLoc(location), count, transpose, value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix2x3fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix2x3fv != nullptr)
    {
        mFunctions->programUniformMatrix2x3fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix2x3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix3x2fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix3x2fv != nullptr)
    {
        mFunctions->programUniformMatrix3x2fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix3x2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix2x4fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix2x4fv != nullptr)
    {
        mFunctions->programUniformMatrix2x4fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix2x4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix4x2fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix4x2fv != nullptr)
    {
        mFunctions->programUniformMatrix4x2fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix4x2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix3x4fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix3x4fv != nullptr)
    {
        mFunctions->programUniformMatrix3x4fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix3x4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::setUniformMatrix4x3fv(GLint location,
                                                GLsizei count,
                                                GLboolean transpose,
                                                const GLfloat *value)
{
    if (mFunctions->programUniformMatrix4x3fv != nullptr)
    {
        mFunctions->programUniformMatrix4x3fv(mProgramID, uniLoc(location), count, transpose,
                                              value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->uniformMatrix4x3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramExecutableGL::getUniformfv(const gl::Context *context,
                                       GLint location,
                                       GLfloat *params) const
{
    mFunctions->getUniformfv(mProgramID, uniLoc(location), params);
}

void ProgramExecutableGL::getUniformiv(const gl::Context *context,
                                       GLint location,
                                       GLint *params) const
{
    mFunctions->getUniformiv(mProgramID, uniLoc(location), params);
}

void ProgramExecutableGL::getUniformuiv(const gl::Context *context,
                                        GLint location,
                                        GLuint *params) const
{
    mFunctions->getUniformuiv(mProgramID, uniLoc(location), params);
}

void ProgramExecutableGL::setUniformBlockBinding(GLuint uniformBlockIndex,
                                                 GLuint uniformBlockBinding)
{
    // Lazy init
    if (mUniformBlockRealLocationMap.empty())
    {
        mUniformBlockRealLocationMap.reserve(mExecutable->getUniformBlocks().size());
        for (const gl::InterfaceBlock &uniformBlock : mExecutable->getUniformBlocks())
        {
            const std::string &mappedNameWithIndex = uniformBlock.mappedNameWithArrayIndex();
            GLuint blockIndex =
                mFunctions->getUniformBlockIndex(mProgramID, mappedNameWithIndex.c_str());
            mUniformBlockRealLocationMap.push_back(blockIndex);
        }
    }

    const GLuint realBlockIndex = mUniformBlockRealLocationMap[uniformBlockIndex];
    if (realBlockIndex != GL_INVALID_INDEX)
    {
        mFunctions->uniformBlockBinding(mProgramID, realBlockIndex, uniformBlockBinding);
    }
}

void ProgramExecutableGL::reapplyUBOBindings()
{
    const std::vector<gl::InterfaceBlock> &blocks = mExecutable->getUniformBlocks();
    for (size_t blockIndex = 0; blockIndex < blocks.size(); ++blockIndex)
    {
        if (blocks[blockIndex].activeShaders().any())
        {
            const GLuint index = static_cast<GLuint>(blockIndex);
            setUniformBlockBinding(index, mExecutable->getUniformBlockBinding(index));
        }
    }
}

void ProgramExecutableGL::syncUniformBlockBindings()
{
    for (size_t uniformBlockIndex : mDirtyUniformBlockBindings)
    {
        const GLuint index = static_cast<GLuint>(uniformBlockIndex);
        setUniformBlockBinding(index, mExecutable->getUniformBlockBinding(index));
    }

    mDirtyUniformBlockBindings.reset();
}
}  // namespace rx
