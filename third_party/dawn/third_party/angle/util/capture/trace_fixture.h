//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_fixture.h:
//   Common code for the ANGLE trace replays.
//

#ifndef ANGLE_TRACE_FIXTURE_H_
#define ANGLE_TRACE_FIXTURE_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "angle_gl.h"
#include "trace_interface.h"
#include "traces_export.h"

#if defined(__cplusplus)
#    include <cstdio>
#    include <cstring>
#    include <limits>
#    include <unordered_map>
#    include <vector>

// TODO(jmadill): Consolidate. http://anglebug.com/42266223
using BlockIndexesMap = std::unordered_map<GLuint, std::unordered_map<GLuint, GLuint>>;
extern BlockIndexesMap gUniformBlockIndexes;
using BufferHandleMap = std::unordered_map<GLuint, void *>;
extern BufferHandleMap gMappedBufferData;
using ClientBufferMap = std::unordered_map<uintptr_t, EGLClientBuffer>;
extern ClientBufferMap gClientBufferMap;
using EGLImageMap = std::unordered_map<uintptr_t, GLeglImageOES>;
extern EGLImageMap gEGLImageMap;
using SyncResourceMap = std::unordered_map<uintptr_t, GLsync>;
extern SyncResourceMap gSyncMap;
using SurfaceMap = std::unordered_map<uintptr_t, EGLSurface>;
extern SurfaceMap gSurfaceMap;
using ContextMap = std::unordered_map<uintptr_t, EGLContext>;
extern ContextMap gContextMap;

extern std::string gBinaryDataDir;
extern angle::TraceInfo gTraceInfo;
extern std::string gTraceGzPath;

using ValidateSerializedStateCallback = void (*)(const char *, const char *, uint32_t);

extern "C" {

// Functions implemented by traces.
// "not exported" tag is a hack to get around trace interpreter codegen -_-
/* not exported */ void SetupReplay();
/* not exported */ void ReplayFrame(uint32_t frameIndex);
/* not exported */ void ResetReplay();
/* not exported */ void FinishReplay();

ANGLE_REPLAY_EXPORT void SetValidateSerializedStateCallback(
    ValidateSerializedStateCallback callback);

// Only defined if serialization is enabled.
ANGLE_REPLAY_EXPORT const char *GetSerializedContextState(uint32_t frameIndex);

ANGLE_REPLAY_EXPORT void SetupEntryPoints(angle::TraceCallbacks *traceCallbacks,
                                          angle::TraceFunctions **traceFunctions);
#endif  // defined(__cplusplus)

// Maps from <captured Program ID, captured location> to run-time location.
extern GLint **gUniformLocations;
extern GLuint gCurrentProgram;

void UpdateUniformLocation(GLuint program, const char *name, GLint location, GLint count);
void DeleteUniformLocations(GLuint program);
void UpdateUniformBlockIndex(GLuint program, const char *name, GLuint index);
void UniformBlockBinding(GLuint program, GLuint uniformblockIndex, GLuint binding);
void UpdateCurrentProgram(GLuint program);

// Global state

extern uint8_t *gBinaryData;
extern uint8_t *gReadBuffer;
extern uint8_t *gClientArrays[];
extern GLuint *gResourceIDBuffer;

extern GLuint *gBufferMap;
extern GLuint *gFenceNVMap;
extern GLuint *gFramebufferMap;
extern GLuint **gFramebufferMapPerContext;
extern GLuint *gMemoryObjectMap;
extern GLuint *gProgramPipelineMap;
extern GLuint *gQueryMap;
extern GLuint *gRenderbufferMap;
extern GLuint *gSamplerMap;
extern GLuint *gSemaphoreMap;
extern GLuint *gShaderProgramMap;
extern GLuint *gTextureMap;
extern GLuint *gTransformFeedbackMap;
extern GLuint *gVertexArrayMap;

// TODO(jmadill): Consolidate. http://anglebug.com/42266223
extern GLeglImageOES *gEGLImageMap2;
extern EGLSurface *gSurfaceMap2;
extern EGLContext *gContextMap2;
extern GLsync *gSyncMap2;
extern EGLSync *gEGLSyncMap;
extern EGLDisplay gEGLDisplay;
extern angle::ReplayResourceMode gReplayResourceMode;

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
                       uint32_t maxEGLSyncID);

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
                       uint32_t maxVertexArray);

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
                       uint32_t maxVertexArray);

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
                      uint32_t maxVertexArray);

