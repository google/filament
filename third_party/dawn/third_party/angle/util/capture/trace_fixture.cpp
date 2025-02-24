//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_fixture.cpp:
//   Common code for the ANGLE trace replays.
//

#include "trace_fixture.h"

#include "angle_trace_gl.h"

#include <string>

namespace
{
void UpdateResourceMap(GLuint *resourceMap, GLuint id, GLsizei readBufferOffset)
{
    GLuint returnedID;
    memcpy(&returnedID, &gReadBuffer[readBufferOffset], sizeof(GLuint));
    resourceMap[id] = returnedID;
}

void UpdateResourceMapPerContext(GLuint **resourceArray,
                                 GLuint contextId,
                                 GLuint id,
                                 GLsizei readBufferOffset)
{
    GLuint returnedID;
    memcpy(&returnedID, &gReadBuffer[readBufferOffset], sizeof(GLuint));
    resourceArray[contextId][id] = returnedID;
}

uint32_t gMaxContexts                  = 0;
angle::TraceCallbacks *gTraceCallbacks = nullptr;

EGLClientBuffer GetClientBuffer(EGLenum target, uintptr_t key)
{
    switch (target)
    {
        case EGL_GL_TEXTURE_2D:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case EGL_GL_TEXTURE_3D:
        {
            uintptr_t id = static_cast<uintptr_t>(gTextureMap[key]);
            return reinterpret_cast<EGLClientBuffer>(id);
        }
        case EGL_GL_RENDERBUFFER:
        {
            uintptr_t id = static_cast<uintptr_t>(gRenderbufferMap[key]);
            return reinterpret_cast<EGLClientBuffer>(id);
        }
        default:
        {
            const auto &iData = gClientBufferMap.find(key);
            return iData != gClientBufferMap.end() ? iData->second : nullptr;
        }
    }
}

ValidateSerializedStateCallback gValidateSerializedStateCallback;
std::unordered_map<GLuint, std::vector<GLint>> gInternalUniformLocationsMap;

constexpr size_t kMaxClientArrays = 16;
}  // namespace

GLint **gUniformLocations;
GLuint gCurrentProgram = 0;

// TODO(jmadill): Hide from the traces. http://anglebug.com/42266223
BlockIndexesMap gUniformBlockIndexes;

void UpdateUniformLocation(GLuint program, const char *name, GLint location, GLint count)
{
    std::vector<GLint> &programLocations = gInternalUniformLocationsMap[program];
    if (static_cast<GLint>(programLocations.size()) < location + count)
    {
        programLocations.resize(location + count, 0);
    }
    GLuint mappedProgramID = gShaderProgramMap[program];
    for (GLint arrayIndex = 0; arrayIndex < count; ++arrayIndex)
    {
        programLocations[location + arrayIndex] =
            glGetUniformLocation(mappedProgramID, name) + arrayIndex;
    }
    gUniformLocations[program] = programLocations.data();
}

void DeleteUniformLocations(GLuint program)
{
    // No-op. We leave uniform locations around so deleted current programs can still use them.
}

void UpdateUniformBlockIndex(GLuint program, const char *name, GLuint index)
{
    gUniformBlockIndexes[program][index] = glGetUniformBlockIndex(program, name);
}

void UniformBlockBinding(GLuint program, GLuint uniformblockIndex, GLuint binding)
{
    glUniformBlockBinding(gShaderProgramMap[program],
                          gUniformBlockIndexes[gShaderProgramMap[program]][uniformblockIndex],
                          binding);
}

void UpdateCurrentProgram(GLuint program)
{
    gCurrentProgram = program;
}

uint8_t *gBinaryData;
uint8_t *gReadBuffer;
uint8_t *gClientArrays[kMaxClientArrays];
GLuint *gResourceIDBuffer;
SyncResourceMap gSyncMap;
ContextMap gContextMap;
GLuint gShareContextId;
GLuint *gBufferMap;
GLuint *gFenceNVMap;
GLuint *gFramebufferMap;
GLuint **gFramebufferMapPerContext;
GLuint *gMemoryObjectMap;
GLuint *gProgramPipelineMap;
GLuint *gQueryMap;
GLuint *gRenderbufferMap;
GLuint *gSamplerMap;
GLuint *gSemaphoreMap;
GLuint *gShaderProgramMap;
GLuint *gTextureMap;
GLuint *gTransformFeedbackMap;
GLuint *gVertexArrayMap;

