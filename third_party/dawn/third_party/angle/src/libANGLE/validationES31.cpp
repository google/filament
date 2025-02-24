//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES31.cpp: Validation functions for OpenGL ES 3.1 entry point parameters

#include "libANGLE/validationES31_autogen.h"

#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/ProgramExecutable.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES2_autogen.h"
#include "libANGLE/validationES31.h"
#include "libANGLE/validationES3_autogen.h"

#include "common/utilities.h"

using namespace angle;

namespace gl
{
using namespace err;

namespace
{

bool ValidateNamedProgramInterface(GLenum programInterface)
{
    switch (programInterface)
    {
        case GL_UNIFORM:
        case GL_UNIFORM_BLOCK:
        case GL_PROGRAM_INPUT:
        case GL_PROGRAM_OUTPUT:
        case GL_TRANSFORM_FEEDBACK_VARYING:
        case GL_BUFFER_VARIABLE:
        case GL_SHADER_STORAGE_BLOCK:
            return true;
        default:
            return false;
    }
}

bool ValidateLocationProgramInterface(GLenum programInterface)
{
    switch (programInterface)
    {
        case GL_UNIFORM:
        case GL_PROGRAM_INPUT:
        case GL_PROGRAM_OUTPUT:
            return true;
        default:
            return false;
    }
}

bool ValidateProgramInterface(GLenum programInterface)
{
    return (programInterface == GL_ATOMIC_COUNTER_BUFFER ||
            ValidateNamedProgramInterface(programInterface));
}

bool ValidateProgramResourceProperty(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLenum prop)
{
    ASSERT(context);
    switch (prop)
    {
        case GL_ACTIVE_VARIABLES:
        case GL_BUFFER_BINDING:
        case GL_NUM_ACTIVE_VARIABLES:

        case GL_ARRAY_SIZE:

        case GL_ARRAY_STRIDE:
        case GL_BLOCK_INDEX:
        case GL_IS_ROW_MAJOR:
        case GL_MATRIX_STRIDE:

        case GL_ATOMIC_COUNTER_BUFFER_INDEX:

        case GL_BUFFER_DATA_SIZE:

        case GL_LOCATION:

        case GL_NAME_LENGTH:

        case GL_OFFSET:

        case GL_REFERENCED_BY_VERTEX_SHADER:
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
        case GL_REFERENCED_BY_COMPUTE_SHADER:

        case GL_TOP_LEVEL_ARRAY_SIZE:
        case GL_TOP_LEVEL_ARRAY_STRIDE:

        case GL_TYPE:
            return true;

        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
            return context->getExtensions().geometryShaderAny() ||
                   context->getClientVersion() >= ES_3_2;

        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
        case GL_IS_PER_PATCH_EXT:
            return context->getExtensions().tessellationShaderAny() ||
                   context->getClientVersion() >= ES_3_2;

        case GL_LOCATION_INDEX_EXT:
            return context->getExtensions().blendFuncExtendedEXT;

        default:
            return false;
    }
}

// GLES 3.10 spec: Page 82 -- Table 7.2
bool ValidateProgramResourcePropertyByInterface(GLenum prop, GLenum programInterface)
{
    switch (prop)
    {
        case GL_ACTIVE_VARIABLES:
        case GL_BUFFER_BINDING:
        case GL_NUM_ACTIVE_VARIABLES:
        {
            switch (programInterface)
            {
                case GL_ATOMIC_COUNTER_BUFFER:
                case GL_SHADER_STORAGE_BLOCK:
                case GL_UNIFORM_BLOCK:
                    return true;
                default:
                    return false;
            }
        }

        case GL_ARRAY_SIZE:
        {
            switch (programInterface)
            {
                case GL_BUFFER_VARIABLE:
                case GL_PROGRAM_INPUT:
                case GL_PROGRAM_OUTPUT:
                case GL_TRANSFORM_FEEDBACK_VARYING:
                case GL_UNIFORM:
                    return true;
                default:
                    return false;
            }
        }

        case GL_ARRAY_STRIDE:
        case GL_BLOCK_INDEX:
        case GL_IS_ROW_MAJOR:
        case GL_MATRIX_STRIDE:
        {
            switch (programInterface)
            {
                case GL_BUFFER_VARIABLE:
                case GL_UNIFORM:
                    return true;
                default:
                    return false;
            }
        }

        case GL_ATOMIC_COUNTER_BUFFER_INDEX:
        {
            if (programInterface == GL_UNIFORM)
            {
                return true;
            }
            return false;
        }

        case GL_BUFFER_DATA_SIZE:
        {
            switch (programInterface)
            {
                case GL_ATOMIC_COUNTER_BUFFER:
                case GL_SHADER_STORAGE_BLOCK:
                case GL_UNIFORM_BLOCK:
                    return true;
                default:
                    return false;
            }
        }

        case GL_LOCATION:
        {
            return ValidateLocationProgramInterface(programInterface);
        }

        case GL_LOCATION_INDEX_EXT:
        {
            // EXT_blend_func_extended
            return (programInterface == GL_PROGRAM_OUTPUT);
        }

        case GL_NAME_LENGTH:
        {
            return ValidateNamedProgramInterface(programInterface);
        }

        case GL_OFFSET:
        {
            switch (programInterface)
            {
                case GL_BUFFER_VARIABLE:
                case GL_UNIFORM:
                    return true;
                default:
                    return false;
            }
        }

        case GL_REFERENCED_BY_VERTEX_SHADER:
        case GL_REFERENCED_BY_FRAGMENT_SHADER:
        case GL_REFERENCED_BY_COMPUTE_SHADER:
        case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
        case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
        case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
        {
            switch (programInterface)
            {
                case GL_ATOMIC_COUNTER_BUFFER:
                case GL_BUFFER_VARIABLE:
                case GL_PROGRAM_INPUT:
                case GL_PROGRAM_OUTPUT:
                case GL_SHADER_STORAGE_BLOCK:
                case GL_UNIFORM:
                case GL_UNIFORM_BLOCK:
                    return true;
                default:
                    return false;
            }
        }

        case GL_TOP_LEVEL_ARRAY_SIZE:
        case GL_TOP_LEVEL_ARRAY_STRIDE:
        {
            if (programInterface == GL_BUFFER_VARIABLE)
            {
                return true;
            }
            return false;
        }

        case GL_TYPE:
        {
            switch (programInterface)
            {
                case GL_BUFFER_VARIABLE:
                case GL_PROGRAM_INPUT:
                case GL_PROGRAM_OUTPUT:
                case GL_TRANSFORM_FEEDBACK_VARYING:
                case GL_UNIFORM:
                    return true;
                default:
                    return false;
            }
        }
        case GL_IS_PER_PATCH_EXT:
            switch (programInterface)
            {
                case GL_PROGRAM_INPUT:
                case GL_PROGRAM_OUTPUT:
                    return true;
            }
            return false;

        default:
            return false;
    }
}

bool ValidateProgramResourceIndex(const Program *programObject,
                                  GLenum programInterface,
                                  GLuint index)
{
    const ProgramExecutable &executable = programObject->getExecutable();
    switch (programInterface)
    {
        case GL_PROGRAM_INPUT:
            return index < executable.getProgramInputs().size();

        case GL_PROGRAM_OUTPUT:
            return index < executable.getOutputVariables().size();

        case GL_UNIFORM:
            return index < executable.getUniforms().size();

        case GL_BUFFER_VARIABLE:
            return index < executable.getBufferVariables().size();

        case GL_SHADER_STORAGE_BLOCK:
            return index < executable.getShaderStorageBlocks().size();

        case GL_UNIFORM_BLOCK:
            return index < executable.getUniformBlocks().size();

        case GL_ATOMIC_COUNTER_BUFFER:
            return index < executable.getAtomicCounterBuffers().size();

        case GL_TRANSFORM_FEEDBACK_VARYING:
            return index < executable.getLinkedTransformFeedbackVaryings().size();

        default:
            UNREACHABLE();
            return false;
    }
}

bool ValidateProgramUniformBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLenum valueType,
                                ShaderProgramID program,
                                UniformLocation location,
                                GLsizei count)
{
    const LinkedUniform *uniform = nullptr;
    Program *programObject       = GetValidProgram(context, entryPoint, program);
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniformValue(context, entryPoint, valueType, uniform->getType());
}

bool ValidateProgramUniformMatrixBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum valueType,
                                      ShaderProgramID program,
                                      UniformLocation location,
                                      GLsizei count,
                                      GLboolean transpose)
{
    const LinkedUniform *uniform = nullptr;
    Program *programObject       = GetValidProgram(context, entryPoint, program);
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniformMatrixValue(context, entryPoint, valueType, uniform->getType());
}

