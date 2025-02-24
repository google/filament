//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.cpp:
//   ANGLE Frame capture common classes.
//

#include "common/frame_capture_utils.h"

namespace angle
{
namespace
{
// Keep the simplest nullptr string for easy C parsing.
constexpr char kNullPointerString[] = "0";
}  // anonymous namespace

ParamCapture::ParamCapture() : type(ParamType::TGLenum), enumGroup(gl::GLESEnum::AllEnums) {}

ParamCapture::ParamCapture(const char *nameIn, ParamType typeIn)
    : name(nameIn),
      type(typeIn),
      enumGroup(gl::GLESEnum::AllEnums),
      bigGLEnum(gl::BigGLEnum::AllEnums)
{}

ParamCapture::~ParamCapture() = default;

ParamCapture::ParamCapture(ParamCapture &&other)
    : type(ParamType::TGLenum),
      enumGroup(gl::GLESEnum::AllEnums),
      bigGLEnum(gl::BigGLEnum::AllEnums)
{
    *this = std::move(other);
}

ParamCapture &ParamCapture::operator=(ParamCapture &&other)
{
    std::swap(name, other.name);
    std::swap(type, other.type);
    std::swap(value, other.value);
    std::swap(enumGroup, other.enumGroup);
    std::swap(bigGLEnum, other.bigGLEnum);
    std::swap(data, other.data);
    std::swap(arrayClientPointerIndex, other.arrayClientPointerIndex);
    std::swap(readBufferSizeBytes, other.readBufferSizeBytes);
    std::swap(dataNElements, other.dataNElements);
    return *this;
}

ParamBuffer::ParamBuffer() {}

ParamBuffer::~ParamBuffer() = default;

ParamBuffer::ParamBuffer(ParamBuffer &&other)
{
    *this = std::move(other);
}

ParamBuffer &ParamBuffer::operator=(ParamBuffer &&other)
{
    std::swap(mParamCaptures, other.mParamCaptures);
    std::swap(mClientArrayDataParam, other.mClientArrayDataParam);
    std::swap(mReadBufferSize, other.mReadBufferSize);
    std::swap(mReturnValueCapture, other.mReturnValueCapture);
    return *this;
}

ParamCapture &ParamBuffer::getParam(const char *paramName, ParamType paramType, int index)
{
    ParamCapture &capture = mParamCaptures[index];
    ASSERT(capture.name == paramName);
    ASSERT(capture.type == paramType);
    return capture;
}

const ParamCapture &ParamBuffer::getParam(const char *paramName,
                                          ParamType paramType,
                                          int index) const
{
    return const_cast<ParamBuffer *>(this)->getParam(paramName, paramType, index);
}

ParamCapture &ParamBuffer::getParamFlexName(const char *paramName1,
                                            const char *paramName2,
                                            ParamType paramType,
                                            int index)
{
    ParamCapture &capture = mParamCaptures[index];
    ASSERT(capture.name == paramName1 || capture.name == paramName2);
    ASSERT(capture.type == paramType);
    return capture;
}

const ParamCapture &ParamBuffer::getParamFlexName(const char *paramName1,
                                                  const char *paramName2,
                                                  ParamType paramType,
                                                  int index) const
{
    return const_cast<ParamBuffer *>(this)->getParamFlexName(paramName1, paramName2, paramType,
                                                             index);
}

void ParamBuffer::addParam(ParamCapture &&param)
{
    if (param.arrayClientPointerIndex != -1)
    {
        ASSERT(mClientArrayDataParam == -1);
        mClientArrayDataParam = static_cast<int>(mParamCaptures.size());
    }

    mReadBufferSize = std::max(param.readBufferSizeBytes, mReadBufferSize);
    mParamCaptures.emplace_back(std::move(param));
}

void ParamBuffer::addReturnValue(ParamCapture &&returnValue)
{
    mReturnValueCapture = std::move(returnValue);
}

const char *ParamBuffer::getNextParamName()
{
    static const char *kParamNames[] = {"p0",  "p1",  "p2",  "p3",  "p4",  "p5",  "p6",  "p7",
                                        "p8",  "p9",  "p10", "p11", "p12", "p13", "p14", "p15",
                                        "p16", "p17", "p18", "p19", "p20", "p21", "p22"};
    ASSERT(mParamCaptures.size() < ArraySize(kParamNames));
    return kParamNames[mParamCaptures.size()];
}

ParamCapture &ParamBuffer::getClientArrayPointerParameter()
{
    ASSERT(hasClientArrayData());
    return mParamCaptures[mClientArrayDataParam];
}

CallCapture::CallCapture(EntryPoint entryPointIn, ParamBuffer &&paramsIn)
    : entryPoint(entryPointIn), params(std::move(paramsIn))
{}

CallCapture::CallCapture(const std::string &customFunctionNameIn, ParamBuffer &&paramsIn)
    : entryPoint(EntryPoint::Invalid),
      customFunctionName(customFunctionNameIn),
      params(std::move(paramsIn))
{}

CallCapture::~CallCapture() = default;

CallCapture::CallCapture(CallCapture &&other)
{
    *this = std::move(other);
}

CallCapture &CallCapture::operator=(CallCapture &&other)
{
    std::swap(entryPoint, other.entryPoint);
    std::swap(customFunctionName, other.customFunctionName);
    std::swap(params, other.params);
    std::swap(isActive, other.isActive);
    std::swap(contextID, other.contextID);
    std::swap(isSyncPoint, other.isSyncPoint);
    return *this;
}

const char *CallCapture::name() const
{
    if (customFunctionName.empty())
    {
        ASSERT(entryPoint != EntryPoint::Invalid);
        return angle::GetEntryPointName(entryPoint);
    }
    else
    {
        return customFunctionName.c_str();
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLboolean>(std::ostream &os,
                                                  const CallCapture &call,
                                                  GLboolean value)
{
    switch (value)
    {
        case GL_TRUE:
            os << "GL_TRUE";
            break;
        case GL_FALSE:
            os << "GL_FALSE";
            break;
        default:
            os << "0x" << std::hex << std::uppercase << GLint(value);
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLbooleanPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         GLboolean *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(GLboolean *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TvoidConstPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         const void *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const void *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TvoidPointer>(std::ostream &os,
                                                    const CallCapture &call,
                                                    void *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(void *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLfloatConstPointer>(std::ostream &os,
                                                            const CallCapture &call,
                                                            const GLfloat *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const GLfloat *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLintConstPointer>(std::ostream &os,
                                                          const CallCapture &call,
                                                          const GLint *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const GLint *)" << static_cast<int>(reinterpret_cast<intptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLsizeiPointer>(std::ostream &os,
                                                       const CallCapture &call,
                                                       GLsizei *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(GLsizei *)" << static_cast<int>(reinterpret_cast<intptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLuintPointer>(std::ostream &os,
                                                      const CallCapture &call,
                                                      GLuint *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(GLuint *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLuintConstPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           const GLuint *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const GLuint *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TGLDEBUGPROCKHR>(std::ostream &os,
                                                       const CallCapture &call,
                                                       GLDEBUGPROCKHR value)
{}

template <>
void WriteParamValueReplay<ParamType::TGLDEBUGPROC>(std::ostream &os,
                                                    const CallCapture &call,
                                                    GLDEBUGPROC value)
{}

template <>
void WriteParamValueReplay<ParamType::TBufferID>(std::ostream &os,
                                                 const CallCapture &call,
                                                 gl::BufferID value)
{
    os << "gBufferMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TFenceNVID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::FenceNVID value)
{
    os << "gFenceNVMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TFramebufferID>(std::ostream &os,
                                                      const CallCapture &call,
                                                      gl::FramebufferID value)
{
    os << "gFramebufferMapPerContext[" << call.contextID.value << "][" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TMemoryObjectID>(std::ostream &os,
                                                       const CallCapture &call,
                                                       gl::MemoryObjectID value)
{
    os << "gMemoryObjectMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TProgramPipelineID>(std::ostream &os,
                                                          const CallCapture &call,
                                                          gl::ProgramPipelineID value)
{
    os << "gProgramPipelineMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TQueryID>(std::ostream &os,
                                                const CallCapture &call,
                                                gl::QueryID value)
{
    os << "gQueryMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TRenderbufferID>(std::ostream &os,
                                                       const CallCapture &call,
                                                       gl::RenderbufferID value)
{
    os << "gRenderbufferMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TSamplerID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::SamplerID value)
{
    os << "gSamplerMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TSemaphoreID>(std::ostream &os,
                                                    const CallCapture &call,
                                                    gl::SemaphoreID value)
{
    os << "gSemaphoreMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TShaderProgramID>(std::ostream &os,
                                                        const CallCapture &call,
                                                        gl::ShaderProgramID value)
{
    os << "gShaderProgramMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TSyncID>(std::ostream &os,
                                               const CallCapture &call,
                                               gl::SyncID value)
{
    os << "gSyncMap2[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TTextureID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::TextureID value)
{
    os << "gTextureMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TTransformFeedbackID>(std::ostream &os,
                                                            const CallCapture &call,
                                                            gl::TransformFeedbackID value)
{
    os << "gTransformFeedbackMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TVertexArrayID>(std::ostream &os,
                                                      const CallCapture &call,
                                                      gl::VertexArrayID value)
{
    os << "gVertexArrayMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TUniformLocation>(std::ostream &os,
                                                        const CallCapture &call,
                                                        gl::UniformLocation value)
{
    if (value.value == -1)
    {
        os << "-1";
        return;
    }

    os << "gUniformLocations[";

    // Find the program from the call parameters.
    std::vector<gl::ShaderProgramID> shaderProgramIDs;
    if (FindResourceIDsInCall<gl::ShaderProgramID>(call, shaderProgramIDs))
    {
        ASSERT(shaderProgramIDs.size() == 1);
        os << shaderProgramIDs[0].value;
    }
    else
    {
        os << "gCurrentProgram";
    }

    os << "][" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TUniformBlockIndex>(std::ostream &os,
                                                          const CallCapture &call,
                                                          gl::UniformBlockIndex value)
{
    // We do not support directly using uniform block indexes due to their multiple indirections.
    // Use CaptureCustomUniformBlockBinding if you end up hitting this assertion.
    UNREACHABLE();
}

template <>
void WriteParamValueReplay<ParamType::TGLubyte>(std::ostream &os,
                                                const CallCapture &call,
                                                GLubyte value)
{
    const int v = value;
    os << v;
}

template <>
void WriteParamValueReplay<ParamType::TEGLDEBUGPROCKHR>(std::ostream &os,
                                                        const CallCapture &call,
                                                        EGLDEBUGPROCKHR value)
{
    // It's not necessary to implement correct capture for these types.
    os << "0";
}

template <>
void WriteParamValueReplay<ParamType::TEGLGetBlobFuncANDROID>(std::ostream &os,
                                                              const CallCapture &call,
                                                              EGLGetBlobFuncANDROID value)
{
    // It's not necessary to implement correct capture for these types.
    os << "0";
}

template <>
void WriteParamValueReplay<ParamType::TEGLSetBlobFuncANDROID>(std::ostream &os,
                                                              const CallCapture &call,
                                                              EGLSetBlobFuncANDROID value)
{
    // It's not necessary to implement correct capture for these types.
    os << "0";
}

template <>
void WriteParamValueReplay<ParamType::Tegl_ConfigPointer>(std::ostream &os,
                                                          const CallCapture &call,
                                                          egl::Config *value)
{
    os << "EGL_NO_CONFIG_KHR";
}

template <>
void WriteParamValueReplay<ParamType::TSurfaceID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  egl::SurfaceID value)
{
    os << "gSurfaceMap2[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TContextID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::ContextID value)
{
    os << "gContextMap2[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::Tegl_DisplayPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           egl::Display *value)
{
    os << "gEGLDisplay";
}

template <>
void WriteParamValueReplay<ParamType::TImageID>(std::ostream &os,
                                                const CallCapture &call,
                                                egl::ImageID value)
{
    os << "gEGLImageMap2[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TEGLClientBuffer>(std::ostream &os,
                                                        const CallCapture &call,
                                                        EGLClientBuffer value)
{
    os << value;
}

template <>
void WriteParamValueReplay<ParamType::Tegl_SyncID>(std::ostream &os,
                                                   const CallCapture &call,
                                                   egl::SyncID value)
{
    os << "gEGLSyncMap[" << value.value << "]";
}

template <>
void WriteParamValueReplay<ParamType::TEGLAttribPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         EGLAttrib *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(EGLAttrib *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TEGLAttribConstPointer>(std::ostream &os,
                                                              const CallCapture &call,
                                                              const EGLAttrib *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const EGLAttrib *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TEGLintConstPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           const EGLint *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const EGLint *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TEGLintPointer>(std::ostream &os,
                                                      const CallCapture &call,
                                                      EGLint *value)
{
    if (value == 0)
    {
        os << kNullPointerString;
    }
    else
    {
        os << "(const EGLint *)" << static_cast<int>(reinterpret_cast<uintptr_t>(value));
    }
}

template <>
void WriteParamValueReplay<ParamType::TEGLTime>(std::ostream &os,
                                                const CallCapture &call,
                                                EGLTime value)
{
    os << value << "ul";
}

template <>
void WriteParamValueReplay<ParamType::TEGLTimeKHR>(std::ostream &os,
                                                   const CallCapture &call,
                                                   EGLTimeKHR value)
{
    os << value << "ul";
}

template <>
void WriteParamValueReplay<ParamType::TGLGETBLOBPROCANGLE>(std::ostream &os,
                                                           const CallCapture &call,
                                                           GLGETBLOBPROCANGLE value)
{
    // It's not necessary to implement correct capture for these types.
    os << "0";
}

template <>
void WriteParamValueReplay<ParamType::TGLSETBLOBPROCANGLE>(std::ostream &os,
                                                           const CallCapture &call,
                                                           GLSETBLOBPROCANGLE value)
{
    // It's not necessary to implement correct capture for these types.
    os << "0";
}

template <typename ParamValueType>
bool FindResourceIDsInCall(const CallCapture &call, std::vector<ParamValueType> &idsOut)
{
    const ParamType paramType = ParamValueTrait<ParamValueType>::typeID;
    for (const ParamCapture &param : call.params.getParamCaptures())
    {
        if (param.type == paramType)
        {
            const ParamValueType id = AccessParamValue<ParamValueType>(paramType, param.value);
            idsOut.push_back(id);
        }
    }

    return !idsOut.empty();
}

// Explicit instantiation
template bool FindResourceIDsInCall<gl::TextureID>(const CallCapture &call,
                                                   std::vector<gl::TextureID> &idsOut);
template bool FindResourceIDsInCall<gl::ShaderProgramID>(const CallCapture &call,
                                                         std::vector<gl::ShaderProgramID> &idsOut);
}  // namespace angle
