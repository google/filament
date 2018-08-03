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

package com.google.android.filament.tungsten.ui

import java.util.function.BiConsumer
import java.util.function.Consumer

interface TungstenFile {

    /**
     * Write new contents to the file, replacing any existing contents.
     * @param callback is called with true if the write is successful, false otherwise.
     */
    fun write(contents: String, callback: Consumer<Boolean>)

    /**
     * Read contents of a file asynchronously. result callback will be called with a boolean
     * representing success and if successful, the full contents of the file.
     */
    fun read(result: BiConsumer<Boolean, String?>)
}