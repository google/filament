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
 * equirectangularToCubemap.destroy();
 *
 * specularFilter = new IBLPrefilterContext.SpecularFilter(context);
 * Texture reflections = specularFilter.run(skyboxTexture);
 * specularFilter.destroy();
 *
 * context.destroy();
 *
 * IndirectLight ibl = IndirectLight.Builder()
 *         .reflections(reflections)
 *         .intensity(30000.0f)
 *         .build(engine);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
public class IBLPrefilterContext {
    private long mNativeObject;

    public IBLPrefilterContext(Engine engine) {
        mNativeObject = nCreate(engine.getNativeObject());
        if (mNativeObject == 0) throw new IllegalStateException("Couldn't create IBLPrefilterContext");
    }

    public void destroy() {
        nDestroy(getNativeObject());
        mNativeObject = 0;
    }

    public static class EquirectangularToCubemap {
        private long mNativeObject;

        public EquirectangularToCubemap(IBLPrefilterContext context) {
            mNativeObject = nCreateEquirectHelper(context.getNativeObject());
        }

        public Texture run(Texture equirect) {
            long nativeTexture = nEquirectHelperRun(getNativeObject(), equirect.getNativeObject());
            return new Texture(nativeTexture);
        }

        public void destroy() {
            nDestroyEquirectHelper(getNativeObject());
            mNativeObject = 0;
        }

        protected long getNativeObject() {
            if (mNativeObject == 0) {
                throw new IllegalStateException("Calling method on destroyed EquirectangularToCubemap");
            }
            return mNativeObject;
        }
    }

    public static class SpecularFilter {
        private long mNativeObject;

        public SpecularFilter(IBLPrefilterContext context) {
            mNativeObject = nCreateSpecularFilter(context.getNativeObject());
        }

        public Texture run(Texture skybox) {
            long nativeTexture = nSpecularFilterRun(getNativeObject(), skybox.getNativeObject());
            return new Texture(nativeTexture);
        }

        public void destroy() {
            nDestroySpecularFilter(getNativeObject());
            mNativeObject = 0;
        }

        protected long getNativeObject() {
            if (mNativeObject == 0) {
                throw new IllegalStateException("Calling method on destroyed SpecularFilter");
            }
            return mNativeObject;
        }
    }


    protected long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed IBLPrefilterContext");
        }
        return mNativeObject;
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
