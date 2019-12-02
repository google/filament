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

#if defined(__has_include)
#if __has_include(<darwin/jawt_md.h>)
#include <darwin/jawt_md.h>
#else
#include <jawt_md.h>
#endif
#else
#include <darwin/jawt_md.h>
#endif

#include <filament/Engine.h>
#include "JAWTUtils.h"

#import <Cocoa/Cocoa.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotReleasedValue"
extern "C" {
void *getNativeWindow(JNIEnv *env, jclass klass, jobject surface) {
    void *win = nullptr;
    JAWT_DrawingSurface *ds = nullptr;
    JAWT_DrawingSurfaceInfo *dsi = nullptr;

    if (!acquireDrawingSurface(env, surface, &ds, &dsi)) {
        return win;
    }
    NSObject<JAWT_SurfaceLayers>* jawldsip = (NSObject<JAWT_SurfaceLayers>*)dsi->platformInfo;
    // Use jawt_DrawingSurfaceInfo.bounds for frame dimension.
    NSView *view = [[NSView alloc] initWithFrame:
            NSMakeRect(dsi->bounds.x, dsi->bounds.y, dsi->bounds.width, dsi->bounds.height)];
    view.wantsLayer = true;
    [jawldsip setLayer:view.layer];

    win = (void*) view;
    releaseDrawingSurface(ds, dsi);
    return win;
}

jlong createNativeSurface(jint width, jint height) {
    NSView *view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
    view.wantsLayer = true;
    return (jlong) view;
}

void destroyNativeSurface(jlong surface) {
    NSView *view = reinterpret_cast<NSView*>(surface);
    [view release];
}

}
#pragma clang diagnostic pop
