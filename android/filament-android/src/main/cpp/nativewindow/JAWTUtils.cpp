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

#include "JAWTUtils.h"

#include <vector>

static std::vector<int> jawtVersions = {
        0x00010003,
        0x00010004,
        0x00010007,
        0x00010009,
};

bool acquireDrawingSurface(JNIEnv *env, jobject surface, JAWT_DrawingSurface** ods,
        JAWT_DrawingSurfaceInfo** odsi) {
    JAWT awt;
    JAWT_DrawingSurface *ds = nullptr;
    JAWT_DrawingSurfaceInfo *dsi = nullptr;

    // Search for a valid AWT
    jboolean foundJawt = false;
    for(int jawtVersion : jawtVersions) {
        awt.version = jawtVersion;
        foundJawt = JAWT_GetAWT(env, &awt);
        if (foundJawt == JNI_TRUE) {
            printf("Found valid AWT v%08x.\n", jawtVersion);
            break;
        } else {
            printf("AWT v%08x not present.\n", jawtVersion);
        }
    }
    fflush(stdout);

    if (foundJawt == JNI_FALSE) {
        printf("AWT Not found\n");
        fflush(stdout);
        return 0;
    }

    // Get the drawing surface
    ds = awt.GetDrawingSurface(env, surface);
    if (ds == NULL) {
        printf("NULL drawing surface\n");
        fflush(stdout);
        return 0;
    }

    // Lock the drawing
    jint lock = ds->Lock(ds);
    if ((lock & JAWT_LOCK_ERROR) != 0) {
        printf("Error locking surface\n");
        fflush(stdout);
        awt.FreeDrawingSurface(ds);
        return 0;
    }

    // Get the drawing surface info
    dsi = ds->GetDrawingSurfaceInfo(ds);
    if (dsi == NULL) {
        printf("Error getting surface info\n");
        fflush(stdout);
        ds->Unlock(ds);
        awt.FreeDrawingSurface(ds);
        return 0;
    }

    *odsi = dsi;
    *ods = ds;
    return 1;
}

void releaseDrawingSurface(JAWT_DrawingSurface *ds, JAWT_DrawingSurfaceInfo *dsi) {
    // Free the drawing surface info
    ds->FreeDrawingSurfaceInfo(dsi);
    // Unlock the drawing surface
    ds->Unlock(ds);
}
