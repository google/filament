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

package com.google.android.filament.tungsten

import com.google.android.filament.tungsten.ui.TungstenFile
import java.io.File
import java.io.IOException
import java.util.function.BiConsumer
import java.util.function.Consumer

class StandaloneFile(private val filePath: String) : TungstenFile {

    override fun write(contents: String, callback: Consumer<Boolean>) {
        try {
            File(filePath).writeText(contents)
            callback.accept(true)
        } catch (e: IOException) {
            e.printStackTrace()
            callback.accept(false)
        }
    }

    override fun read(result: BiConsumer<Boolean, String?>) {
        try {
            result.accept(true, File(filePath).readText())
        } catch (e: IOException) {
            e.printStackTrace()
            result.accept(false, null)
        }
    }
}

/**
 * An InMemoryFile represents a file that has not been written to disk yet.
 * When the user opens the standalone version of Tungsten, they are editing this in-memory buffer.
 * When a file is first chosen to save to, the InMemoryFile can be "upgraded" to a StandaloneFile.
 */
class InMemoryFile(private var buffer: String = "") : TungstenFile {

    override fun write(contents: String, callback: Consumer<Boolean>) {
        buffer = contents
        callback.accept(true)
    }

    override fun read(result: BiConsumer<Boolean, String?>) {
        result.accept(true, buffer)
    }
}