bool ValidateVertexAttribFormatCommon(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLuint relativeOffset)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (relativeOffset > static_cast<GLuint>(caps.maxVertexAttribRelativeOffset))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kRelativeOffsetTooLarge);
        return false;
    }

    // [OpenGL ES 3.1] Section 10.3.1 page 243:
    // An INVALID_OPERATION error is generated if the default vertex array object is bound.
    if (context->getState().getVertexArrayId().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultVertexArray);
        return false;
    }

    return true;
}

}  // anonymous namespace

bool ValidateGetBooleani_v(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLenum target,
                           GLuint index,
                           const GLboolean *data)
{
    if (context->getClientVersion() < ES_3_1 && !context->getExtensions().drawBuffersIndexedAny())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                               kES31OrDrawBuffersIndexedExtensionNotAvailable);
        return false;
    }

    if (!ValidateIndexedStateQuery(context, entryPoint, target, index, nullptr))
    {
        return false;
    }

    return true;
}

bool ValidateGetBooleani_vRobustANGLE(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum target,
                                      GLuint index,
                                      GLsizei bufSize,
                                      const GLsizei *length,
                                      const GLboolean *data)
{
    if (context->getClientVersion() < ES_3_1 && !context->getExtensions().drawBuffersIndexedAny())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                               kES31OrDrawBuffersIndexedExtensionNotAvailable);
        return false;
    }

    if (!ValidateRobustEntryPoint(context, entryPoint, bufSize))
    {
        return false;
    }

    GLsizei numParams = 0;

    if (!ValidateIndexedStateQuery(context, entryPoint, target, index, &numParams))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, entryPoint, bufSize, numParams))
    {
        return false;
    }

    SetRobustLengthParam(length, numParams);
    return true;
}

bool ValidateDrawIndirectBase(const Context *context,
                              angle::EntryPoint entryPoint,
                              PrimitiveMode mode,
                              const void *indirect)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    // Here the third parameter 1 is only to pass the count validation.
    if (!ValidateDrawBase(context, entryPoint, mode))
    {
        return false;
    }

    const State &state = context->getState();

    // An INVALID_OPERATION error is generated if zero is bound to VERTEX_ARRAY_BINDING,
    // DRAW_INDIRECT_BUFFER or to any enabled vertex array.
    if (state.getVertexArrayId().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultVertexArray);
        return false;
    }

    if (context->getStateCache().hasAnyActiveClientAttrib())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kClientDataInVertexArray);
        return false;
    }

    Buffer *drawIndirectBuffer = state.getTargetBuffer(BufferBinding::DrawIndirect);
    if (!drawIndirectBuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDrawIndirectBufferNotBound);
        return false;
    }

    // An INVALID_VALUE error is generated if indirect is not a multiple of the size, in basic
    // machine units, of uint.
    GLint64 offset = reinterpret_cast<GLint64>(indirect);
    if ((static_cast<GLuint>(offset) % sizeof(GLuint)) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidIndirectOffset);
        return false;
    }

    return true;
}

bool ValidateDrawArraysIndirect(const Context *context,
                                angle::EntryPoint entryPoint,
                                PrimitiveMode mode,
                                const void *indirect)
{
    const State &state                      = context->getState();
    TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
    if (curTransformFeedback && curTransformFeedback->isActive() &&
        !curTransformFeedback->isPaused())
    {
        // EXT_geometry_shader allows transform feedback to work with all draw commands.
        // [EXT_geometry_shader] Section 12.1, "Transform Feedback"
        if (context->getExtensions().geometryShaderAny() || context->getClientVersion() >= ES_3_2)
        {
            if (!ValidateTransformFeedbackPrimitiveMode(
                    context, entryPoint, curTransformFeedback->getPrimitiveMode(), mode))
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidDrawModeTransformFeedback);
                return false;
            }
        }
        else
        {
            // An INVALID_OPERATION error is generated if transform feedback is active and not
            // paused.
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kUnsupportedDrawModeForTransformFeedback);
            return false;
        }
    }

    if (!ValidateDrawIndirectBase(context, entryPoint, mode, indirect))
        return false;

    Buffer *drawIndirectBuffer = state.getTargetBuffer(BufferBinding::DrawIndirect);
    CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(indirect));
    // In OpenGL ES3.1 spec, session 10.5, it defines the struct of DrawArraysIndirectCommand
    // which's size is 4 * sizeof(uint).
    auto checkedSum = checkedOffset + 4 * sizeof(GLuint);
    if (!checkedSum.IsValid() ||
        checkedSum.ValueOrDie() > static_cast<size_t>(drawIndirectBuffer->getSize()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kParamOverflow);
        return false;
    }

    return true;
}

bool ValidateDrawElementsIndirect(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  PrimitiveMode mode,
                                  DrawElementsType type,
                                  const void *indirect)
{
    if (!ValidateDrawElementsBase(context, entryPoint, mode, type))
    {
        return false;
    }

    const State &state         = context->getState();
    const VertexArray *vao     = state.getVertexArray();
    Buffer *elementArrayBuffer = vao->getElementArrayBuffer();
    if (!elementArrayBuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMustHaveElementArrayBinding);
        return false;
    }

    if (!ValidateDrawIndirectBase(context, entryPoint, mode, indirect))
        return false;

    Buffer *drawIndirectBuffer = state.getTargetBuffer(BufferBinding::DrawIndirect);
    CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(indirect));
    // In OpenGL ES3.1 spec, session 10.5, it defines the struct of DrawElementsIndirectCommand
    // which's size is 5 * sizeof(uint).
    auto checkedSum = checkedOffset + 5 * sizeof(GLuint);
    if (!checkedSum.IsValid() ||
        checkedSum.ValueOrDie() > static_cast<size_t>(drawIndirectBuffer->getSize()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kParamOverflow);
        return false;
    }

    return true;
}

bool ValidateMultiDrawIndirectBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLsizei drawcount,
                                   GLsizei stride)
{
    if (!context->getExtensions().multiDrawIndirectEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    // An INVALID_VALUE error is generated if stride is neither 0 nor a multiple of 4.
    if ((stride & 3) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidDrawBufferValue);
        return false;
    }

    // An INVALID_VALUE error is generated if drawcount is not positive.
    if (drawcount <= 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidValueNonPositive);
        return false;
    }

    return true;
}

bool ValidateProgramUniform1iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLint v0)
{
    return ValidateProgramUniform1ivBase(context, entryPoint, program, location, 1, &v0);
}

bool ValidateProgramUniform2iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLint v0,
                                  GLint v1)
{
    GLint xy[2] = {v0, v1};
    return ValidateProgramUniform2ivBase(context, entryPoint, program, location, 1, xy);
}

bool ValidateProgramUniform3iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLint v0,
                                  GLint v1,
                                  GLint v2)
{
    GLint xyz[3] = {v0, v1, v2};
    return ValidateProgramUniform3ivBase(context, entryPoint, program, location, 1, xyz);
}

bool ValidateProgramUniform4iBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLint v0,
                                  GLint v1,
                                  GLint v2,
                                  GLint v3)
{
    GLint xyzw[4] = {v0, v1, v2, v3};
    return ValidateProgramUniform4ivBase(context, entryPoint, program, location, 1, xyzw);
}

bool ValidateProgramUniform1uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLuint v0)
{
    return ValidateProgramUniform1uivBase(context, entryPoint, program, location, 1, &v0);
}

bool ValidateProgramUniform2uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLuint v0,
                                   GLuint v1)
{
    GLuint xy[2] = {v0, v1};
    return ValidateProgramUniform2uivBase(context, entryPoint, program, location, 1, xy);
}

bool ValidateProgramUniform3uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLuint v0,
                                   GLuint v1,
                                   GLuint v2)
{
    GLuint xyz[3] = {v0, v1, v2};
    return ValidateProgramUniform3uivBase(context, entryPoint, program, location, 1, xyz);
}

bool ValidateProgramUniform4uiBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLuint v0,
                                   GLuint v1,
                                   GLuint v2,
                                   GLuint v3)
{
    GLuint xyzw[4] = {v0, v1, v2, v3};
    return ValidateProgramUniform4uivBase(context, entryPoint, program, location, 1, xyzw);
}

bool ValidateProgramUniform1fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLfloat v0)
{
    return ValidateProgramUniform1fvBase(context, entryPoint, program, location, 1, &v0);
}

bool ValidateProgramUniform2fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLfloat v0,
                                  GLfloat v1)
{
    GLfloat xy[2] = {v0, v1};
    return ValidateProgramUniform2fvBase(context, entryPoint, program, location, 1, xy);
}

bool ValidateProgramUniform3fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLfloat v0,
                                  GLfloat v1,
                                  GLfloat v2)
{
    GLfloat xyz[3] = {v0, v1, v2};
    return ValidateProgramUniform3fvBase(context, entryPoint, program, location, 1, xyz);
}

bool ValidateProgramUniform4fBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  UniformLocation location,
                                  GLfloat v0,
                                  GLfloat v1,
                                  GLfloat v2,
                                  GLfloat v3)
{
    GLfloat xyzw[4] = {v0, v1, v2, v3};
    return ValidateProgramUniform4fvBase(context, entryPoint, program, location, 1, xyzw);
}

bool ValidateProgramUniform1ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLint *value)
{
    const LinkedUniform *uniform = nullptr;
    Program *programObject       = GetValidProgram(context, entryPoint, program);
    return ValidateUniformCommonBase(context, entryPoint, programObject, location, count,
                                     &uniform) &&
           ValidateUniform1ivValue(context, entryPoint, uniform->getType(), count, value);
}

bool ValidateProgramUniform2ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_INT_VEC2, program, location, count);
}

bool ValidateProgramUniform3ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_INT_VEC3, program, location, count);
}

bool ValidateProgramUniform4ivBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_INT_VEC4, program, location, count);
}

bool ValidateProgramUniform1uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLuint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_UNSIGNED_INT, program, location,
                                      count);
}

bool ValidateProgramUniform2uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLuint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_UNSIGNED_INT_VEC2, program, location,
                                      count);
}

bool ValidateProgramUniform3uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLuint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_UNSIGNED_INT_VEC3, program, location,
                                      count);
}

bool ValidateProgramUniform4uivBase(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID program,
                                    UniformLocation location,
                                    GLsizei count,
                                    const GLuint *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_UNSIGNED_INT_VEC4, program, location,
                                      count);
}

bool ValidateProgramUniform1fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLfloat *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_FLOAT, program, location, count);
}

bool ValidateProgramUniform2fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLfloat *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_FLOAT_VEC2, program, location, count);
}

bool ValidateProgramUniform3fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLfloat *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_FLOAT_VEC3, program, location, count);
}

bool ValidateProgramUniform4fvBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   UniformLocation location,
                                   GLsizei count,
                                   const GLfloat *value)
{
    return ValidateProgramUniformBase(context, entryPoint, GL_FLOAT_VEC4, program, location, count);
}

bool ValidateProgramUniformMatrix2fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformLocation location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT2, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix3fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformLocation location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT3, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix4fvBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ShaderProgramID program,
                                         UniformLocation location,
                                         GLsizei count,
                                         GLboolean transpose,
                                         const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT4, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix2x3fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT2x3, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix3x2fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT3x2, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix2x4fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT2x4, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix4x2fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT4x2, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix3x4fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT3x4, program, location,
                                            count, transpose);
}

bool ValidateProgramUniformMatrix4x3fvBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ShaderProgramID program,
                                           UniformLocation location,
                                           GLsizei count,
                                           GLboolean transpose,
                                           const GLfloat *value)
{
    return ValidateProgramUniformMatrixBase(context, entryPoint, GL_FLOAT_MAT4x3, program, location,
                                            count, transpose);
}

bool ValidateGetTexLevelParameterfv(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    TextureTarget target,
                                    GLint level,
                                    GLenum pname,
                                    const GLfloat *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetTexLevelParameterBase(context, entryPoint, target, level, pname, nullptr);
}

bool ValidateGetTexLevelParameterfvRobustANGLE(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               TextureTarget target,
                                               GLint level,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               const GLsizei *length,
                                               const GLfloat *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetTexLevelParameteriv(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    TextureTarget target,
                                    GLint level,
                                    GLenum pname,
                                    const GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetTexLevelParameterBase(context, entryPoint, target, level, pname, nullptr);
}

bool ValidateGetTexLevelParameterivRobustANGLE(const Context *context,
                                               angle::EntryPoint entryPoint,
                                               TextureTarget target,
                                               GLint level,
                                               GLenum pname,
                                               GLsizei bufSize,
                                               const GLsizei *length,
                                               const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateTexStorage2DMultisample(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     TextureType target,
                                     GLsizei samples,
                                     GLenum internalFormat,
                                     GLsizei width,
                                     GLsizei height,
                                     GLboolean fixedSampleLocations)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateTexStorage2DMultisampleBase(context, entryPoint, target, samples, internalFormat,
                                               width, height);
}

bool ValidateTexStorageMem2DMultisampleEXT(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei samples,
                                           GLenum internalFormat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLboolean fixedSampleLocations,
                                           MemoryObjectID memory,
                                           GLuint64 offset)
{
    if (!context->getExtensions().memoryObjectEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    UNIMPLEMENTED();
    return false;
}

bool ValidateGetMultisamplefv(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLenum pname,
                              GLuint index,
                              const GLfloat *val)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetMultisamplefvBase(context, entryPoint, pname, index, val);
}

bool ValidateGetMultisamplefvRobustANGLE(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         GLenum pname,
                                         GLuint index,
                                         GLsizei bufSize,
                                         const GLsizei *length,
                                         const GLfloat *val)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateFramebufferParameteri(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLenum pname,
                                   GLint param)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateFramebufferParameteriBase(context, entryPoint, target, pname, param);
}

