//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.h:
//   ANGLE Frame capture common classes.
//

#ifndef COMMON_FRAME_CAPTURE_UTILS_H_
#define COMMON_FRAME_CAPTURE_UTILS_H_

#include "common/frame_capture_utils_autogen.h"
#include "common/gl_enum_utils_autogen.h"

namespace angle
{
namespace
{
template <typename ParamValueType>
struct ParamValueTrait
{
    static_assert(sizeof(ParamValueType) == 0, "invalid ParamValueType");
};

template <>
struct ParamValueTrait<gl::FramebufferID>
{
    static constexpr const char *name = "framebufferPacked";
    static const ParamType typeID     = ParamType::TFramebufferID;
};

template <>
struct ParamValueTrait<gl::BufferID>
{
    static constexpr const char *name = "bufferPacked";
    static const ParamType typeID     = ParamType::TBufferID;
};

template <>
struct ParamValueTrait<gl::RenderbufferID>
{
    static constexpr const char *name = "renderbufferPacked";
    static const ParamType typeID     = ParamType::TRenderbufferID;
};

template <>
struct ParamValueTrait<gl::TextureID>
{
    static constexpr const char *name = "texturePacked";
    static const ParamType typeID     = ParamType::TTextureID;
};

template <>
struct ParamValueTrait<gl::ShaderProgramID>
{
    static constexpr const char *name = "programPacked";
    static const ParamType typeID     = ParamType::TShaderProgramID;
};
}  // namespace

using ParamData = std::vector<std::vector<uint8_t>>;
struct ParamCapture : angle::NonCopyable
{
    ParamCapture();
    ParamCapture(const char *nameIn, ParamType typeIn);
    ~ParamCapture();

    ParamCapture(ParamCapture &&other);
    ParamCapture &operator=(ParamCapture &&other);

    std::string name;
    ParamType type;
    ParamValue value;
    gl::GLESEnum enumGroup;   // only used for param type GLenum, GLboolean and GLbitfield
    gl::BigGLEnum bigGLEnum;  // only used for param type GLenum, GLboolean and GLbitfield
    ParamData data;
    int dataNElements           = 0;
    int arrayClientPointerIndex = -1;
    size_t readBufferSizeBytes  = 0;
};

using Captures = std::vector<ParamCapture>;

class ParamBuffer final : angle::NonCopyable
{
  public:
    ParamBuffer();
    ~ParamBuffer();

    ParamBuffer(ParamBuffer &&other);
    ParamBuffer &operator=(ParamBuffer &&other);

    template <typename T>
    void addValueParam(const char *paramName, ParamType paramType, T paramValue);
    template <typename T>
    void setValueParamAtIndex(const char *paramName, ParamType paramType, T paramValue, int index);
    template <typename T>
    void addEnumParam(const char *paramName,
                      gl::GLESEnum enumGroup,
                      ParamType paramType,
                      T paramValue);
    template <typename T>
    void addEnumParam(const char *paramName,
                      gl::BigGLEnum enumGroup,
                      ParamType paramType,
                      T paramValue);

    template <typename T>
    void addUnnamedParam(ParamType paramType, T paramValue);

    ParamCapture &getParam(const char *paramName, ParamType paramType, int index);
    const ParamCapture &getParam(const char *paramName, ParamType paramType, int index) const;
    ParamCapture &getParamFlexName(const char *paramName1,
                                   const char *paramName2,
                                   ParamType paramType,
                                   int index);
    const ParamCapture &getParamFlexName(const char *paramName1,
                                         const char *paramName2,
                                         ParamType paramType,
                                         int index) const;
    const ParamCapture &getReturnValue() const { return mReturnValueCapture; }

    void addParam(ParamCapture &&param);
    void addReturnValue(ParamCapture &&returnValue);
    bool hasClientArrayData() const { return mClientArrayDataParam != -1; }
    ParamCapture &getClientArrayPointerParameter();
    size_t getReadBufferSize() const { return mReadBufferSize; }

    bool empty() const { return mParamCaptures.empty(); }
    const std::vector<ParamCapture> &getParamCaptures() const { return mParamCaptures; }

    const char *getNextParamName();

  private:
    std::vector<ParamCapture> mParamCaptures;
    ParamCapture mReturnValueCapture;
    int mClientArrayDataParam = -1;
    size_t mReadBufferSize    = 0;
};

struct CallCapture
{
    CallCapture(EntryPoint entryPointIn, ParamBuffer &&paramsIn);
    CallCapture(const std::string &customFunctionNameIn, ParamBuffer &&paramsIn);
    ~CallCapture();

    CallCapture(CallCapture &&other);
    CallCapture &operator=(CallCapture &&other);

    const char *name() const;