// TODO(jmadill): Consolidate. http://anglebug.com/42266223
ClientBufferMap gClientBufferMap;
EGLImageMap gEGLImageMap;
SurfaceMap gSurfaceMap;

GLeglImageOES *gEGLImageMap2;
GLuint *gEGLImageMap2Resources;
EGLSurface *gSurfaceMap2;
EGLContext *gContextMap2;
GLsync *gSyncMap2;
EGLSync *gEGLSyncMap;
EGLDisplay gEGLDisplay;

std::string gBinaryDataDir = ".";

angle::ReplayResourceMode gReplayResourceMode = angle::ReplayResourceMode::Active;

template <typename T>
T *AllocateZeroedValues(size_t count)
{
    T *mem = new T[count + 1];
    memset(mem, 0, sizeof(T) * (count + 1));
    return mem;
}

GLuint *AllocateZeroedUints(size_t count)
{
    return AllocateZeroedValues<GLuint>(count);
}

void InitializeReplay4(const char *binaryDataFileName,
                       size_t maxClientArraySize,
                       size_t readBufferSize,
                       size_t resourceIDBufferSize,
                       GLuint contextId,
                       uint32_t maxBuffer,
                       uint32_t maxContext,
                       uint32_t maxFenceNV,
                       uint32_t maxFramebuffer,
                       uint32_t maxImage,
                       uint32_t maxMemoryObject,
                       uint32_t maxProgramPipeline,
                       uint32_t maxQuery,
                       uint32_t maxRenderbuffer,
                       uint32_t maxSampler,
                       uint32_t maxSemaphore,
                       uint32_t maxShaderProgram,
                       uint32_t maxSurface,
                       uint32_t maxSync,
                       uint32_t maxTexture,
                       uint32_t maxTransformFeedback,
                       uint32_t maxVertexArray,
                       GLuint maxEGLSyncID)
{
    InitializeReplay3(binaryDataFileName, maxClientArraySize, readBufferSize, resourceIDBufferSize,
                      contextId, maxBuffer, maxContext, maxFenceNV, maxFramebuffer, maxImage,
                      maxMemoryObject, maxProgramPipeline, maxQuery, maxRenderbuffer, maxSampler,
                      maxSemaphore, maxShaderProgram, maxSurface, maxSync, maxTexture,
                      maxTransformFeedback, maxVertexArray);
    gEGLSyncMap = AllocateZeroedValues<EGLSync>(maxEGLSyncID);
    gEGLDisplay = eglGetCurrentDisplay();

    gMaxContexts              = maxContext + 1;
    gFramebufferMapPerContext = new GLuint *[gMaxContexts];
    memset(gFramebufferMapPerContext, 0, sizeof(GLuint *) * (gMaxContexts));
    for (uint8_t i = 0; i < gMaxContexts; i++)
    {
        gFramebufferMapPerContext[i] = AllocateZeroedValues<GLuint>(maxFramebuffer);
    }
}

void InitializeReplay3(const char *binaryDataFileName,
                       size_t maxClientArraySize,
                       size_t readBufferSize,
                       size_t resourceIDBufferSize,
                       GLuint contextId,
                       uint32_t maxBuffer,
                       uint32_t maxContext,
                       uint32_t maxFenceNV,
                       uint32_t maxFramebuffer,
                       uint32_t maxImage,
                       uint32_t maxMemoryObject,
                       uint32_t maxProgramPipeline,
                       uint32_t maxQuery,
                       uint32_t maxRenderbuffer,
                       uint32_t maxSampler,
                       uint32_t maxSemaphore,
                       uint32_t maxShaderProgram,
                       uint32_t maxSurface,
                       uint32_t maxSync,
                       uint32_t maxTexture,
                       uint32_t maxTransformFeedback,
                       uint32_t maxVertexArray)
{
    InitializeReplay2(binaryDataFileName, maxClientArraySize, readBufferSize, contextId, maxBuffer,
                      maxContext, maxFenceNV, maxFramebuffer, maxImage, maxMemoryObject,
                      maxProgramPipeline, maxQuery, maxRenderbuffer, maxSampler, maxSemaphore,
                      maxShaderProgram, maxSurface, maxTexture, maxTransformFeedback,
                      maxVertexArray);

    gSyncMap2         = AllocateZeroedValues<GLsync>(maxSync);
    gResourceIDBuffer = AllocateZeroedUints(resourceIDBufferSize);
}