bool ValidateGetFramebufferParameteriv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       GLenum target,
                                       GLenum pname,
                                       const GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetFramebufferParameterivBase(context, entryPoint, target, pname, params);
}

bool ValidateGetFramebufferParameterivRobustANGLE(const Context *context,
                                                  angle::EntryPoint entryPoint,
                                                  GLenum target,
                                                  GLenum pname,
                                                  GLsizei bufSize,
                                                  const GLsizei *length,
                                                  const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGetProgramResourceIndex(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID program,
                                     GLenum programInterface,
                                     const GLchar *name)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    if (!ValidateNamedProgramInterface(programInterface))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramInterface);
        return false;
    }

    return true;
}

bool ValidateBindVertexBuffer(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLuint bindingIndex,
                              BufferID buffer,
                              GLintptr offset,
                              GLsizei stride)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    if (!context->isBufferGenerated(buffer))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (bindingIndex >= static_cast<GLuint>(caps.maxVertexAttribBindings))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribBindings);
        return false;
    }

    if (offset < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if (stride < 0 || stride > caps.maxVertexAttribStride)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribStride);
        return false;
    }

    // [OpenGL ES 3.1] Section 10.3.1 page 244:
    // An INVALID_OPERATION error is generated if the default vertex array object is bound.
    if (context->getState().getVertexArrayId().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultVertexArray);
        return false;
    }

    return true;
}

bool ValidateVertexBindingDivisor(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  GLuint bindingIndex,
                                  GLuint divisor)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (bindingIndex >= static_cast<GLuint>(caps.maxVertexAttribBindings))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribBindings);
        return false;
    }

    // [OpenGL ES 3.1] Section 10.3.1 page 243:
    // An INVALID_OPERATION error is generated if the default vertex array object is bound.
    if (context->getState().getVertexArrayId().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultVertexArray);
        return false;
    }

    return true;
}

bool ValidateVertexAttribFormat(const Context *context,
                                angle::EntryPoint entryPoint,
                                GLuint attribindex,
                                GLint size,
                                VertexAttribType type,
                                GLboolean normalized,
                                GLuint relativeoffset)
{
    if (!ValidateVertexAttribFormatCommon(context, entryPoint, relativeoffset))
    {
        return false;
    }

    return ValidateFloatVertexFormat(context, entryPoint, attribindex, size, type);
}

bool ValidateVertexAttribIFormat(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint attribindex,
                                 GLint size,
                                 VertexAttribType type,
                                 GLuint relativeoffset)
{
    if (!ValidateVertexAttribFormatCommon(context, entryPoint, relativeoffset))
    {
        return false;
    }

    return ValidateIntegerVertexFormat(context, entryPoint, attribindex, size, type);
}

bool ValidateVertexAttribBinding(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLuint attribIndex,
                                 GLuint bindingIndex)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    // [OpenGL ES 3.1] Section 10.3.1 page 243:
    // An INVALID_OPERATION error is generated if the default vertex array object is bound.
    if (context->getState().getVertexArrayId().value == 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDefaultVertexArray);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (attribIndex >= static_cast<GLuint>(caps.maxVertexAttributes))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kIndexExceedsMaxVertexAttribute);
        return false;
    }

    if (bindingIndex >= static_cast<GLuint>(caps.maxVertexAttribBindings))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxVertexAttribBindings);
        return false;
    }

    return true;
}

bool ValidateGetProgramResourceName(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    ShaderProgramID program,
                                    GLenum programInterface,
                                    GLuint index,
                                    GLsizei bufSize,
                                    const GLsizei *length,
                                    const GLchar *name)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    if (!ValidateNamedProgramInterface(programInterface))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramInterface);
        return false;
    }

    if (!ValidateProgramResourceIndex(programObject, programInterface, index))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramResourceIndex);
        return false;
    }

    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    return true;
}

bool ValidateDispatchCompute(const Context *context,
                             angle::EntryPoint entryPoint,
                             GLuint numGroupsX,
                             GLuint numGroupsY,
                             GLuint numGroupsZ)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    const State &state                  = context->getState();
    const ProgramExecutable *executable = state.getLinkedProgramExecutable(context);

    if (executable == nullptr || !executable->hasLinkedShaderStage(ShaderType::Compute))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoActiveProgramWithComputeShader);
        return false;
    }

    const Caps &caps = context->getCaps();
    if (numGroupsX > static_cast<GLuint>(caps.maxComputeWorkGroupCount[0]))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsComputeWorkGroupCountX);
        return false;
    }
    if (numGroupsY > static_cast<GLuint>(caps.maxComputeWorkGroupCount[1]))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsComputeWorkGroupCountY);
        return false;
    }
    if (numGroupsZ > static_cast<GLuint>(caps.maxComputeWorkGroupCount[2]))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsComputeWorkGroupCountZ);
        return false;
    }

    return true;
}

bool ValidateDispatchComputeIndirect(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLintptr indirect)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    const State &state                  = context->getState();
    const ProgramExecutable *executable = state.getProgramExecutable();

    if (executable == nullptr || !executable->hasLinkedShaderStage(ShaderType::Compute))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kNoActiveProgramWithComputeShader);
        return false;
    }

    if (indirect < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeOffset);
        return false;
    }

    if ((indirect & (sizeof(GLuint) - 1)) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kOffsetMustBeMultipleOfUint);
        return false;
    }

    Buffer *dispatchIndirectBuffer = state.getTargetBuffer(BufferBinding::DispatchIndirect);
    if (!dispatchIndirectBuffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kDispatchIndirectBufferNotBound);
        return false;
    }

    CheckedNumeric<GLuint64> checkedOffset(static_cast<GLuint64>(indirect));
    auto checkedSum = checkedOffset + static_cast<GLuint64>(3 * sizeof(GLuint));
    if (!checkedSum.IsValid() ||
        checkedSum.ValueOrDie() > static_cast<GLuint64>(dispatchIndirectBuffer->getSize()))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInsufficientBufferSize);
        return false;
    }

    return true;
}

