/*
 * Copyright (C) 2020 The Android Open Source Project
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

package com.google.android.filament.pagecurl;

import android.annotation.SuppressLint;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.util.Log;

import com.google.android.filament.Engine;
import com.google.android.filament.IndirectLight;
import com.google.android.filament.Texture;
import com.google.android.filament.Texture.Format;
import com.google.android.filament.Texture.InternalFormat;
import com.google.android.filament.Texture.PixelBufferDescriptor;
import com.google.android.filament.Texture.Sampler;
import com.google.android.filament.Texture.Type;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

@SuppressWarnings("WeakerAccess")
public final class IblLoader {
    public static IndirectLight loadIndirectLight(AssetManager assets, String name, Engine engine) {
        final int[] size = new int[2];
        peekSize(assets, name + "/m0_nx.rgb32f", size);

        final int levels = 1 + (int) (Math.log(size[0]) / Math.log(2));

        @SuppressLint("Range")
        Texture texture = new Texture.Builder()
                .width(size[0])
                .height(size[1])
                .levels(levels)
                .format(InternalFormat.R11F_G11F_B10F)
                .sampler(Sampler.SAMPLER_CUBEMAP)
                .build(engine);

        try {
            for(int i = 0; i < levels; ++i) {
                if (!loadCubemap(texture, assets, name, engine, "" + 'm' + i + '_', i)) {
                    Log.e("Filament", "Unable to load cubemap.");
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return new com.google.android.filament.IndirectLight.Builder()
            .reflections(texture)
            .intensity(40_000)
            .build(engine);
    }

    private static boolean loadCubemap(Texture texture, AssetManager assets, String name,
            Engine engine, String prefix, int level) throws IOException {
        Options opts = new Options();
        opts.inPremultiplied = false;
        final int faceSize = texture.getWidth(level) * texture.getHeight(level) * 4;

        final int[] offsets = new int[6];
        for(int i = 0; i < 6; ++i) {
            offsets[i] = i * faceSize;
        }

        final String[] suffixes = new String[]{"px", "nx", "py", "ny", "pz", "nz"};

        ByteBuffer storage = ByteBuffer.allocateDirect(faceSize * 6);
        for (String suffix : suffixes) {
            InputStream is = assets.open(name + '/' + prefix + suffix + ".rgb32f");
            Bitmap bitmap = BitmapFactory.decodeStream(is, null, opts);
            if (bitmap != null) {
                bitmap.copyPixelsToBuffer(storage);
            }
        }
        storage.flip();

        PixelBufferDescriptor buffer = new PixelBufferDescriptor(storage, Format.RGB,
                Type.UINT_10F_11F_11F_REV);
        texture.setImage(engine, level, buffer, offsets);
        return true;
    }

    private static void peekSize(AssetManager assets, String name, int[] size) {
        InputStream input = null;
        try {
            input = assets.open(name);
        } catch (IOException e) {
            Log.e("Filament", "Unable to peek at cubemap: " + name);
            e.printStackTrace();
        }
        Options options = new Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(input, null, options);
        size[0] = options.outWidth;
        size[1] = options.outHeight;
    }
}
