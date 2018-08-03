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

package com.google.android.filament.tungsten.util;

public final class OperatingSystem {

    private static final String OS_PROPERTY =
            System.getProperties().getProperty("os.name", "generic").toLowerCase();

    private OperatingSystem() {

    }

    public static boolean isWindows() {
        return OS_PROPERTY.contains("win");
    }

    public static boolean isMac() {
        return OS_PROPERTY.contains("mac") || OS_PROPERTY.contains("darwin");
    }

    public static boolean isLinux() {
        return OS_PROPERTY.contains("nux");
    }
}