bool ValidateBindImageTexture(const Context *context,
                              angle::EntryPoint entryPoint,
                              GLuint unit,
                              TextureID texture,
                              GLint level,
                              GLboolean layered,
                              GLint layer,
                              GLenum access,
                              GLenum format)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    GLuint maxImageUnits = static_cast<GLuint>(context->getCaps().maxImageUnits);
    if (unit >= maxImageUnits)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kExceedsMaxImageUnits);
        return false;
    }

    if (level < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLevel);
        return false;
    }

    if (layer < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeLayer);
        return false;
    }

    if (access != GL_READ_ONLY && access != GL_WRITE_ONLY && access != GL_READ_WRITE)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidImageAccess);
        return false;
    }

    switch (format)
    {
        case GL_RGBA32F:
        case GL_RGBA16F:
        case GL_R32F:
        case GL_RGBA32UI:
        case GL_RGBA16UI:
        case GL_RGBA8UI:
        case GL_R32UI:
        case GL_RGBA32I:
        case GL_RGBA16I:
        case GL_RGBA8I:
        case GL_R32I:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
            break;
        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidImageFormat);
            return false;
    }

    if (texture.value != 0)
    {
        Texture *tex = context->getTexture(texture);

        if (tex == nullptr)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kMissingTextureName);
            return false;
        }

        if (!tex->getImmutableFormat() && tex->getType() != gl::TextureType::Buffer)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION,
                                   kTextureIsNeitherImmutableNorTextureBuffer);
            return false;
        }

        if (context->getExtensions().textureStorageCompressionEXT &&
            tex->getType() != gl::TextureType::Buffer)
        {
            if (tex->getImageCompressionRate(context) != GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE,
                                       kTextureFixedCompressedNotSupportBindImageTexture);
                return false;
            }
        }
    }

    return true;
}

bool ValidateGetProgramResourceLocation(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        ShaderProgramID program,
                                        GLenum programInterface,
                                        const GLchar *name)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    if (!ValidateLocationProgramInterface(programInterface))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramInterface);
        return false;
    }
    return true;
}

bool ValidateGetProgramResourceiv(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderProgramID program,
                                  GLenum programInterface,
                                  GLuint index,
                                  GLsizei propCount,
                                  const GLenum *props,
                                  GLsizei bufSize,
                                  const GLsizei *length,
                                  const GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }
    if (!ValidateProgramInterface(programInterface))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramInterface);
        return false;
    }
    if (propCount <= 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidPropCount);
        return false;
    }
    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }
    if (!ValidateProgramResourceIndex(programObject, programInterface, index))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidProgramResourceIndex);
        return false;
    }
    for (GLsizei i = 0; i < propCount; i++)
    {
        if (!ValidateProgramResourceProperty(context, entryPoint, props[i]))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramResourceProperty);
            return false;
        }
        if (!ValidateProgramResourcePropertyByInterface(props[i], programInterface))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kInvalidPropertyForProgramInterface);
            return false;
        }
    }
    return true;
}

bool ValidateGetProgramInterfaceiv(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ShaderProgramID program,
                                   GLenum programInterface,
                                   GLenum pname,
                                   const GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (programObject == nullptr)
    {
        return false;
    }

    if (!ValidateProgramInterface(programInterface))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidProgramInterface);
        return false;
    }

    switch (pname)
    {
        case GL_ACTIVE_RESOURCES:
        case GL_MAX_NAME_LENGTH:
        case GL_MAX_NUM_ACTIVE_VARIABLES:
            break;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    if (pname == GL_MAX_NAME_LENGTH && programInterface == GL_ATOMIC_COUNTER_BUFFER)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kAtomicCounterResourceName);
        return false;
    }

    if (pname == GL_MAX_NUM_ACTIVE_VARIABLES)
    {
        switch (programInterface)
        {
            case GL_ATOMIC_COUNTER_BUFFER:
            case GL_SHADER_STORAGE_BLOCK:
            case GL_UNIFORM_BLOCK:
                break;

            default:
                ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kMaxActiveVariablesInterface);
                return false;
        }
    }

    return true;
}

bool ValidateGetProgramInterfaceivRobustANGLE(const Context *context,
                                              angle::EntryPoint entryPoint,
                                              ShaderProgramID program,
                                              GLenum programInterface,
                                              GLenum pname,
                                              GLsizei bufSize,
                                              const GLsizei *length,
                                              const GLint *params)
{
    UNIMPLEMENTED();
    return false;
}

bool ValidateGenProgramPipelinesBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     GLsizei n,
                                     const ProgramPipelineID *pipelines)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateDeleteProgramPipelinesBase(const Context *context,
                                        angle::EntryPoint entryPoint,
                                        GLsizei n,
                                        const ProgramPipelineID *pipelines)
{
    return ValidateGenOrDelete(context, entryPoint, n);
}

bool ValidateBindProgramPipelineBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ProgramPipelineID pipeline)
{
    if (!context->isProgramPipelineGenerated({pipeline}))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    return true;
}

bool ValidateIsProgramPipelineBase(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   ProgramPipelineID pipeline)
{
    return true;
}

bool ValidateUseProgramStagesBase(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ProgramPipelineID pipeline,
                                  GLbitfield stages,
                                  ShaderProgramID programId)
{
    // GL_INVALID_VALUE is generated if shaders contains set bits that are not recognized, and is
    // not the reserved value GL_ALL_SHADER_BITS.
    GLbitfield knownShaderBits =
        GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT | GL_COMPUTE_SHADER_BIT;

    if (context->getClientVersion() >= ES_3_2 || context->getExtensions().geometryShaderAny())
    {
        knownShaderBits |= GL_GEOMETRY_SHADER_BIT;
    }

    if (context->getClientVersion() >= ES_3_2 || context->getExtensions().tessellationShaderAny())
    {
        knownShaderBits |= GL_TESS_CONTROL_SHADER_BIT;
        knownShaderBits |= GL_TESS_EVALUATION_SHADER_BIT;
    }

    if ((stages & ~knownShaderBits) && (stages != GL_ALL_SHADER_BITS))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kUnrecognizedShaderStageBit);
        return false;
    }

    // GL_INVALID_OPERATION is generated if pipeline is not a name previously returned from a call
    // to glGenProgramPipelines or if such a name has been deleted by a call to
    // glDeleteProgramPipelines.
    if (!context->isProgramPipelineGenerated({pipeline}))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    // If program is zero, or refers to a program object with no valid shader executable for a given
    // stage, it is as if the pipeline object has no programmable stage configured for the indicated
    // shader stages.
    if (programId.value == 0)
    {
        return true;
    }

    Program *program = context->getProgramNoResolveLink(programId);
    if (!program)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kProgramDoesNotExist);
        return false;
    }

    // GL_INVALID_OPERATION is generated if program refers to a program object that was not linked
    // with its GL_PROGRAM_SEPARABLE status set.
    // resolveLink() may not have been called if glCreateShaderProgramv() was not used and
    // glDetachShader() was not called.
    program->resolveLink(context);
    if (!program->isSeparable())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotSeparable);
        return false;
    }

    // GL_INVALID_OPERATION is generated if program refers to a program object that has not been
    // successfully linked.
    if (!program->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    return true;
}

bool ValidateActiveShaderProgramBase(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ProgramPipelineID pipeline,
                                     ShaderProgramID programId)
{
    // An INVALID_OPERATION error is generated if pipeline is not a name returned from a previous
    // call to GenProgramPipelines or if such a name has since been deleted by
    // DeleteProgramPipelines.
    if (!context->isProgramPipelineGenerated({pipeline}))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kObjectNotGenerated);
        return false;
    }

    // An INVALID_VALUE error is generated if program is not zero and is not the name of either a
    // program or shader object.
    if ((programId.value != 0) && !context->isProgram(programId) && !context->isShader(programId))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kProgramDoesNotExist);
        return false;
    }

    // An INVALID_OPERATION error is generated if program is the name of a shader object.
    if (context->isShader(programId))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExpectedProgramName);
        return false;
    }

    // An INVALID_OPERATION error is generated if program is not zero and has not been linked, or
    // was last linked unsuccessfully. The active program is not modified.
    Program *program = context->getProgramNoResolveLink(programId);
    if ((programId.value != 0) && !program->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }

    return true;
}

bool ValidateCreateShaderProgramvBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderType type,
                                      GLsizei count,
                                      const GLchar *const *strings)
{
    switch (type)
    {
        case ShaderType::InvalidEnum:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
            return false;
        case ShaderType::Vertex:
        case ShaderType::Fragment:
        case ShaderType::Compute:
            break;
        case ShaderType::Geometry:
            if (!context->getExtensions().geometryShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
                return false;
            }
            break;
        case ShaderType::TessControl:
        case ShaderType::TessEvaluation:
            if (!context->getExtensions().tessellationShaderAny() &&
                context->getClientVersion() < ES_3_2)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidShaderType);
                return false;
            }
            break;
        default:
            UNREACHABLE();
    }

    // GL_INVALID_VALUE is generated if count is negative.
    if (count < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeCount);
        return false;
    }

    return true;
}

bool ValidateCreateShaderProgramvBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ShaderType type,
                                      GLsizei count,
                                      const GLchar **strings)
{
    const GLchar *const *tmpStrings = strings;
    return ValidateCreateShaderProgramvBase(context, entryPoint, type, count, tmpStrings);
}

bool ValidateGetProgramPipelineivBase(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      ProgramPipelineID pipeline,
                                      GLenum pname,
                                      const GLint *params)
{
    // An INVALID_OPERATION error is generated if pipeline is not a name returned from a previous
    // call to GenProgramPipelines or if such a name has since been deleted by
    // DeleteProgramPipelines.
    if ((pipeline.value == 0) || (!context->isProgramPipelineGenerated(pipeline)))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramPipelineDoesNotExist);
        return false;
    }

    // An INVALID_ENUM error is generated if pname is not ACTIVE_PROGRAM,
    // INFO_LOG_LENGTH, VALIDATE_STATUS, or one of the type arguments in
    // table 7.1.
    switch (pname)
    {
        case GL_ACTIVE_PROGRAM:
        case GL_INFO_LOG_LENGTH:
        case GL_VALIDATE_STATUS:
        case GL_VERTEX_SHADER:
        case GL_FRAGMENT_SHADER:
        case GL_COMPUTE_SHADER:
            break;
        case GL_GEOMETRY_SHADER:
            return context->getExtensions().geometryShaderAny() ||
                   context->getClientVersion() >= ES_3_2;
        case GL_TESS_CONTROL_SHADER:
        case GL_TESS_EVALUATION_SHADER:
            return context->getExtensions().tessellationShaderAny() ||
                   context->getClientVersion() >= ES_3_2;

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kInvalidPname);
            return false;
    }

    return true;
}

bool ValidateValidateProgramPipelineBase(const Context *context,
                                         angle::EntryPoint entryPoint,
                                         ProgramPipelineID pipeline)
{
    if (pipeline.value == 0)
    {
        return false;
    }

    if (!context->isProgramPipelineGenerated(pipeline))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramPipelineDoesNotExist);
        return false;
    }

    return true;
}

bool ValidateGetProgramPipelineInfoLogBase(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           ProgramPipelineID pipeline,
                                           GLsizei bufSize,
                                           const GLsizei *length,
                                           const GLchar *infoLog)
{
    if (bufSize < 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kNegativeBufSize);
        return false;
    }

    if (!context->isProgramPipelineGenerated(pipeline))
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kProgramPipelineDoesNotExist);
        return false;
    }

    return true;
}

bool ValidateActiveShaderProgram(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ProgramPipelineID pipelinePacked,
                                 ShaderProgramID programPacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateActiveShaderProgramBase(context, entryPoint, pipelinePacked, programPacked);
}

bool ValidateBindProgramPipeline(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 ProgramPipelineID pipelinePacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateBindProgramPipelineBase(context, entryPoint, pipelinePacked);
}

bool ValidateCreateShaderProgramv(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ShaderType typePacked,
                                  GLsizei count,
                                  const GLchar *const *strings)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateCreateShaderProgramvBase(context, entryPoint, typePacked, count, strings);
}

bool ValidateDeleteProgramPipelines(const Context *context,
                                    angle::EntryPoint entryPoint,
                                    GLsizei n,
                                    const ProgramPipelineID *pipelinesPacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateDeleteProgramPipelinesBase(context, entryPoint, n, pipelinesPacked);
}

bool ValidateGenProgramPipelines(const Context *context,
                                 angle::EntryPoint entryPoint,
                                 GLsizei n,
                                 const ProgramPipelineID *pipelinesPacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGenProgramPipelinesBase(context, entryPoint, n, pipelinesPacked);
}

bool ValidateGetProgramPipelineInfoLog(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ProgramPipelineID pipelinePacked,
                                       GLsizei bufSize,
                                       const GLsizei *length,
                                       const GLchar *infoLog)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetProgramPipelineInfoLogBase(context, entryPoint, pipelinePacked, bufSize,
                                                 length, infoLog);
}

bool ValidateGetProgramPipelineiv(const Context *context,
                                  angle::EntryPoint entryPoint,
                                  ProgramPipelineID pipelinePacked,
                                  GLenum pname,
                                  const GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateGetProgramPipelineivBase(context, entryPoint, pipelinePacked, pname, params);
}

bool ValidateIsProgramPipeline(const Context *context,
                               angle::EntryPoint entryPoint,
                               ProgramPipelineID pipelinePacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateIsProgramPipelineBase(context, entryPoint, pipelinePacked);
}

bool ValidateProgramUniform1f(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLfloat v0)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1fBase(context, entryPoint, programPacked, locationPacked, v0);
}

bool ValidateProgramUniform1fv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1fvBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform1i(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLint v0)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1iBase(context, entryPoint, programPacked, locationPacked, v0);
}

bool ValidateProgramUniform1iv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1ivBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform1ui(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLuint v0)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1uiBase(context, entryPoint, programPacked, locationPacked, v0);
}

bool ValidateProgramUniform1uiv(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID programPacked,
                                UniformLocation locationPacked,
                                GLsizei count,
                                const GLuint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform1uivBase(context, entryPoint, programPacked, locationPacked, count,
                                          value);
}

bool ValidateProgramUniform2f(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLfloat v0,
                              GLfloat v1)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2fBase(context, entryPoint, programPacked, locationPacked, v0, v1);
}

bool ValidateProgramUniform2fv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2fvBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform2i(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLint v0,
                              GLint v1)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2iBase(context, entryPoint, programPacked, locationPacked, v0, v1);
}

bool ValidateProgramUniform2iv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2ivBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform2ui(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLuint v0,
                               GLuint v1)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2uiBase(context, entryPoint, programPacked, locationPacked, v0,
                                         v1);
}

bool ValidateProgramUniform2uiv(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID programPacked,
                                UniformLocation locationPacked,
                                GLsizei count,
                                const GLuint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform2uivBase(context, entryPoint, programPacked, locationPacked, count,
                                          value);
}

bool ValidateProgramUniform3f(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLfloat v0,
                              GLfloat v1,
                              GLfloat v2)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3fBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                        v2);
}

bool ValidateProgramUniform3fv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3fvBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform3i(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLint v0,
                              GLint v1,
                              GLint v2)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3iBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                        v2);
}

