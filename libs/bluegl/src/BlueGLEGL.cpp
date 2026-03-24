/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <dlfcn.h>
#include <string.h>

#include <stdio.h>

namespace bluegl {

struct Driver {
    void* (*eglGetProcAddress)(const char*);
    void* library;
    void* glapi;
    void* glesv2;
} g_driver = {nullptr, nullptr, nullptr, nullptr};

extern "C" {
#define BLUEGL_EGL_STUB(name) \
    void* bluegl_##name; \
    void* name() { return bluegl_##name; }

    void* bluegl_eglGetProcAddress;
    void* bluegl_eglGetDisplay;
    void* bluegl_eglInitialize;
    void* bluegl_eglTerminate;
    void* bluegl_eglBindAPI;
    void* bluegl_eglCreateContext;
    void* bluegl_eglDestroyContext;
    void* bluegl_eglMakeCurrent;
    void* bluegl_eglCreatePbufferSurface;
    void* bluegl_eglCreateWindowSurface;
    void* bluegl_eglDestroySurface;
    void* bluegl_eglSurfaceAttrib;
    void* bluegl_eglSwapBuffers;
    void* bluegl_eglQueryString;
    void* bluegl_eglChooseConfig;
    void* bluegl_eglGetConfigAttrib;
    void* bluegl_eglGetConfigs;
    void* bluegl_eglGetCurrentContext;
    void* bluegl_eglReleaseThread;
    void* bluegl_eglGetError;
    void* bluegl_eglGetPlatformDisplayEXT;
}

bool initBinder() {
#ifdef __APPLE__
    const char* library_name = "libEGL.1.dylib";
    const char* glapi_name = "libglapi.0.dylib";
    const char* glesv2_name = "libGLESv2.2.dylib";
#else
    const char* library_name = "libEGL.so.1";
    const char* glapi_name = "libglapi.so.0";
    const char* glesv2_name = "libGLESv2.so.2";
#endif
    g_driver.library = dlopen(library_name, RTLD_GLOBAL | RTLD_NOW);

    if (!g_driver.library) {
        return false;
    }

    g_driver.glapi = dlopen(glapi_name, RTLD_GLOBAL | RTLD_NOW);
    g_driver.glesv2 = dlopen(glesv2_name, RTLD_GLOBAL | RTLD_NOW);

    g_driver.eglGetProcAddress = (void *(*)(const char *))
            dlsym(g_driver.library, "eglGetProcAddress");

#define LOAD_EGL_SYMBOL(name) bluegl_##name = dlsym(g_driver.library, #name)
    LOAD_EGL_SYMBOL(eglGetProcAddress);
    LOAD_EGL_SYMBOL(eglGetDisplay);
    LOAD_EGL_SYMBOL(eglInitialize);
    LOAD_EGL_SYMBOL(eglTerminate);
    LOAD_EGL_SYMBOL(eglBindAPI);
    LOAD_EGL_SYMBOL(eglCreateContext);
    LOAD_EGL_SYMBOL(eglDestroyContext);
    LOAD_EGL_SYMBOL(eglMakeCurrent);
    LOAD_EGL_SYMBOL(eglCreatePbufferSurface);
    LOAD_EGL_SYMBOL(eglCreateWindowSurface);
    LOAD_EGL_SYMBOL(eglDestroySurface);
    LOAD_EGL_SYMBOL(eglSurfaceAttrib);
    LOAD_EGL_SYMBOL(eglSwapBuffers);
    LOAD_EGL_SYMBOL(eglQueryString);
    LOAD_EGL_SYMBOL(eglChooseConfig);
    LOAD_EGL_SYMBOL(eglGetConfigAttrib);
    LOAD_EGL_SYMBOL(eglGetConfigs);
    LOAD_EGL_SYMBOL(eglGetCurrentContext);
    LOAD_EGL_SYMBOL(eglReleaseThread);
    LOAD_EGL_SYMBOL(eglGetError);
    bluegl_eglGetPlatformDisplayEXT = (void*)g_driver.eglGetProcAddress("eglGetPlatformDisplayEXT");

    return g_driver.eglGetProcAddress;
}

void* loadFunction(const char* name) {
  void* ptr = (void*) g_driver.eglGetProcAddress((const char*) name);
  if (!ptr) {
      if (g_driver.glesv2) {
          ptr = dlsym(g_driver.glesv2, name);
      }
      if (!ptr && g_driver.glapi) {
          ptr = dlsym(g_driver.glapi, name);
      }
      if (!ptr) {
          ptr = dlsym(g_driver.library, name);
      }
  }
  return ptr;
}

void shutdownBinder() {
    if (g_driver.library) dlclose(g_driver.library);
    if (g_driver.glapi) dlclose(g_driver.glapi);
    if (g_driver.glesv2) dlclose(g_driver.glesv2);
    memset(&g_driver, 0, sizeof(g_driver));
}

} // namespace bluegl