void UpdateClientArrayPointer(int arrayIndex, const void *data, uint64_t size);
void UpdateClientBufferData(GLuint bufferID, const void *source, GLsizei size);
void UpdateClientBufferDataWithOffset(GLuint bufferID,
                                      const void *source,
                                      GLsizei size,
                                      GLsizei offset);
void UpdateResourceIDBuffer(int resourceIndex, GLuint id);
void UpdateBufferID(GLuint id, GLsizei readBufferOffset);
void UpdateFenceNVID(GLuint id, GLsizei readBufferOffset);
void UpdateFramebufferID(GLuint id, GLsizei readBufferOffset);
void UpdateFramebufferID2(GLuint contextId, GLuint id, GLsizei readBufferOffset);
void UpdateMemoryObjectID(GLuint id, GLsizei readBufferOffset);
void UpdateProgramPipelineID(GLuint id, GLsizei readBufferOffset);
void UpdateQueryID(GLuint id, GLsizei readBufferOffset);
void UpdateRenderbufferID(GLuint id, GLsizei readBufferOffset);
void UpdateSamplerID(GLuint id, GLsizei readBufferOffset);
void UpdateSemaphoreID(GLuint id, GLsizei readBufferOffset);
void UpdateShaderProgramID(GLuint id, GLsizei readBufferOffset);
void UpdateTextureID(GLuint id, GLsizei readBufferOffset);
void UpdateTransformFeedbackID(GLuint id, GLsizei readBufferOffset);
void UpdateVertexArrayID(GLuint id, GLsizei readBufferOffset);

void SetCurrentContextID(GLuint id);

void SetFramebufferID(GLuint id);
void SetFramebufferID2(GLuint contextID, GLuint id);
void SetBufferID(GLuint id);
void SetRenderbufferID(GLuint id);
void SetTextureID(GLuint id);

// These functions allow the traces to change variable assignments into function calls,
// which makes it so the trace C interpreter doesn't need to implement operators at all.
void MapBufferRange(GLenum target,
                    GLintptr offset,
                    GLsizeiptr length,
                    GLbitfield access,
                    GLuint buffer);
void MapBufferRangeEXT(GLenum target,
                       GLintptr offset,
                       GLsizeiptr length,
                       GLbitfield access,
                       GLuint buffer);
void MapBufferOES(GLenum target, GLbitfield access, GLuint buffer);
void CreateShader(GLenum shaderType, GLuint shaderProgram);
void CreateProgram(GLuint shaderProgram);
void CreateShaderProgramv(GLenum type,
                          GLsizei count,
                          const GLchar *const *strings,
                          GLuint shaderProgram);
void FenceSync(GLenum condition, GLbitfield flags, uintptr_t fenceSync);
void FenceSync2(GLenum condition, GLbitfield flags, uintptr_t fenceSync);
void CreateEGLImage(EGLDisplay dpy,
                    EGLContext ctx,
                    EGLenum target,
                    uintptr_t buffer,
                    const EGLAttrib *attrib_list,
                    GLsizei width,
                    GLsizei height,
                    GLuint imageID);
void CreateEGLImageKHR(EGLDisplay dpy,
                       EGLContext ctx,
                       EGLenum target,
                       uintptr_t buffer,
                       const EGLint *attrib_list,
                       GLsizei width,
                       GLsizei height,
                       GLuint imageID);
void DestroyEGLImage(EGLDisplay dpy, EGLImage image, GLuint imageID);
void DestroyEGLImageKHR(EGLDisplay dpy, EGLImageKHR image, GLuint imageID);
void CreateEGLSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list, GLuint syncID);
void CreateEGLSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list, GLuint syncID);
void CreatePbufferSurface(EGLDisplay dpy,
                          EGLConfig config,
                          const EGLint *attrib_list,
                          GLuint surfaceID);
void CreateNativeClientBufferANDROID(const EGLint *attrib_list, uintptr_t clientBuffer);
void CreateContext(GLuint contextID);

void ValidateSerializedState(const char *serializedState, const char *fileName, uint32_t line);
#define VALIDATE_CHECKPOINT(STATE) ValidateSerializedState(STATE, __FILE__, __LINE__)

#if defined(__cplusplus)
}       // extern "C"
#endif  // defined(__cplusplus)

#endif  // ANGLE_TRACE_FIXTURE_H_