void InitializeReplay2(const char *binaryDataFileName,
                       size_t maxClientArraySize,
                       size_t readBufferSize,
                       GLuint contextId,
                       uint32_t maxBuffer,
                       uint32_t maxContext,
                       uint32_t maxFenceNV,
                       uint32_t maxFramebuffer,
                       uint32_t maxImage,
                       uint32_t maxMemoryObject,
                       uint32_t maxProgramPipeline,
                       uint32_t maxQuery,
                       uint32_t maxRenderbuffer,
                       uint32_t maxSampler,
                       uint32_t maxSemaphore,
                       uint32_t maxShaderProgram,
                       uint32_t maxSurface,
                       uint32_t maxTexture,
                       uint32_t maxTransformFeedback,
                       uint32_t maxVertexArray)
{
    InitializeReplay(binaryDataFileName, maxClientArraySize, readBufferSize, maxBuffer, maxFenceNV,
                     maxFramebuffer, maxMemoryObject, maxProgramPipeline, maxQuery, maxRenderbuffer,
                     maxSampler, maxSemaphore, maxShaderProgram, maxTexture, maxTransformFeedback,
                     maxVertexArray);

    gContextMap2           = AllocateZeroedValues<EGLContext>(maxContext);
    gEGLImageMap2          = AllocateZeroedValues<EGLImage>(maxImage);
    gEGLImageMap2Resources = AllocateZeroedValues<GLuint>(maxImage);
    gSurfaceMap2           = AllocateZeroedValues<EGLSurface>(maxSurface);

    gContextMap2[0]         = EGL_NO_CONTEXT;
    gShareContextId         = contextId;
    gContextMap2[contextId] = eglGetCurrentContext();
}

void InitializeReplay(const char *binaryDataFileName,
                      size_t maxClientArraySize,
                      size_t readBufferSize,
                      uint32_t maxBuffer,
                      uint32_t maxFenceNV,
                      uint32_t maxFramebuffer,
                      uint32_t maxMemoryObject,
                      uint32_t maxProgramPipeline,
                      uint32_t maxQuery,
                      uint32_t maxRenderbuffer,
                      uint32_t maxSampler,
                      uint32_t maxSemaphore,
                      uint32_t maxShaderProgram,
                      uint32_t maxTexture,
                      uint32_t maxTransformFeedback,
                      uint32_t maxVertexArray)
{
    gBinaryData = gTraceCallbacks->LoadBinaryData(binaryDataFileName);

    for (uint8_t *&clientArray : gClientArrays)
    {
        clientArray = new uint8_t[maxClientArraySize];
    }

    gReadBuffer = new uint8_t[readBufferSize];

    gBufferMap            = AllocateZeroedUints(maxBuffer);
    gFenceNVMap           = AllocateZeroedUints(maxFenceNV);
    gFramebufferMap       = AllocateZeroedUints(maxFramebuffer);
    gMemoryObjectMap      = AllocateZeroedUints(maxMemoryObject);
    gProgramPipelineMap   = AllocateZeroedUints(maxProgramPipeline);
    gQueryMap             = AllocateZeroedUints(maxQuery);
    gRenderbufferMap      = AllocateZeroedUints(maxRenderbuffer);
    gSamplerMap           = AllocateZeroedUints(maxSampler);
    gSemaphoreMap         = AllocateZeroedUints(maxSemaphore);
    gShaderProgramMap     = AllocateZeroedUints(maxShaderProgram);
    gTextureMap           = AllocateZeroedUints(maxTexture);
    gTransformFeedbackMap = AllocateZeroedUints(maxTransformFeedback);
    gVertexArrayMap       = AllocateZeroedUints(maxVertexArray);

    gUniformLocations = new GLint *[maxShaderProgram + 1];
    memset(gUniformLocations, 0, sizeof(GLint *) * (maxShaderProgram + 1));

    gContextMap[0] = EGL_NO_CONTEXT;
}