bool ValidateProgramUniform3iv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3ivBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform3ui(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLuint v0,
                               GLuint v1,
                               GLuint v2)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3uiBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                         v2);
}

bool ValidateProgramUniform3uiv(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID programPacked,
                                UniformLocation locationPacked,
                                GLsizei count,
                                const GLuint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform3uivBase(context, entryPoint, programPacked, locationPacked, count,
                                          value);
}

bool ValidateProgramUniform4f(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLfloat v0,
                              GLfloat v1,
                              GLfloat v2,
                              GLfloat v3)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4fBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                        v2, v3);
}

bool ValidateProgramUniform4fv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4fvBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform4i(const Context *context,
                              angle::EntryPoint entryPoint,
                              ShaderProgramID programPacked,
                              UniformLocation locationPacked,
                              GLint v0,
                              GLint v1,
                              GLint v2,
                              GLint v3)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4iBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                        v2, v3);
}

bool ValidateProgramUniform4iv(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLsizei count,
                               const GLint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4ivBase(context, entryPoint, programPacked, locationPacked, count,
                                         value);
}

bool ValidateProgramUniform4ui(const Context *context,
                               angle::EntryPoint entryPoint,
                               ShaderProgramID programPacked,
                               UniformLocation locationPacked,
                               GLuint v0,
                               GLuint v1,
                               GLuint v2,
                               GLuint v3)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4uiBase(context, entryPoint, programPacked, locationPacked, v0, v1,
                                         v2, v3);
}

bool ValidateProgramUniform4uiv(const Context *context,
                                angle::EntryPoint entryPoint,
                                ShaderProgramID programPacked,
                                UniformLocation locationPacked,
                                GLsizei count,
                                const GLuint *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniform4uivBase(context, entryPoint, programPacked, locationPacked, count,
                                          value);
}

bool ValidateProgramUniformMatrix2fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID programPacked,
                                     UniformLocation locationPacked,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix2fvBase(context, entryPoint, programPacked, locationPacked,
                                               count, transpose, value);
}

bool ValidateProgramUniformMatrix2x3fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix2x3fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateProgramUniformMatrix2x4fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix2x4fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateProgramUniformMatrix3fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID programPacked,
                                     UniformLocation locationPacked,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix3fvBase(context, entryPoint, programPacked, locationPacked,
                                               count, transpose, value);
}

bool ValidateProgramUniformMatrix3x2fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix3x2fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateProgramUniformMatrix3x4fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix3x4fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateProgramUniformMatrix4fv(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ShaderProgramID programPacked,
                                     UniformLocation locationPacked,
                                     GLsizei count,
                                     GLboolean transpose,
                                     const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix4fvBase(context, entryPoint, programPacked, locationPacked,
                                               count, transpose, value);
}

bool ValidateProgramUniformMatrix4x2fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix4x2fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateProgramUniformMatrix4x3fv(const Context *context,
                                       angle::EntryPoint entryPoint,
                                       ShaderProgramID programPacked,
                                       UniformLocation locationPacked,
                                       GLsizei count,
                                       GLboolean transpose,
                                       const GLfloat *value)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateProgramUniformMatrix4x3fvBase(context, entryPoint, programPacked, locationPacked,
                                                 count, transpose, value);
}

bool ValidateUseProgramStages(const Context *context,
                              angle::EntryPoint entryPoint,
                              ProgramPipelineID pipelinePacked,
                              GLbitfield stages,
                              ShaderProgramID programPacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateUseProgramStagesBase(context, entryPoint, pipelinePacked, stages, programPacked);
}

bool ValidateValidateProgramPipeline(const Context *context,
                                     angle::EntryPoint entryPoint,
                                     ProgramPipelineID pipelinePacked)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateValidateProgramPipelineBase(context, entryPoint, pipelinePacked);
}

bool ValidateMemoryBarrier(const Context *context,
                           angle::EntryPoint entryPoint,
                           GLbitfield barriers)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    if (barriers == GL_ALL_BARRIER_BITS)
    {
        return true;
    }

    GLbitfield supported_barrier_bits =
        GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT |
        GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_COMMAND_BARRIER_BIT |
        GL_PIXEL_BUFFER_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT |
        GL_FRAMEBUFFER_BARRIER_BIT | GL_TRANSFORM_FEEDBACK_BARRIER_BIT |
        GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;

    if (context->getExtensions().bufferStorageEXT)
    {
        supported_barrier_bits |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT;
    }

    if (barriers == 0 || (barriers & ~supported_barrier_bits) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMemoryBarrierBit);
        return false;
    }

    return true;
}

bool ValidateMemoryBarrierByRegion(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLbitfield barriers)
{
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    if (barriers == GL_ALL_BARRIER_BITS)
    {
        return true;
    }

    GLbitfield supported_barrier_bits = GL_ATOMIC_COUNTER_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT |
                                        GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                                        GL_SHADER_STORAGE_BARRIER_BIT |
                                        GL_TEXTURE_FETCH_BARRIER_BIT | GL_UNIFORM_BARRIER_BIT;
    if (barriers == 0 || (barriers & ~supported_barrier_bits) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMemoryBarrierBit);
        return false;
    }

    return true;
}

bool ValidateSampleMaski(const PrivateState &state,
                         ErrorSet *errors,
                         angle::EntryPoint entryPoint,
                         GLuint maskNumber,
                         GLbitfield mask)
{
    if (state.getClientVersion() < ES_3_1)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    return ValidateSampleMaskiBase(state, errors, entryPoint, maskNumber, mask);
}

bool ValidateMinSampleShadingOES(const PrivateState &state,
                                 ErrorSet *errors,
                                 angle::EntryPoint entryPoint,
                                 GLfloat value)
{
    if (!state.getExtensions().sampleShadingOES)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    return true;
}

bool ValidateFramebufferTextureCommon(const Context *context,
                                      angle::EntryPoint entryPoint,
                                      GLenum target,
                                      GLenum attachment,
                                      TextureID texture,
                                      GLint level)
{
    if (texture.value != 0)
    {
        Texture *tex = context->getTexture(texture);

        // [EXT_geometry_shader] Section 9.2.8 "Attaching Texture Images to a Framebuffer"
        // An INVALID_VALUE error is generated if <texture> is not the name of a texture object.
        // We put this validation before ValidateFramebufferTextureBase because it is an
        // INVALID_OPERATION error for both FramebufferTexture2D and FramebufferTextureLayer:
        // [OpenGL ES 3.1] Chapter 9.2.8 (FramebufferTexture2D)
        // An INVALID_OPERATION error is generated if texture is not zero, and does not name an
        // existing texture object of type matching textarget.
        // [OpenGL ES 3.1 Chapter 9.2.8 (FramebufferTextureLayer)
        // An INVALID_OPERATION error is generated if texture is non-zero and is not the name of a
        // three-dimensional or two-dimensional array texture.
        if (tex == nullptr)
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidTextureName);
            return false;
        }

        if (!ValidMipLevel(context, tex->getType(), level))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kInvalidMipLevel);
            return false;
        }

        // GLES spec 3.2, Section 9.2.8 "Attaching Texture Images to a Framebuffer"
        // * If textarget is TEXTURE_2D_MULTISAMPLE, then level must be zero.
        // * If texture is a two-dimensional multisample array texture, then level must be zero.
        // Already validated in ValidMipLevel.
        ASSERT(level == 0 || !IsMultisampled(tex->getType()));
    }

    if (!ValidateFramebufferTextureBase(context, entryPoint, target, attachment, texture, level))
    {
        return false;
    }

    return true;
}

