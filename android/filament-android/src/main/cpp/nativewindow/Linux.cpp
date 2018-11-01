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

#include <stdio.h>

#include <jawt.h>
#include <linux/jawt_md.h>

#include "JAWTUtils.h"
#include<GL/glx.h>

extern "C" {
void *getNativeWindow(JNIEnv* env, jclass, jobject surface) {
    void* win = nullptr;
    JAWT_DrawingSurface* ds = nullptr;
    JAWT_DrawingSurfaceInfo* dsi = nullptr;

    if (!acquireDrawingSurface(env, surface, &ds, &dsi)) {
        return win;
    }
    JAWT_X11DrawingSurfaceInfo* dsi_x11 = (JAWT_X11DrawingSurfaceInfo*) dsi->platformInfo;

    win = (void*) dsi_x11->drawable;
    releaseDrawingSurface(ds, dsi);
    return win;
}

jlong createNativeSurface(jint width, jint height) {
    Display* display = XOpenDisplay(nullptr);
    int screen = DefaultScreen(display);
    Window window = 0;

    #ifndef NDEBUG
        int major, minor;
        glXQueryVersion(display, &major, &minor);
        printf("Using GLX v%d.%d\n", major, minor); fflush(stdout);
    #endif

    static int visualAttributess[] = {
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER,  True,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_DEPTH_SIZE,   24,
        None
    };
    int numConfigs = 0;
    GLXFBConfig* configs = glXChooseFBConfig( display, screen, visualAttributess, &numConfigs);
    if (numConfigs == 0) {
        printf("Unable to find a suitable Framebuffer Configuration.\n"); fflush(stdout);
        return 0;
    }

    int pbufferAttributes[] = {
        GLX_PBUFFER_WIDTH, width,
        GLX_PBUFFER_HEIGHT, height,
        None
    };
    window = glXCreatePbuffer( display, configs[0], pbufferAttributes );
    XFree(configs);
    // Make sure pbuffer creation has not been buffered in the event queue (we need it NOW).
    XFlush(display);

    // Camouflage the pbuffer as a window which are both XID anyway.
    return (jlong) window;
}

void destroyNativeSurface(jlong surface) {
    const char* displayName = nullptr;
    Display* display = XOpenDisplay(displayName);
    GLXPbuffer pBuffer = (GLXPbuffer)surface;
    glXDestroyPbuffer(display, pBuffer);
}

}