    EntryPoint entryPoint;
    std::string customFunctionName;
    ParamBuffer params;
    bool isActive = true;
    gl::ContextID contextID;
    bool isSyncPoint = false;
};

template <typename T>
void ParamBuffer::addValueParam(const char *paramName, ParamType paramType, T paramValue)
{
    ParamCapture capture(paramName, paramType);
    InitParamValue(paramType, paramValue, &capture.value);
    mParamCaptures.emplace_back(std::move(capture));
}

template <typename T>
void ParamBuffer::setValueParamAtIndex(const char *paramName,
                                       ParamType paramType,
                                       T paramValue,
                                       int index)
{
    ASSERT(mParamCaptures.size() > static_cast<size_t>(index));

    ParamCapture capture(paramName, paramType);
    InitParamValue(paramType, paramValue, &capture.value);
    mParamCaptures[index] = std::move(capture);
}

template <typename T>
void ParamBuffer::addEnumParam(const char *paramName,
                               gl::GLESEnum enumGroup,
                               ParamType paramType,
                               T paramValue)
{
    ParamCapture capture(paramName, paramType);
    InitParamValue(paramType, paramValue, &capture.value);
    capture.enumGroup = enumGroup;
    mParamCaptures.emplace_back(std::move(capture));
}

template <typename T>
void ParamBuffer::addEnumParam(const char *paramName,
                               gl::BigGLEnum enumGroup,
                               ParamType paramType,
                               T paramValue)
{
    ParamCapture capture(paramName, paramType);
    InitParamValue(paramType, paramValue, &capture.value);
    capture.bigGLEnum = enumGroup;
    mParamCaptures.emplace_back(std::move(capture));
}

template <typename T>
void ParamBuffer::addUnnamedParam(ParamType paramType, T paramValue)
{
    addValueParam(getNextParamName(), paramType, paramValue);
}

template <ParamType ParamT, typename T>
void WriteParamValueReplay(std::ostream &os, const CallCapture &call, T value);

template <>
void WriteParamValueReplay<ParamType::TGLboolean>(std::ostream &os,
                                                  const CallCapture &call,
                                                  GLboolean value);

template <>
void WriteParamValueReplay<ParamType::TGLbooleanPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         GLboolean *value);

template <>
void WriteParamValueReplay<ParamType::TvoidConstPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         const void *value);

template <>
void WriteParamValueReplay<ParamType::TvoidPointer>(std::ostream &os,
                                                    const CallCapture &call,
                                                    void *value);

template <>
void WriteParamValueReplay<ParamType::TGLfloatConstPointer>(std::ostream &os,
                                                            const CallCapture &call,
                                                            const GLfloat *value);

template <>
void WriteParamValueReplay<ParamType::TGLintConstPointer>(std::ostream &os,
                                                          const CallCapture &call,
                                                          const GLint *value);

template <>
void WriteParamValueReplay<ParamType::TGLsizeiPointer>(std::ostream &os,
                                                       const CallCapture &call,
                                                       GLsizei *value);
template <>
void WriteParamValueReplay<ParamType::TGLuintPointer>(std::ostream &os,
                                                      const CallCapture &call,
                                                      GLuint *value);
template <>
void WriteParamValueReplay<ParamType::TGLuintConstPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           const GLuint *value);

template <>
void WriteParamValueReplay<ParamType::TGLDEBUGPROCKHR>(std::ostream &os,
                                                       const CallCapture &call,
                                                       GLDEBUGPROCKHR value);

template <>
void WriteParamValueReplay<ParamType::TGLDEBUGPROC>(std::ostream &os,
                                                    const CallCapture &call,
                                                    GLDEBUGPROC value);

template <>
void WriteParamValueReplay<ParamType::TBufferID>(std::ostream &os,
                                                 const CallCapture &call,
                                                 gl::BufferID value);

template <>
void WriteParamValueReplay<ParamType::TFenceNVID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::FenceNVID value);

template <>
void WriteParamValueReplay<ParamType::TFramebufferID>(std::ostream &os,
                                                      const CallCapture &call,
                                                      gl::FramebufferID value);

template <>
void WriteParamValueReplay<ParamType::TMemoryObjectID>(std::ostream &os,
                                                       const CallCapture &call,
                                                       gl::MemoryObjectID value);

template <>
void WriteParamValueReplay<ParamType::TProgramPipelineID>(std::ostream &os,
                                                          const CallCapture &call,
                                                          gl::ProgramPipelineID value);

template <>
void WriteParamValueReplay<ParamType::TQueryID>(std::ostream &os,
                                                const CallCapture &call,
                                                gl::QueryID value);

template <>
void WriteParamValueReplay<ParamType::TRenderbufferID>(std::ostream &os,
                                                       const CallCapture &call,
                                                       gl::RenderbufferID value);

template <>
void WriteParamValueReplay<ParamType::TSamplerID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::SamplerID value);

template <>
void WriteParamValueReplay<ParamType::TSemaphoreID>(std::ostream &os,
                                                    const CallCapture &call,
                                                    gl::SemaphoreID value);

template <>
void WriteParamValueReplay<ParamType::TShaderProgramID>(std::ostream &os,
                                                        const CallCapture &call,
                                                        gl::ShaderProgramID value);

template <>
void WriteParamValueReplay<ParamType::TTextureID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::TextureID value);