void FinishReplay()
{
    for (uint8_t *&clientArray : gClientArrays)
    {
        delete[] clientArray;
    }
    delete[] gReadBuffer;
    delete[] gResourceIDBuffer;
    delete[] gBufferMap;
    delete[] gContextMap2;
    delete[] gEGLImageMap2;
    delete[] gEGLSyncMap;
    delete[] gRenderbufferMap;
    delete[] gTextureMap;
    delete[] gFramebufferMap;
    delete[] gShaderProgramMap;
    delete[] gFenceNVMap;
    delete[] gMemoryObjectMap;
    delete[] gProgramPipelineMap;
    delete[] gQueryMap;
    delete[] gSamplerMap;
    delete[] gSemaphoreMap;
    delete[] gSurfaceMap2;
    delete[] gSyncMap2;
    delete[] gTransformFeedbackMap;
    delete[] gVertexArrayMap;

    for (uint8_t i = 0; i < gMaxContexts; i++)
    {
        delete[] gFramebufferMapPerContext[i];
    }
    delete[] gFramebufferMapPerContext;
}

void SetValidateSerializedStateCallback(ValidateSerializedStateCallback callback)
{
    gValidateSerializedStateCallback = callback;
}

angle::TraceInfo gTraceInfo;
std::string gTraceGzPath;

struct TraceFunctionsImpl : angle::TraceFunctions
{
    void SetupReplay() override { ::SetupReplay(); }

    void ReplayFrame(uint32_t frameIndex) override { ::ReplayFrame(frameIndex); }

    void ResetReplay() override { ::ResetReplay(); }

    void FinishReplay() override { ::FinishReplay(); }

    void SetBinaryDataDir(const char *dataDir) override { gBinaryDataDir = dataDir; }

    void SetReplayResourceMode(const angle::ReplayResourceMode resourceMode) override
    {
        gReplayResourceMode = resourceMode;
    }

    void SetTraceInfo(const angle::TraceInfo &traceInfo) override { gTraceInfo = traceInfo; }

    void SetTraceGzPath(const std::string &traceGzPath) override { gTraceGzPath = traceGzPath; }
};

TraceFunctionsImpl gTraceFunctionsImpl;

void SetupEntryPoints(angle::TraceCallbacks *traceCallbacks, angle::TraceFunctions **traceFunctions)
{
    gTraceCallbacks = traceCallbacks;
    *traceFunctions = &gTraceFunctionsImpl;
}

void UpdateClientArrayPointer(int arrayIndex, const void *data, uint64_t size)
{
    memcpy(gClientArrays[arrayIndex], data, static_cast<size_t>(size));
}
BufferHandleMap gMappedBufferData;

void UpdateClientBufferData(GLuint bufferID, const void *source, GLsizei size)
{
    memcpy(gMappedBufferData[gBufferMap[bufferID]], source, size);
}

void UpdateClientBufferDataWithOffset(GLuint bufferID,
                                      const void *source,
                                      GLsizei size,
                                      GLsizei offset)
{
    uintptr_t dest = reinterpret_cast<uintptr_t>(gMappedBufferData[gBufferMap[bufferID]]) + offset;
    memcpy(reinterpret_cast<void *>(dest), source, size);
}

void UpdateResourceIDBuffer(int resourceIndex, GLuint id)
{
    gResourceIDBuffer[resourceIndex] = id;
}

void UpdateBufferID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gBufferMap, id, readBufferOffset);
}

void UpdateFenceNVID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gFenceNVMap, id, readBufferOffset);
}

void UpdateFramebufferID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gFramebufferMap, id, readBufferOffset);
}

void UpdateFramebufferID2(GLuint contextId, GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMapPerContext(gFramebufferMapPerContext, contextId, id, readBufferOffset);
}

void UpdateMemoryObjectID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gMemoryObjectMap, id, readBufferOffset);
}

void UpdateProgramPipelineID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gProgramPipelineMap, id, readBufferOffset);
}

void UpdateQueryID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gQueryMap, id, readBufferOffset);
}

void UpdateRenderbufferID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gRenderbufferMap, id, readBufferOffset);
}

