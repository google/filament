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

package com.google.android.filament.android;

import android.os.Build;
import android.view.Display;

import com.google.android.filament.Renderer;

import androidx.annotation.NonNull;

public class DisplayHelper {
    public static long getAppVsyncOffsetNanos(Display display) {
        if (Build.VERSION.SDK_INT >= 29) {
            return display.getAppVsyncOffsetNanos();
        }
        return 0;
    }

    public static long getPresentationDeadlineNanos(Display display) {
        if (Build.VERSION.SDK_INT >= 29) {
            return display.getPresentationDeadlineNanos();
        }
        return 0;
    }

    public static long getRefreshPeriodNanos(Display display) {
        return (long) (1000000000.0 / display.getRefreshRate());
    }

    public static float getRefreshRate(Display display) {
        return display.getRefreshRate();
    }


    @NonNull
    public static Renderer.DisplayInfo getDisplayInfo(Display display, @NonNull Renderer.DisplayInfo info) {
        info.refreshRate = DisplayHelper.getRefreshRate(display);
        info.presentationDeadlineNanos = DisplayHelper.getPresentationDeadlineNanos(display);
        info.vsyncOffsetNanos = DisplayHelper.getAppVsyncOffsetNanos(display);
        return info;
    }
}
