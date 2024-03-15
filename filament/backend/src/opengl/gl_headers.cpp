/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gl_headers.h"

#if defined(FILAMENT_IMPORT_ENTRY_POINTS)

#include <EGL/egl.h>
#include <mutex>

// for non EGL platforms, we'd need to implement this differently. Currently, it's not a problem.
template<typename T>
static void getProcAddress(T& pfn, const char* name) noexcept {
    pfn = (T)eglGetProcAddress(name);
}

namespace glext {
#ifndef __EMSCRIPTEN__
#ifdef GL_OES_EGL_image
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
#endif
#if GL_EXT_debug_marker
PFNGLINSERTEVENTMARKEREXTPROC glInsertEventMarkerEXT;
PFNGLPUSHGROUPMARKEREXTPROC glPushGroupMarkerEXT;
PFNGLPOPGROUPMARKEREXTPROC glPopGroupMarkerEXT;
#endif
#if GL_EXT_multisampled_render_to_texture
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
#endif
#ifdef GL_KHR_debug
PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
PFNGLGETDEBUGMESSAGELOGKHRPROC glGetDebugMessageLogKHR;
#endif
#ifdef GL_EXT_disjoint_timer_query
PFNGLGENQUERIESEXTPROC glGenQueriesEXT;
PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT;
PFNGLBEGINQUERYEXTPROC glBeginQueryEXT;
PFNGLENDQUERYEXTPROC glEndQueryEXT;
PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT;
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;
#endif
#ifdef GL_OES_vertex_array_object
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
#endif
#ifdef GL_EXT_clip_control
PFNGLCLIPCONTROLEXTPROC glClipControlEXT;
#endif
#ifdef GL_EXT_discard_framebuffer
PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT;
#endif
#ifdef GL_KHR_parallel_shader_compile
PFNGLMAXSHADERCOMPILERTHREADSKHRPROC glMaxShaderCompilerThreadsKHR;
#endif
#ifdef GL_OVR_multiview
PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
#endif

#if defined(__ANDROID__) && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
// On Android, If we want to support a build system less than ANDROID_API 21, we need to
// use getProcAddress for ES3.1 and above entry points.
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
#endif
static std::once_flag sGlExtInitialized;
#endif // __EMSCRIPTEN__

void importGLESExtensionsEntryPoints() {
#ifndef __EMSCRIPTEN__
    std::call_once(sGlExtInitialized, +[]() {
#ifdef GL_OES_EGL_image
    getProcAddress(glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES");
#endif
#if GL_EXT_debug_marker
    getProcAddress(glInsertEventMarkerEXT, "glInsertEventMarkerEXT");
    getProcAddress(glPushGroupMarkerEXT, "glPushGroupMarkerEXT");
    getProcAddress(glPopGroupMarkerEXT, "glPopGroupMarkerEXT");
#endif
#if GL_EXT_multisampled_render_to_texture
    getProcAddress(glFramebufferTexture2DMultisampleEXT, "glFramebufferTexture2DMultisampleEXT");
    getProcAddress(glRenderbufferStorageMultisampleEXT, "glRenderbufferStorageMultisampleEXT");
#endif
#ifdef GL_KHR_debug
    getProcAddress(glDebugMessageCallbackKHR, "glDebugMessageCallbackKHR");
    getProcAddress(glGetDebugMessageLogKHR, "glGetDebugMessageLogKHR");
#endif
#ifdef GL_EXT_disjoint_timer_query
    getProcAddress(glGenQueriesEXT, "glGenQueriesEXT");
    getProcAddress(glDeleteQueriesEXT, "glDeleteQueriesEXT");
    getProcAddress(glBeginQueryEXT, "glBeginQueryEXT");
    getProcAddress(glEndQueryEXT, "glEndQueryEXT");
    getProcAddress(glGetQueryObjectuivEXT, "glGetQueryObjectuivEXT");
    getProcAddress(glGetQueryObjectui64vEXT, "glGetQueryObjectui64vEXT");
#endif
#if defined(GL_OES_vertex_array_object)
    getProcAddress(glBindVertexArrayOES, "glBindVertexArrayOES");
    getProcAddress(glDeleteVertexArraysOES, "glDeleteVertexArraysOES");
    getProcAddress(glGenVertexArraysOES, "glGenVertexArraysOES");
#endif
#ifdef GL_EXT_clip_control
    getProcAddress(glClipControlEXT, "glClipControlEXT");
#endif
#ifdef GL_EXT_discard_framebuffer
        getProcAddress(glDiscardFramebufferEXT, "glDiscardFramebufferEXT");
#endif
#ifdef GL_KHR_parallel_shader_compile
        getProcAddress(glMaxShaderCompilerThreadsKHR, "glMaxShaderCompilerThreadsKHR");
#endif
#ifdef GL_OVR_multiview
        getProcAddress(glFramebufferTextureMultiviewOVR, "glFramebufferTextureMultiviewOVR");
#endif
#if defined(__ANDROID__) && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
        getProcAddress(glDispatchCompute, "glDispatchCompute");
#endif
    });
#endif // __EMSCRIPTEN__
}

} // namespace glext

#endif // defined(FILAMENT_IMPORT_ENTRY_POINTS)