void UpdateSamplerID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gSamplerMap, id, readBufferOffset);
}

void UpdateSemaphoreID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gSemaphoreMap, id, readBufferOffset);
}

void UpdateShaderProgramID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gShaderProgramMap, id, readBufferOffset);
}

void UpdateTextureID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gTextureMap, id, readBufferOffset);
}

void UpdateTransformFeedbackID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gTransformFeedbackMap, id, readBufferOffset);
}

void UpdateVertexArrayID(GLuint id, GLsizei readBufferOffset)
{
    UpdateResourceMap(gVertexArrayMap, id, readBufferOffset);
}

void SetFramebufferID(GLuint id)
{
    glGenFramebuffers(1, &gFramebufferMap[id]);
}

void SetFramebufferID2(GLuint contextID, GLuint id)
{
    glGenFramebuffers(1, &gFramebufferMapPerContext[contextID][id]);
}

void SetBufferID(GLuint id)
{
    glGenBuffers(1, &gBufferMap[id]);
}

void SetRenderbufferID(GLuint id)
{
    glGenRenderbuffers(1, &gRenderbufferMap[id]);
}

void SetTextureID(GLuint id)
{
    glGenTextures(1, &gTextureMap[id]);
}

void ValidateSerializedState(const char *serializedState, const char *fileName, uint32_t line)
{
    if (gValidateSerializedStateCallback)
    {
        gValidateSerializedStateCallback(serializedState, fileName, line);
    }
}

void MapBufferRange(GLenum target,
                    GLintptr offset,
                    GLsizeiptr length,
                    GLbitfield access,
                    GLuint buffer)
{
    gMappedBufferData[gBufferMap[buffer]] = glMapBufferRange(target, offset, length, access);
}

void MapBufferRangeEXT(GLenum target,
                       GLintptr offset,
                       GLsizeiptr length,
                       GLbitfield access,
                       GLuint buffer)
{
    gMappedBufferData[gBufferMap[buffer]] = glMapBufferRangeEXT(target, offset, length, access);
}

void MapBufferOES(GLenum target, GLbitfield access, GLuint buffer)
{
    gMappedBufferData[gBufferMap[buffer]] = glMapBufferOES(target, access);
}

void CreateShader(GLenum shaderType, GLuint shaderProgram)
{
    gShaderProgramMap[shaderProgram] = glCreateShader(shaderType);
}

void CreateProgram(GLuint shaderProgram)
{
    gShaderProgramMap[shaderProgram] = glCreateProgram();
}

void CreateShaderProgramv(GLenum type,
                          GLsizei count,
                          const GLchar *const *strings,
                          GLuint shaderProgram)
{
    gShaderProgramMap[shaderProgram] = glCreateShaderProgramv(type, count, strings);
}

void FenceSync(GLenum condition, GLbitfield flags, uintptr_t fenceSync)
{
    gSyncMap[fenceSync] = glFenceSync(condition, flags);
}

void FenceSync2(GLenum condition, GLbitfield flags, uintptr_t fenceSync)
{
    gSyncMap2[fenceSync] = glFenceSync(condition, flags);
}

GLuint CreateEGLImageResource(GLsizei width, GLsizei height)
{
    GLint previousTexId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexId);
    GLint previousAlignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);

    // Create a texture and fill with a placeholder green value
    GLuint stagingTexId;
    glGenTextures(1, &stagingTexId);
    glBindTexture(GL_TEXTURE_2D, stagingTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    std::vector<GLubyte> pixels;
    pixels.reserve(width * height * 3);
    for (int i = 0; i < width * height; i++)
    {
        pixels.push_back(61);
        pixels.push_back(220);
        pixels.push_back(132);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 pixels.data());

    glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
    glBindTexture(GL_TEXTURE_2D, previousTexId);
    return stagingTexId;
}