bool ValidateFramebufferTextureEXT(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLenum attachment,
                                   TextureID texture,
                                   GLint level)
{
    if (!context->getExtensions().geometryShaderEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGeometryShaderExtensionNotEnabled);
        return false;
    }

    return ValidateFramebufferTextureCommon(context, entryPoint, target, attachment, texture,
                                            level);
}

bool ValidateFramebufferTextureOES(const Context *context,
                                   angle::EntryPoint entryPoint,
                                   GLenum target,
                                   GLenum attachment,
                                   TextureID texture,
                                   GLint level)
{
    if (!context->getExtensions().geometryShaderOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kGeometryShaderExtensionNotEnabled);
        return false;
    }

    return ValidateFramebufferTextureCommon(context, entryPoint, target, attachment, texture,
                                            level);
}

bool ValidateTexStorageMem3DMultisampleEXT(const Context *context,
                                           angle::EntryPoint entryPoint,
                                           TextureType target,
                                           GLsizei samples,
                                           GLenum internalFormat,
                                           GLsizei width,
                                           GLsizei height,
                                           GLsizei depth,
                                           GLboolean fixedSampleLocations,
                                           MemoryObjectID memory,
                                           GLuint64 offset)
{
    if (!context->getExtensions().memoryObjectEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }

    UNIMPLEMENTED();
    return false;
}

bool ValidateGetProgramResourceLocationIndexEXT(const Context *context,
                                                angle::EntryPoint entryPoint,
                                                ShaderProgramID program,
                                                GLenum programInterface,
                                                const char *name)
{
    if (!context->getExtensions().blendFuncExtendedEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kExtensionNotEnabled);
        return false;
    }
    if (context->getClientVersion() < ES_3_1)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kES31Required);
        return false;
    }
    if (programInterface != GL_PROGRAM_OUTPUT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kProgramInterfaceMustBeProgramOutput);
        return false;
    }
    Program *programObject = GetValidProgram(context, entryPoint, program);
    if (!programObject)
    {
        return false;
    }
    if (!programObject->isLinked())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kProgramNotLinked);
        return false;
    }
    return true;
}

// GL_OES_texture_buffer
bool ValidateTexBufferOES(const Context *context,
                          angle::EntryPoint entryPoint,
                          TextureType target,
                          GLenum internalformat,
                          BufferID bufferPacked)
{
    if (!context->getExtensions().textureBufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureBufferExtensionNotAvailable);
        return false;
    }

    return ValidateTexBufferBase(context, entryPoint, target, internalformat, bufferPacked);
}

bool ValidateTexBufferRangeOES(const Context *context,
                               angle::EntryPoint entryPoint,
                               TextureType target,
                               GLenum internalformat,
                               BufferID bufferPacked,
                               GLintptr offset,
                               GLsizeiptr size)
{
    if (!context->getExtensions().textureBufferOES)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureBufferExtensionNotAvailable);
        return false;
    }

    return ValidateTexBufferRangeBase(context, entryPoint, target, internalformat, bufferPacked,
                                      offset, size);
}

// GL_EXT_texture_buffer
bool ValidateTexBufferEXT(const Context *context,
                          angle::EntryPoint entryPoint,
                          TextureType target,
                          GLenum internalformat,
                          BufferID bufferPacked)
{
    if (!context->getExtensions().textureBufferEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureBufferExtensionNotAvailable);
        return false;
    }

    return ValidateTexBufferBase(context, entryPoint, target, internalformat, bufferPacked);
}

bool ValidateTexBufferRangeEXT(const Context *context,
                               angle::EntryPoint entryPoint,
                               TextureType target,
                               GLenum internalformat,
                               BufferID bufferPacked,
                               GLintptr offset,
                               GLsizeiptr size)
{
    if (!context->getExtensions().textureBufferEXT)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureBufferExtensionNotAvailable);
        return false;
    }

    return ValidateTexBufferRangeBase(context, entryPoint, target, internalformat, bufferPacked,
                                      offset, size);
}

bool ValidateTexBufferBase(const Context *context,
                           angle::EntryPoint entryPoint,
                           TextureType target,
                           GLenum internalformat,
                           BufferID bufferPacked)
{
    if (target != TextureType::Buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferTarget);
        return false;
    }

    switch (internalformat)
    {
        case GL_R8:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R16I:
        case GL_R32I:
        case GL_R8UI:
        case GL_R16UI:
        case GL_R32UI:
        case GL_RG8:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG16I:
        case GL_RG32I:
        case GL_RG8UI:
        case GL_RG16UI:
        case GL_RG32UI:
        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA16I:
        case GL_RGBA32I:
        case GL_RGBA8UI:
        case GL_RGBA16UI:
        case GL_RGBA32UI:
            break;
        case GL_R16_EXT:
        case GL_RG16_EXT:
        case GL_RGBA16_EXT:
        {
            if (!context->getExtensions().textureNorm16EXT)
            {
                ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferInternalFormat);
                return false;
            }
            break;
        }

        default:
            ANGLE_VALIDATION_ERROR(GL_INVALID_ENUM, kTextureBufferInternalFormat);
            return false;
    }

    if (bufferPacked.value != 0)
    {
        if (!context->isBufferGenerated(bufferPacked))
        {
            ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kTextureBufferInvalidBuffer);
            return false;
        }
    }

    return true;
}

bool ValidateTexBufferRangeBase(const Context *context,
                                angle::EntryPoint entryPoint,
                                TextureType target,
                                GLenum internalformat,
                                BufferID bufferPacked,
                                GLintptr offset,
                                GLsizeiptr size)
{
    const Caps &caps = context->getCaps();

    if (offset < 0 || (offset % caps.textureBufferOffsetAlignment) != 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureBufferOffsetAlignment);
        return false;
    }
    if (size <= 0)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureBufferSize);
        return false;
    }
    const Buffer *buffer = context->getBuffer(bufferPacked);

    if (!buffer)
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_OPERATION, kBufferNotBound);
        return false;
    }

    if (offset + size > buffer->getSize())
    {
        ANGLE_VALIDATION_ERROR(GL_INVALID_VALUE, kTextureBufferSizeOffset);
        return false;
    }

    return ValidateTexBufferBase(context, entryPoint, target, internalformat, bufferPacked);
}

bool ValidatePatchParameteriBase(const PrivateState &state,
                                 ErrorSet *errors,
                                 angle::EntryPoint entryPoint,
                                 GLenum pname,
                                 GLint value)
{
    if (state.getClientVersion() < ES_3_1)
    {
        errors->validationError(entryPoint, GL_INVALID_OPERATION, kES31Required);
        return false;
    }

    if (pname != GL_PATCH_VERTICES)
    {
        errors->validationError(entryPoint, GL_INVALID_ENUM, kInvalidPname);
        return false;
    }

    if (value <= 0)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidValueNonPositive);
        return false;
    }

    if (value > state.getCaps().maxPatchVertices)
    {
        errors->validationError(entryPoint, GL_INVALID_VALUE, kInvalidValueExceedsMaxPatchSize);
        return false;
    }

    return true;
}

}  // namespace gl
