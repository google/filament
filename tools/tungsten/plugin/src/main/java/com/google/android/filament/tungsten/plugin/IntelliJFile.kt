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

package com.google.android.filament.tungsten.plugin

import com.google.android.filament.tungsten.ui.TungstenFile
import com.intellij.openapi.application.ApplicationManager
import com.intellij.openapi.vfs.VirtualFile
import java.io.IOException
import java.util.function.BiConsumer
import java.util.function.Consumer

class IntelliJFile(val file: VirtualFile) : TungstenFile {

    override fun write(contents: String, callback: Consumer<Boolean>) {
        ApplicationManager.getApplication().runWriteAction {
            try {
                file.setBinaryContent(contents.toByteArray())
                callback.accept(true)
            } catch (e: IOException) {
                e.printStackTrace()
                callback.accept(false)
            }
        }
    }

    override fun read(result: BiConsumer<Boolean, String?>) {
        ApplicationManager.getApplication().runReadAction {
            try {
                val contents = String(file.contentsToByteArray())
                result.accept(true, contents)
            } catch (e: IOException) {
                e.printStackTrace()
                result.accept(false, null)
            }
        }
    }
}