void CreateEGLImage(EGLDisplay dpy,
                    EGLContext ctx,
                    EGLenum target,
                    uintptr_t buffer,
                    const EGLAttrib *attrib_list,
                    GLsizei width,
                    GLsizei height,
                    GLuint imageID)
{
    if (target == EGL_NATIVE_BUFFER_ANDROID || buffer == 0)
    {
        // If this image was created from an AHB or the backing resource was not
        // captured, create a new GL texture during replay to use instead.
        // Substituting a GL texture for an AHB allows the trace to run on
        // non-Android systems.
        gEGLImageMap2Resources[imageID] = CreateEGLImageResource(width, height);
        gEGLImageMap2[imageID]          = eglCreateImage(
            dpy, eglGetCurrentContext(), EGL_GL_TEXTURE_2D,
            reinterpret_cast<EGLClientBuffer>(gEGLImageMap2Resources[imageID]), attrib_list);
    }
    else
    {
        EGLClientBuffer clientBuffer = GetClientBuffer(target, buffer);
        gEGLImageMap2[imageID]       = eglCreateImage(dpy, ctx, target, clientBuffer, attrib_list);
    }
}

void CreateEGLImageKHR(EGLDisplay dpy,
                       EGLContext ctx,
                       EGLenum target,
                       uintptr_t buffer,
                       const EGLint *attrib_list,
                       GLsizei width,
                       GLsizei height,
                       GLuint imageID)
{
    if (target == EGL_NATIVE_BUFFER_ANDROID || buffer == 0)
    {
        gEGLImageMap2Resources[imageID] = CreateEGLImageResource(width, height);
        gEGLImageMap2[imageID]          = eglCreateImageKHR(
            dpy, eglGetCurrentContext(), EGL_GL_TEXTURE_2D,
            reinterpret_cast<EGLClientBuffer>(gEGLImageMap2Resources[imageID]), attrib_list);
    }
    else
    {
        EGLClientBuffer clientBuffer = GetClientBuffer(target, buffer);
        gEGLImageMap2[imageID] = eglCreateImageKHR(dpy, ctx, target, clientBuffer, attrib_list);
    }
}

void DestroyEGLImage(EGLDisplay dpy, EGLImage image, GLuint imageID)
{
    if (gEGLImageMap2Resources[imageID])
    {
        glDeleteTextures(1, &gEGLImageMap2Resources[imageID]);
        gEGLImageMap2Resources[imageID] = 0;
    }
    eglDestroyImage(dpy, image);
}

void DestroyEGLImageKHR(EGLDisplay dpy, EGLImageKHR image, GLuint imageID)
{
    if (gEGLImageMap2Resources[imageID])
    {
        glDeleteTextures(1, &gEGLImageMap2Resources[imageID]);
        gEGLImageMap2Resources[imageID] = 0;
    }
    eglDestroyImageKHR(dpy, image);
}

void CreateEGLSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list, GLuint syncID)
{
    gEGLSyncMap[syncID] = eglCreateSyncKHR(dpy, type, attrib_list);
}

void CreateEGLSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list, GLuint syncID)
{
    gEGLSyncMap[syncID] = eglCreateSync(dpy, type, attrib_list);
}

void CreatePbufferSurface(EGLDisplay dpy,
                          EGLConfig config,
                          const EGLint *attrib_list,
                          GLuint surfaceID)
{
    gSurfaceMap2[surfaceID] = eglCreatePbufferSurface(dpy, config, attrib_list);
}

void CreateNativeClientBufferANDROID(const EGLint *attrib_list, uintptr_t clientBuffer)
{
    gClientBufferMap[clientBuffer] = eglCreateNativeClientBufferANDROID(attrib_list);
}

void CreateContext(GLuint contextID)
{
    EGLContext shareContext = gContextMap2[gShareContextId];
    EGLContext context      = eglCreateContext(nullptr, nullptr, shareContext, nullptr);
    gContextMap2[contextID] = context;
}

void SetCurrentContextID(GLuint id)
{
    gContextMap2[id] = eglGetCurrentContext();
}

ANGLE_REPLAY_EXPORT PFNEGLCREATEIMAGEPROC r_eglCreateImage;
ANGLE_REPLAY_EXPORT PFNEGLCREATEIMAGEKHRPROC r_eglCreateImageKHR;
ANGLE_REPLAY_EXPORT PFNEGLDESTROYIMAGEPROC r_eglDestroyImage;
ANGLE_REPLAY_EXPORT PFNEGLDESTROYIMAGEKHRPROC r_eglDestroyImageKHR;
