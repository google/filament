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

package com.google.android.filament.tungsten.texture

import com.google.android.filament.Texture
import com.google.android.filament.tungsten.Filament
import java.io.File
import java.util.concurrent.CompletableFuture

typealias FutureTexture = CompletableFuture<Texture>

object TextureCache {

    private data class TextureCacheKey(
        val canonicalPath: String,
        val colorSpace: TextureUtils.ColorSpaceStrategy
    )

    // Maps from canonical file path to Filament Texture
    private val cache = mutableMapOf<TextureCacheKey, FutureTexture>()
    private var defaultTexture: Texture? = null

    // Only accessed from Filament thread
    private var allowNewTextures = true
    private val textures = mutableListOf<Texture>()

    /**
     * Loads the image into a Filament texture and returns a future that will be completed when the
     * texture loads successfully or completed exceptionally if an error occurs.
     */
    fun getTextureForFile(file: File, colorSpace: TextureUtils.ColorSpaceStrategy): FutureTexture {
        val key = TextureCacheKey(file.canonicalPath, colorSpace)
        cache.computeIfAbsent(key) {
            createTextureForImageSource(file, colorSpace)
        }
        return cache[key] ?: getDefaultTexture()
    }

    fun getDefaultTexture(): FutureTexture {
        defaultTexture?.let { return CompletableFuture.completedFuture(it) }
        val futureTexture = FutureTexture()
        Filament.getInstance().runOnFilamentThread { engine ->
            val texture = TextureUtils.createDefaultTexture(engine)
            futureTexture.complete(texture)
            defaultTexture = texture
            textures.add(texture)
        }
        return futureTexture
    }

    /*
     * Keep track of texture and delete when Filament shutdownAndDestroyTextures is called.
     */
    fun addTextureForRemoval(texture: Texture) {
        Filament.getInstance().assertIsFilamentThread()
        textures.add(texture)
    }

    /**
     * Delete all cached textures and disallow any additional texture caching.
     */
    fun shutdownAndDestroyTextures() {
        Filament.getInstance().runOnFilamentThread { engine ->
            for (texture in textures) {
                engine.destroyTexture(texture)
            }
            allowNewTextures = false
        }
    }

    private fun createTextureForImageSource(
        imageSource: File,
        colorSpace: TextureUtils.ColorSpaceStrategy
    ): FutureTexture {
        val futureTexture = FutureTexture()
        Filament.getInstance().runOnFilamentThread { engine ->
            if (!allowNewTextures) return@runOnFilamentThread
            val texture = TextureUtils.loadTextureFromFile(engine, imageSource, colorSpace)
            if (texture != null) {
                futureTexture.complete(texture)
                textures.add(texture)
            } else {
                futureTexture.completeExceptionally(RuntimeException("Unable to load texture."))
            }
        }
        return futureTexture
    }
}