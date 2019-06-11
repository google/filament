/*
 * Copyright (C) 2019 The Android Open Source Project
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

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.nio.Buffer;
import java.nio.BufferOverflowException;

public class RenderTarget {
    private long mNativeObject;
    private final Texture[] mTextures = new Texture[2];

    private RenderTarget(long nativeRenderTarget, Builder builder) {
        mNativeObject = nativeRenderTarget;
        mTextures[0] = builder.mTextures[0];
        mTextures[1] = builder.mTextures[1];
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed RenderTarget");
        }
        return mNativeObject;
    }

    public enum AttachmentPoint {
        COLOR,
        DEPTH,
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;
        private final Texture[] mTextures = new Texture[2];

        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder texture(@NonNull AttachmentPoint attachment, @Nullable Texture texture) {
            mTextures[attachment.ordinal()] = texture;
            nBuilderTexture(mNativeBuilder, attachment.ordinal(), texture != null ? texture.getNativeObject() : 0);
            return this;
        }

        @NonNull
        public Builder mipLevel(@NonNull AttachmentPoint attachment, @IntRange(from = 0) int level) {
            nBuilderMipLevel(mNativeBuilder, attachment.ordinal(), level);
            return this;
        }

        @NonNull
        public Builder face(@NonNull AttachmentPoint attachment, Texture.CubemapFace face) {
            nBuilderFace(mNativeBuilder, attachment.ordinal(), face.ordinal());
            return this;
        }

        @NonNull
        public Builder layer(@NonNull AttachmentPoint attachment, @IntRange(from = 0) int layer) {
            nBuilderLayer(mNativeBuilder, attachment.ordinal(), layer);
            return this;
        }

        @NonNull
        public RenderTarget build(@NonNull Engine engine) {
            long nativeRenderTarget = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeRenderTarget == 0)
                throw new IllegalStateException("Couldn't create RenderTarget");
            return new RenderTarget(nativeRenderTarget, this);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) {
                mNativeObject = nativeObject;
            }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }

    @Nullable
    public Texture getTexture(@NonNull AttachmentPoint attachment) {
        return mTextures[attachment.ordinal()];
    }

    @IntRange(from = 0)
    public int getMipLevel(@NonNull AttachmentPoint attachment) {
        return nGetMipLevel(getNativeObject(), attachment.ordinal());
    }

    public Texture.CubemapFace getFace(AttachmentPoint attachment) {
        return Texture.CubemapFace.values()[nGetFace(getNativeObject(), attachment.ordinal())];
    }

    @IntRange(from = 0)
    public int getLayer(@NonNull AttachmentPoint attachment) {
        return nGetLayer(getNativeObject(), attachment.ordinal());
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native long nDestroyBuilder(long nativeBuilder);
    private static native long nBuilderTexture(long nativeBuilder, int attachment, long nativeTexture);
    private static native long nBuilderMipLevel(long nativeBuilder, int attachment, int level);
    private static native long nBuilderFace(long nativeBuilder, int attachment, int face);
    private static native long nBuilderLayer(long nativeBuilder, int attachment, int layer);
    private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);

    private static native int nGetMipLevel(long nativeRenderTarget, int attachment);
    private static native int nGetFace(long nativeRenderTarget, int attachment);
    private static native int nGetLayer(long nativeRenderTarget, int attachment);
}
