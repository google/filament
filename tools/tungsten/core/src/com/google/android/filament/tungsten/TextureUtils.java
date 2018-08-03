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

package com.google.android.filament.tungsten;

import com.google.android.filament.Engine;
import com.google.android.filament.Texture;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class TextureUtils {

    private static Texture loadTexture(Engine engine, String path, Texture.InternalFormat format) {
        BufferedImage img = null;
        try {
            img = ImageIO.read(new File(path));
        } catch (IOException e) {
            e.printStackTrace();
            System.err.println("Could not read image file from disk");
            System.exit(1);
        }
        img.getType();
        DataBufferByte data = (DataBufferByte) img.getData().getDataBuffer();
        byte[] pixels = data.getData();

        // ABGR -> RGBA
        assert (pixels.length % 4 == 0);
        for (int i = 0; i < pixels.length; i += 4) {
            byte A = pixels[i];
            byte B = pixels[i+1];
            byte G = pixels[i+2];
            byte R = pixels[i+3];
            pixels[i] = R;
            pixels[i+1] = G;
            pixels[i+2] = B;
            pixels[i+3] = A;
        }

        Texture texture = new Texture.Builder()
                .width(img.getWidth())
                .height(img.getHeight())
                .format(format)
                .sampler(Texture.Sampler.SAMPLER_2D)
                .build(engine);

        ByteBuffer buf = ByteBuffer.wrap(pixels);
        buf.order(ByteOrder.BIG_ENDIAN);

        Texture.PixelBufferDescriptor desc = new Texture.PixelBufferDescriptor(
                buf, Texture.Format.RGBA, Texture.Type.UBYTE);

        texture.setImage(engine, Texture.BASE_LEVEL, desc);

        return texture;
    }
}
