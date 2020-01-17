/*
 * Copyright (C) 2017 The Android Open Source Project
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

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;

import com.google.android.filament.proguard.UsedByReflection;

/**
 * Skybox
 * <p>When added to a {@link Scene}, the <code>Skybox</code> fills all untouched pixels.</p>
 *
 * <h1>Creation and destruction</h1>
 *
 * A <code>Skybox</code> object is created using the {@link Skybox.Builder} and destroyed by calling
 * {@link Engine#destroySkybox}.<br>
 * <pre>
 *  Engine engine = Engine.create();
 *
 *  Scene scene = engine.createScene();
 *
 *  Skybox skybox = new Skybox.Builder()
 *              .environment(cubemap)
 *              .build(engine);
 *
 *  scene.setSkybox(skybox);
 * </pre>
 *
 * Currently only {@link Texture} based sky boxes are supported.
 *
 * @see Scene
 * @see IndirectLight
 */
public class Skybox {
    private long mNativeObject;

    public Skybox(long nativeSkybox) {
        mNativeObject = nativeSkybox;
    }


    /**
     * Use <code>Builder</code> to construct a <code>Skybox</code> object instance.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Use <code>Builder</code> to construct a <code>Skybox</code> object instance.
         */
        public Builder() {
            mNativeBuilder = nCreateBuilder();
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Set the environment map (i.e. the skybox content).
         *
         * <p>The <code>Skybox</code> is rendered as though it were an infinitely large cube with the
         * camera inside it. This means that the cubemap which is mapped onto the cube's exterior
         * will appear mirrored. This follows the OpenGL conventions.</p>
         *
         * <p>The <code>cmgen</code> tool generates reflection maps by default which are therefore
         * ideal to use as skyboxes.</p>
         *
         * @param cubemap A cubemap {@link Texture}
         *
         * @return This Builder, for chaining calls.
         *
         * @see Texture
         */
        @NonNull
        public Builder environment(@NonNull Texture cubemap) {
            nBuilderEnvironment(mNativeBuilder, cubemap.getNativeObject());
            return this;
        }

        /**
         * Indicates whether the sun should be rendered. The sun can only be
         * rendered if there is at least one light of type {@link LightManager.Type#SUN} in
         * the {@link Scene}. The default value is <code>false</code>.
         *
         * @param show <code>true</code> if the sun should be rendered, <code>false</code> otherwise
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder showSun(boolean show) {
            nBuilderShowSun(mNativeBuilder, show);
            return this;
        }

        /**
         * Sets the <code>Skybox</code> intensity when no {@link IndirectLight} is set
         *
         * <p>This call is ignored when an  {@link IndirectLight} is set, otherwise it is used in
         * its place.</p>
         *
         * @param envIntensity  Scale factor applied to the skybox texel values such that
         *                      the result is in cd/m<sup>2</sup> (lux) units (default = 30000)
         *
         * @return This Builder, for chaining calls.
         *
         * @see IndirectLight.Builder#intensity
         */
        @NonNull
        public Builder intensity(float envIntensity) {
            nBuilderIntensity(mNativeBuilder, envIntensity);
            return this;
        }

        /**
         * Creates a <code>Skybox</code> object
         *
         * @param engine the {@link Engine} to associate this <code>Skybox</code> with.
         *
         * @return A newly created <code>Skybox</code>object
         *
         * @exception IllegalStateException can be thrown if the  <code>Skybox</code> couldn't be created
         */
        @NonNull
        public Skybox build(@NonNull Engine engine) {
            long nativeSkybox = nBuilderBuild(mNativeBuilder, engine.getNativeObject());
            if (nativeSkybox == 0) throw new IllegalStateException("Couldn't create Skybox");
            return new Skybox(nativeSkybox);
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;

            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }

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

    /**
     * Sets bits in a visibility mask. By default, this is <code>0x1</code>.
     * <p>This provides a simple mechanism for hiding or showing this <code>Skybox</code> in a
     * {@link Scene}.</p>
     *
     * <p>For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits unaffected,
     * call: <code>setLayerMask(7, 2)</code>.</p>
     *
     * @param select the set of bits to affect
     * @param values the replacement values for the affected bits
     *
     * @see View#setVisibleLayers
     */
    public void setLayerMask(@IntRange(from = 0, to = 255) int select, @IntRange(from = 0, to = 255) int values) {
        nSetLayerMask(getNativeObject(), select & 0xff, values & 0xff);
    }

    /**
     * @return the visibility mask bits
     */
    public int getLayerMask() {
        return nGetLayerMask(getNativeObject());
    }

    /**
     * Returns the <code>Skybox</code>'s intensity in cd/m<sup>2</sup>.
     */
    public float getIntensity() { return nGetIntensity(getNativeObject()); }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Skybox");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeSkyboxBuilder);
    private static native void nBuilderEnvironment(long nativeSkyboxBuilder, long nativeTexture);
    private static native void nBuilderShowSun(long nativeSkyboxBuilder, boolean show);
    private static native void nBuilderIntensity(long nativeSkyboxBuilder, float intensity);
    private static native long nBuilderBuild(long nativeSkyboxBuilder, long nativeEngine);
    private static native void nSetLayerMask(long nativeSkybox, int select, int value);
    private static native int  nGetLayerMask(long nativeSkybox);
    private static native float nGetIntensity(long nativeSkybox);
}
