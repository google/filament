/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <GL/gl.h>
#include <dlfcn.h>
#include <string.h>

#ifdef FILAMENT_USE_EGL_OPENGL
#include <EGL/egl.h>
#endif

namespace bluegl {

struct Driver {
    void* (*glXGetProcAddress)(const GLubyte*);
    void* library;
} g_driver = {nullptr, nullptr};

bool initBinder() {
#ifdef FILAMENT_USE_EGL_OPENGL
    return true;
#endif
    const char* library_name = "libGL.so.1";
    g_driver.library = dlopen(library_name, RTLD_GLOBAL | RTLD_NOW);

    if (!g_driver.library) {
        return false;
    }

    g_driver.glXGetProcAddress = (void *(*)(const GLubyte *))
            dlsym(g_driver.library, "glXGetProcAddressARB");

    return g_driver.glXGetProcAddress;
}

void* loadFunction(const char* name) {
#ifdef FILAMENT_USE_EGL_OPENGL
  return (void *)eglGetProcAddress(name);
#else
  return (void*) g_driver.glXGetProcAddress((const GLubyte*) name);
#endif
}

void shutdownBinder() {
#ifdef FILAMENT_USE_EGL_OPENGL
    return;
#endif
    dlclose(g_driver.library);
    memset(&g_driver, 0, sizeof(g_driver));
}

} // namespace bluegl
