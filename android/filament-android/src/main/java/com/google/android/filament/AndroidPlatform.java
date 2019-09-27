/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.os.Build;
import android.util.Log;
import android.view.Surface;
import java.lang.reflect.Method;

final class AndroidPlatform extends Platform {
    private static final String LOG_TAG = "Filament";

    static {
        // workaround a deadlock during loading of /vendor/lib64/egl/libGLESv*
        // on Pixel 2. This loads the GL libraries before we load filament.
        EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
    }

    AndroidPlatform() { }

    @Override
    void log(String message) {
        Log.d(LOG_TAG, message);
    }

    @Override
    void warn(String message) {
        Log.w(LOG_TAG, message);
    }

    boolean validateStreamSource(Object object) {
        return object instanceof SurfaceTexture;
    }

    @Override
    boolean validateSurface(Object object) {
        return object instanceof Surface;
    }

    @Override
    boolean validateSharedContext(Object object) {
        return object instanceof EGLContext;
    }

    @Override
    long getSharedContextNativeHandle(Object sharedContext) {
        if (Build.VERSION.SDK_INT >= 21) {
            return AndroidPlatform21.getSharedContextNativeHandle(sharedContext);
        } else {
            try {
                //noinspection JavaReflectionMemberAccess
                Method method = EGLContext.class.getDeclaredMethod("getHandle");
                Integer handle = (Integer) method.invoke(sharedContext);
                //noinspection ConstantConditions
                return handle.longValue();
            } catch (Exception e) {
                Log.d(LOG_TAG, "Could not access shared context's native handle", e);
            }
            // Should not happen
            return 0;
        }
    }
}
