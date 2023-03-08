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
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v;
#endif
#ifdef GL_EXT_clip_control
PFNGLCLIPCONTROLEXTPROC glClipControlEXT;
#endif

#if defined(__ANDROID__)
// On Android, If we want to support a build system less than ANDROID_API 21, we need to
// use getProcAddress for ES3.1 and above entry points.
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
#endif

static std::once_flag sGlExtInitialized;

void importGLESExtensionsEntryPoints() {
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
    getProcAddress(glGetQueryObjectui64v, "glGetQueryObjectui64vEXT");
#endif
#ifdef GL_EXT_clip_control
    getProcAddress(glClipControlEXT, "glClipControlEXT");
#endif
#if defined(__ANDROID__)
        getProcAddress(glDispatchCompute, "glDispatchCompute");
#endif
    });
}

} // namespace glext

#endif // defined(FILAMENT_IMPORT_ENTRY_POINTS)
