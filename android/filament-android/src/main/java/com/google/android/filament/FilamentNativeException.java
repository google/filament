/*
 * Copyright (C) 2025 The Android Open Source Project
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

/**
 * Exception thrown when a native C++ exception occurs on a worker thread.
 *
 * <p>This exception indicates that a C++ std::exception or unknown native exception
 * occurred on a background worker thread. When this exception is thrown, Filament
 * is automatically disabled to prevent further errors.</p>
 *
 * <p>Use this exception type for Firebase Crashlytics filtering to track native
 * exceptions separately from Filament panics.</p>
 *
 * @see Filament#healthCheck()
 * @see FilamentPanicException
 */
public class FilamentNativeException extends RuntimeException {
    /**
     * Constructs a new FilamentNativeException with the specified detail message.
     *
     * @param message the detail message describing the native exception
     */
    public FilamentNativeException(String message) {
        super(message);
    }
}
