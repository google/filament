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

import java.awt.*;
import java.nio.IntBuffer;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

import javax.swing.JPanel;

import java.util.concurrent.ConcurrentLinkedDeque;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.nio.ByteBuffer;

import androidx.annotation.NonNull;


/**
 * A Swing friendly component which can be used if an AWT based FilamentCanvas object results in
 * issues due to interactions with Swing lightweight pop-ups and tooltips. However keep in mind
 * it carries a much higher cost of operation.
 */
public class FilamentPanel extends JPanel implements FilamentTarget {

    /**
     * A Slot contains a buffer for Filament to write and an Image for Swing to read.
     * It is ok for the buffer capacity to be > height * width. The image however MUST
     * be exactly of dimension height * width.
     */
    class Slot {

        IntBuffer buffer;
        BufferedImage image;

        Slot(int width, int height) {
            buffer = ByteBuffer.allocateDirect(4 * width * height).asIntBuffer();
            image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        }
    }

    /**
     * SlotPool is a circular buffer which provide "Slot"s where ExecutorService thread can write
     * pixels and Swing can read from. Its purpose it to reuse memory and avoid allocation. During
     * "normal" operation, SlotPool will generate no memory allocations. During a resize, it will
     * have to realloc for each frames if the surface gets bigger. If the new size of the frame is
     * smaller, only new BufferedImage are allocated.
     */
    private final static int NUM_SLOTS = 4;

    class SlotPool {
        private ConcurrentLinkedDeque<Slot> queue = new ConcurrentLinkedDeque<>();

        public SlotPool() {
            for (int i = 0; i < NUM_SLOTS; i++) {
                Slot slot = new Slot(1, 1); // Zero sized BufferedImage are forbidden.
                queue.add(slot);
            }
        }

        /**
         * Returns null if no more slots are available. Otherwise, returns a slot adequate to
         * receive and process an image of dimension width * height * RGBA.
         */
        public Slot getSlot(int width, int height) {
            Slot slot = queue.poll();

            // Sorry we are out.
            if (slot == null) {
                return slot;
            }

            // The slot can avoid an allocation and reuse the same BufferedImage if dimensions have
            // not changed.
            if (slot.image.getWidth() != width || slot.image.getHeight() != height) {
                slot.image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
            }

            // The slot can avoid an allocation and reuse the same IntBuffer if it has the capacity.
            if (slot.buffer.capacity() < width * height) {
                slot.buffer = ByteBuffer.allocateDirect(4 * width * height).asIntBuffer();
            } else {
                // Reuse the buffer, just reset position
                slot.buffer.rewind();
            }
            return slot;
        }

        public void putSlot(Slot slot) {
            queue.add(slot);
        }
    }

    private final SlotPool mSlotPool = new SlotPool();


    private NativeSurface mNativeSurface;
    private SwapChain mSwapChain;
    private BufferedImage mImage = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB);
    private final Object mImageLock = new Object();
    private int mHeight;
    private int mWidth;
    private Dimension mBackendDimension = new Dimension(0, 0);

    private static final ExecutorService sExecutor = Executors.newSingleThreadExecutor();

    // Amount of padding added to surface dimension upon allocating them.
    private static final int SURFACE_PADDING = 128;

    @Override
    public void paint(Graphics g) {
        synchronized (mImageLock) {
            g.drawImage(mImage, 0, 0, Color.BLACK, null);
        }
    }


    /**
     * This must be called on Filament thread.
     * Prepare to renders to offscreen native window.
     */
    public boolean beginFrame(@NonNull Engine engine, @NonNull Renderer renderer) {
        ensureSurface(engine);
        return renderer.beginFrame(mSwapChain);
    }


    /**
     * This must be called on Filament thread.
     * Pixels are read from the Gl (located in VRAM) and copied to the JVM (in RAM).
     * Colors are converted from ABGR to RGBA. A BufferedImage is created (copy RAM to RAM
     * of all pixels). Finally the image is drawn to screen via Graphics.drawImage (one more trip
     * from RAM to VRAM).
     */
    public void endFrame(@NonNull Renderer renderer) {
        // By the time readPixel callback is invoked, the dimension of the Panel may have changed.
        // Capture dimension so  they match the current GL framebuffer.
        int capturedWidth = mWidth;
        int capturedHeight = mHeight;

        // Get a slot where Filament can write its pixels and where we have an appropriately sized
        // BufferedImage.
        Slot slot = mSlotPool.getSlot(capturedWidth, capturedHeight);
        if (slot == null) {
            // It seems Filament is producing data faster than Swing can display it. Skip this frame
            // to allow Swing to catch up.
            renderer.endFrame();
            return;
        }

        Texture.PixelBufferDescriptor pb = new Texture.PixelBufferDescriptor(slot.buffer,
                Texture.Format.RGBA, Texture.Type.UBYTE, 1, 0, 0, 0, sExecutor, new Runnable() {
            @Override
            public void run() {
                synchronized (mImageLock) {
                    // Copy pixels read from the GL into the BufferedImage. Then convert from RGBA
                    // to ARGB because that is what the BufferedImage expects.
                    int[] data = ((DataBufferInt) slot.image.getRaster().getDataBuffer()).getData();
                    slot.buffer.get(data, 0, capturedHeight * capturedWidth);
                    for (int i = 0; i < capturedHeight * capturedWidth; i++) {
                        int rgba = data[i];
                        data[i] = (rgba >>> 8) | (rgba << 24);
                    }
                    mImage = slot.image;
                    repaint();
                    mSlotPool.putSlot(slot);
                }
            }
        });
        renderer.readPixels(0, 0, mWidth, mHeight, pb);
        renderer.endFrame();
    }

    private void createSurfaces(@NonNull Engine engine, int width, int height) {
        mBackendDimension = new Dimension(width, height);
        mNativeSurface = new NativeSurface(width, height);
        mSwapChain = engine.createSwapChainFromNativeSurface(mNativeSurface, 0L);
    }

    private void destroySurfaces(@NonNull Engine engine) {

        if (mNativeSurface == null) {
            return;
        }

        // Previous frames using this surface may still be in the pipeline, so we must wait for
        // them to finish.
        engine.flushAndWait();

        mNativeSurface.dispose();
        engine.destroySwapChain(mSwapChain);
    }

    private void ensureSurface(@NonNull Engine engine) {
        // Capture the dimension at the time the surface was ensured.
        mWidth = getWidth();
        mHeight = getHeight();

        if (mBackendDimension.width < mWidth || mBackendDimension.height < mHeight) {
            destroySurfaces(engine);

            // A surface needs to be allocated. Allocate something a little bit bigger than
            // necessary in order to avoid reallocating too often if the Panel is resized.
            int widthToAllocate = mWidth + SURFACE_PADDING;
            int heightToAllocate = mHeight + SURFACE_PADDING;
            createSurfaces(engine, widthToAllocate, heightToAllocate);
        }
    }

    /**
     * This must be called on Filament thread.
     */
    public void destroy(@NonNull Engine engine) {
        destroySurfaces(engine);
    }
}
