/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.google.android.filament.utils;

import com.google.android.filament.Engine;
import com.google.android.filament.Texture;

/**
 * IBLPrefilterContext creates and initializes GPU state common to all environment map filters
 * supported. Typically, only one instance per filament Engine of this object needs to exist.
 *
 * Java usage example:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * context = new IBLPrefilterContext(engine);
 * equirectangularToCubemap = new IBLPrefilterContext.EquirectangularToCubemap(context);
 *
 * Texture equirect = HDRLoader.createTexture("foo.hdr");
 * Texture skyboxTexture = equirectangularToCubemap.run(equirect);
 * engine.destroy(equirect);
 *
 * specularFilter = new IBLPrefilterContext.SpecularFilter(context);
 * Texture reflections = specularFilter.run(skyboxTexture);
 *
 * IndirectLight ibl = IndirectLight.Builder()
 *         .reflections(reflections)
 *         .intensity(30000.0f)
 *         .build(engine);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
public class IBLPrefilterContext {
    private final long mNativeObject;

    public IBLPrefilterContext(Engine engine) {
        mNativeObject = nCreate(engine.getNativeObject());
        if (mNativeObject == 0) throw new IllegalStateException("Couldn't create IBLPrefilterContext");
    }

    @Override
    protected void finalize() throws Throwable {
        nDestroy(mNativeObject);
        super.finalize();
    }

    public static class EquirectangularToCubemap {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final HelperFinalizer mFinalizer;
        private final long mNativeHelper;

        public EquirectangularToCubemap(IBLPrefilterContext context) {
            mNativeHelper = nCreateEquirectHelper(context.mNativeObject);
            mFinalizer = new HelperFinalizer(mNativeHelper);
        }

        public Texture run(Texture equirect) {
            long nativeTexture = nEquirectHelperRun(mNativeHelper, equirect.getNativeObject());
            return new Texture(nativeTexture);
        }

        private static class HelperFinalizer {
            private final long mNativeObject;

            HelperFinalizer(long nativeObject) { mNativeObject = nativeObject; }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyEquirectHelper(mNativeObject);
                }
            }
        }
    }

    public static class SpecularFilter {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final HelperFinalizer mFinalizer;
        private final long mNativeHelper;

        public SpecularFilter(IBLPrefilterContext context) {
            mNativeHelper = nCreateSpecularFilter(context.mNativeObject);
            mFinalizer = new HelperFinalizer(mNativeHelper);
        }

        public Texture run(Texture skybox) {
            long nativeTexture = nSpecularFilterRun(mNativeHelper, skybox.getNativeObject());
            return new Texture(nativeTexture);
        }

        private static class HelperFinalizer {
            private final long mNativeObject;

            HelperFinalizer(long nativeObject) { mNativeObject = nativeObject; }

            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroySpecularFilter(mNativeObject);
                }
            }
        }
    }

    private static native long nCreate(long nativeEngine);
    private static native void nDestroy(long nativeObject);

    private static native long nCreateEquirectHelper(long nativeContext);
    private static native long nEquirectHelperRun(long nativeHelper, long nativeEquirect);
    private static native void nDestroyEquirectHelper(long nativeObject);

    private static native long nCreateSpecularFilter(long nativeContext);
    private static native long nSpecularFilterRun(long nativeHelper, long nativeSkybox);
    private static native void nDestroySpecularFilter(long nativeObject);
}