template <>
void WriteParamValueReplay<ParamType::TTransformFeedbackID>(std::ostream &os,
                                                            const CallCapture &call,
                                                            gl::TransformFeedbackID value);

template <>
void WriteParamValueReplay<ParamType::TVertexArrayID>(std::ostream &os,
                                                      const CallCapture &call,
                                                      gl::VertexArrayID value);

template <>
void WriteParamValueReplay<ParamType::TUniformLocation>(std::ostream &os,
                                                        const CallCapture &call,
                                                        gl::UniformLocation value);

template <>
void WriteParamValueReplay<ParamType::TUniformBlockIndex>(std::ostream &os,
                                                          const CallCapture &call,
                                                          gl::UniformBlockIndex value);

template <>
void WriteParamValueReplay<ParamType::TSyncID>(std::ostream &os,
                                               const CallCapture &call,
                                               gl::SyncID value);

template <>
void WriteParamValueReplay<ParamType::TGLubyte>(std::ostream &os,
                                                const CallCapture &call,
                                                GLubyte value);

template <>
void WriteParamValueReplay<ParamType::TContextID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  gl::ContextID value);

template <>
void WriteParamValueReplay<ParamType::Tegl_DisplayPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           egl::Display *value);

template <>
void WriteParamValueReplay<ParamType::TImageID>(std::ostream &os,
                                                const CallCapture &call,
                                                egl::ImageID value);

template <>
void WriteParamValueReplay<ParamType::TSurfaceID>(std::ostream &os,
                                                  const CallCapture &call,
                                                  egl::SurfaceID value);

template <>
void WriteParamValueReplay<ParamType::TEGLDEBUGPROCKHR>(std::ostream &os,
                                                        const CallCapture &call,
                                                        EGLDEBUGPROCKHR value);

template <>
void WriteParamValueReplay<ParamType::TEGLGetBlobFuncANDROID>(std::ostream &os,
                                                              const CallCapture &call,
                                                              EGLGetBlobFuncANDROID value);

template <>
void WriteParamValueReplay<ParamType::TEGLSetBlobFuncANDROID>(std::ostream &os,
                                                              const CallCapture &call,
                                                              EGLSetBlobFuncANDROID value);
template <>
void WriteParamValueReplay<ParamType::TEGLClientBuffer>(std::ostream &os,
                                                        const CallCapture &call,
                                                        EGLClientBuffer value);

template <>
void WriteParamValueReplay<ParamType::TEGLAttribPointer>(std::ostream &os,
                                                         const CallCapture &call,
                                                         EGLAttrib *value);

template <>
void WriteParamValueReplay<ParamType::TEGLAttribConstPointer>(std::ostream &os,
                                                              const CallCapture &call,
                                                              const EGLAttrib *value);

template <>
void WriteParamValueReplay<ParamType::TEGLintPointer>(std::ostream &os,
                                                      const CallCapture &call,
                                                      EGLint *value);

template <>
void WriteParamValueReplay<ParamType::TEGLintConstPointer>(std::ostream &os,
                                                           const CallCapture &call,
                                                           const EGLint *value);

template <>
void WriteParamValueReplay<ParamType::Tegl_ConfigPointer>(std::ostream &os,
                                                          const CallCapture &call,
                                                          egl::Config *value);

template <>
void WriteParamValueReplay<ParamType::Tegl_SyncID>(std::ostream &os,
                                                   const CallCapture &call,
                                                   egl::SyncID value);

template <>
void WriteParamValueReplay<ParamType::TEGLTime>(std::ostream &os,
                                                const CallCapture &call,
                                                EGLTime value);

template <>
void WriteParamValueReplay<ParamType::TEGLTimeKHR>(std::ostream &os,
                                                   const CallCapture &call,
                                                   EGLTimeKHR value);

template <>
void WriteParamValueReplay<ParamType::TGLGETBLOBPROCANGLE>(std::ostream &os,
                                                           const CallCapture &call,
                                                           GLGETBLOBPROCANGLE value);

template <>
void WriteParamValueReplay<ParamType::TGLSETBLOBPROCANGLE>(std::ostream &os,
                                                           const CallCapture &call,
                                                           GLSETBLOBPROCANGLE value);

// General fallback for any unspecific type.
template <ParamType ParamT, typename T>
void WriteParamValueReplay(std::ostream &os, const CallCapture &call, T value)
{
    os << value;
}

struct FmtPointerIndex
{
    FmtPointerIndex(const void *ptrIn) : ptr(ptrIn) {}
    const void *ptr;
};

inline std::ostream &operator<<(std::ostream &os, const FmtPointerIndex &fmt)
{
    os << reinterpret_cast<uintptr_t>(fmt.ptr) << "ul";
    return os;
}

template <typename ParamValueType>
bool FindResourceIDsInCall(const CallCapture &call, std::vector<ParamValueType> &idsOut);
}  // namespace angle

#endif  // COMMON_FRAME_CAPTURE_UTILS_H_
