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

package com.google.android.filament;

import com.google.android.filament.Platform;

public class DesktopPlatform extends Platform {
    public DesktopPlatform() {
    }

    @Override
    void log(String message) {
        System.out.println(message);
    }

    @Override
    void warn(String message) {
        System.out.println(message);
    }

    boolean validateStreamSource(Object object) {
        return false;
    }

    @Override
    boolean validateSurface(Object object) {
        return object instanceof java.awt.Canvas;
    }

    @Override
    boolean validateSharedContext(Object object) {
        return false;
    }

    @Override
    long getSharedContextNativeHandle(Object sharedContext) {
        return 0;
    }
}
