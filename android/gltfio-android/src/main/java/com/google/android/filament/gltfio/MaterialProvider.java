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

package com.google.android.filament.gltfio;

import androidx.annotation.NonNull;

import com.google.android.filament.Engine;
import com.google.android.filament.Material;
import com.google.android.filament.MaterialInstance;
import com.google.android.filament.proguard.UsedByNative;

import java.lang.reflect.Method;

/**
 * Interface to a provider of glTF materials.
 *
 * <p>This class is used by {@link AssetLoader} to create Filament materials.
 * Client applications do not need to call methods on it.</p>
 */
@UsedByNative("MaterialProvider.cpp")
public class MaterialProvider {

    public enum MaterialSource {
        GENERATE_SHADERS(0),
        LOAD_UBERSHADERS(1),
        EXTERNAL(2);

        private final long mNativeValue;

        MaterialSource(final long nativeValue) {
            mNativeValue = nativeValue;
        }

        long getNativeValue() {
            return mNativeValue;
        }

        public static MaterialSource fromNativeValue(final long nativeValue) {
            for (MaterialSource value : MaterialSource.values()) {
                if (value.getNativeValue() == nativeValue) {
                    return value;
                }
            }
            return null;
        }
    }

    public enum AlphaMode {
        OPAQUE(0),
        MASK(1),
        BLEND(2);

        private final long mNativeValue;

        AlphaMode(final long nativeValue) {
            mNativeValue = nativeValue;
        }

        public long getNativeValue() {
            return mNativeValue;
        }

        public static AlphaMode fromNativeValue(final long nativeValue) {
            for (AlphaMode value : AlphaMode.values()) {
                if (value.getNativeValue() == nativeValue) {
                    return value;
                }
            }
            return null;
        }
    }

    @UsedByNative("MaterialProvider.cpp")
    public static class MaterialConfig {
        // -- 32 bit boundary --
        private boolean mDoubleSided;
        private boolean mUnlit;
        private boolean mHasVertexColors;
        private boolean mHasBaseColorTexture;
        private boolean mHasNormalTexture;
        private boolean mHasOcclusionTexture;
        private boolean mHasEmissiveTexture;
        private boolean mUseSpecularGlossiness;
        private AlphaMode mAlphaMode;
        private boolean mEnableDiagnostics;
        private boolean mHasMetallicRoughnessTexture;
        private int mMetallicRoughnessUV;
        private boolean mHasSpecularGlossinessTexture;
        private int mSpecularGlossinessUV;
        private int mBaseColorUV;

        // -- 32 bit boundary --
        private boolean mHasClearCoatTexture;
        private int mClearCoatUV;
        private boolean mHasClearCoatRoughnessTexture;
        private int mClearCoatRoughnessUV;
        private boolean mHasClearCoatNormalTexture;
        private int mClearCoatNormalUV;
        private boolean mHasClearCoat;
        private boolean mHasTextureTransforms;

        // -- 32 bit boundary --
        private int mEmissiveUV;
        private int mAoUV;
        private int mNormalUV;
        private boolean mHasTransmissionTexture;
        private int mTransmissionUV;

        // -- 32 bit boundary --
        private boolean mHasSheenColorTexture;
        private int mSheenColorUV;
        private boolean mHasSheenRoughnessTexture;
        private int mSheenRoughnessUV;
        private boolean mHasSheen;

        @UsedByNative("MaterialProvider.cpp")
        public MaterialConfig(final long nativeValue0, final long nativeValue1) {
            // NOTE: This has to be in sync with MaterialKey in MaterialProvider.h

            // -- 32 bit boundary --
            mDoubleSided = ((nativeValue0 & 0x1) != 0);
            mUnlit = (((nativeValue0 >> 1) & 0x1) != 0);
            mHasVertexColors = (((nativeValue0 >> 2) & 0x1) != 0);
            mHasBaseColorTexture = (((nativeValue0 >> 3) & 0x1) != 0);
            mHasNormalTexture = (((nativeValue0 >> 4) & 0x1) != 0);
            mHasOcclusionTexture = (((nativeValue0 >> 5) & 0x1) != 0);
            mHasEmissiveTexture = (((nativeValue0 >> 6) & 0x1) != 0);
            mUseSpecularGlossiness = (((nativeValue0 >> 7) & 0x1) != 0);
            mAlphaMode = AlphaMode.fromNativeValue((nativeValue0 >> 8) & 0xF);
            mEnableDiagnostics = (((nativeValue0 >> 12) & 0xF) != 0);
            mHasMetallicRoughnessTexture = mHasSpecularGlossinessTexture = (((nativeValue0 >> 16) & 0x1) != 0);
            mMetallicRoughnessUV = mSpecularGlossinessUV = (int)((nativeValue0 >> 17) & 0x7F);
            mBaseColorUV = (int)((nativeValue0 >> 24) & 0xFF);

            // -- 32 bit boundary --
            mHasClearCoatTexture = (((nativeValue0 >> 32) & 0x1) != 0);
            mClearCoatUV = (int)((nativeValue0 >> 33) & 0x7F);
            mHasClearCoatRoughnessTexture = (((nativeValue0 >> 40) & 0x1) != 0);
            mClearCoatRoughnessUV = (int)((nativeValue0 >> 41) & 0x7F);
            mHasClearCoatNormalTexture = (((nativeValue0 >> 48) & 0x1) != 0);
            mClearCoatNormalUV = (int)((nativeValue0 >> 49) & 0x7F);
            mHasClearCoat = (((nativeValue0 >> 56) & 0x1) != 0);
            mHasTextureTransforms = (((nativeValue0 >> 57) & 0x7F) != 0);

            // -- 32 bit boundary --
            mEmissiveUV = (int)(nativeValue1 & 0xFF);
            mAoUV = (int)((nativeValue1 >> 8) & 0xFF);
            mNormalUV = (int)((nativeValue1 >> 16) & 0xFF);
            mHasTransmissionTexture = (((nativeValue1 >> 24) & 0x1) != 0);
            mTransmissionUV = (int)((nativeValue1 >> 25) & 0x7F);

            // -- 32 bit boundary --
            mHasSheenColorTexture = (((nativeValue1 >> 32) & 0x1) != 0);
            mSheenColorUV = (int)((nativeValue1 >> 33) & 0x7F);
            mHasSheenRoughnessTexture = (((nativeValue1 >> 40) & 0x1) != 0);
            mSheenRoughnessUV = (int)((nativeValue1 >> 41) & 0x7F);
            mHasSheen = (((nativeValue1 >> 48) & 0x1) != 0);
        }

        public boolean getDoubleSided() {
            return mDoubleSided;
        }

        public boolean getUnlit() {
            return mUnlit;
        }

        public boolean getHasVertexColors() {
            return mHasVertexColors;
        }

        public boolean getHasBaseColorTexture() {
            return mHasBaseColorTexture;
        }

        public boolean getHasNormalTexture() {
            return mHasNormalTexture;
        }

        public boolean getHasOcclusionTexture() {
            return mHasOcclusionTexture;
        }

        public boolean getHasEmissiveTexture() {
            return mHasEmissiveTexture;
        }

        public boolean getUseSpecularGlossiness() {
            return mUseSpecularGlossiness;
        }

        public AlphaMode getAlphaMode() {
            return mAlphaMode;
        }

        public boolean getEnableDiagnostics() {
            return mEnableDiagnostics;
        }

        public boolean getHasMetallicRoughnessTexture() {
            return mHasMetallicRoughnessTexture;
        }

        public int getMetallicRoughnessUV() {
            return mMetallicRoughnessUV;
        }

        public boolean getHasSpecularGlossinessTexture() {
            return mHasSpecularGlossinessTexture;
        }

        public int getSpecularGlossinessUV() {
            return mSpecularGlossinessUV;
        }

        public int getBaseColorUV() {
            return mBaseColorUV;
        }

        public boolean getHasClearCoatTexture() {
            return mHasClearCoatTexture;
        }

        public int getClearCoatUV() {
            return mClearCoatUV;
        }

        public boolean getHasClearCoatRoughnessTexture() {
            return mHasClearCoatRoughnessTexture;
        }

        public int getClearCoatRoughnessUV() {
            return mClearCoatRoughnessUV;
        }

        public boolean getHasClearCoatNormalTexture() {
            return mHasClearCoatNormalTexture;
        }

        public int getClearCoatNormalUV() {
            return mClearCoatNormalUV;
        }

        public boolean getHasClearCoat() {
            return mHasClearCoat;
        }

        public boolean getHasTextureTransforms() {
            return mHasTextureTransforms;
        }

        public int getEmissiveUV() {
            return mEmissiveUV;
        }

        public int getAoUV() {
            return mAoUV;
        }

        public int getNormalUV() {
            return mNormalUV;
        }

        public boolean getHasTransmissionTexture() {
            return mHasTransmissionTexture;
        }

        public int getTransmissionUV() {
            return mTransmissionUV;
        }

        public boolean getHasSheenColorTexture() {
            return mHasSheenColorTexture;
        }

        public int getSheenColorUV() {
            return mSheenColorUV;
        }

        public boolean getHasSheenRoughnessTexture() {
            return mHasSheenRoughnessTexture;
        }

        public int getSheenRoughnessUV() {
            return mSheenRoughnessUV;
        }

        public boolean getHasSheen() {
            return mHasSheen;
        }

    }

    public enum UvChannel {
        UNUSED(0),
        UV1(1),
        UV2(2);

        private final long mNativeValue;

        UvChannel(final long nativeValue) {
            mNativeValue = nativeValue;
        }

        public long getNativeValue() {
            return mNativeValue;
        }

        public static UvChannel fromNativeValue(final long nativeValue) {
            for (UvChannel value : UvChannel.values()) {
                if (value.getNativeValue() == nativeValue) {
                    return value;
                }
            }
            return null;
        }
    }

    @UsedByNative("MaterialProvider.cpp")
    public static class UvMap {
        private long mNativeValue;

        @UsedByNative("MaterialProvider.cpp")
        public UvMap(final long nativeValue) {
            mNativeValue = nativeValue;
        }

        public long getNativeValue() {
            return mNativeValue;
        }

        public UvChannel getChannel(Integer index) {
            if (index < 0 || index > 7) {
                throw new IndexOutOfBoundsException();
            }
            return UvChannel.fromNativeValue((mNativeValue >> (index * 8)) & 0xff);
        }

        public void setChannel(Integer index, UvChannel uvChannel) {
            if (index < 0 || index > 7) {
                throw new IndexOutOfBoundsException();
            }
            mNativeValue = (mNativeValue & ~(0xff << (index * 8))) | (uvChannel.getNativeValue() << (index * 8));
        }
    }

    @UsedByNative("MaterialProvider.cpp")
    public interface ExternalSource {
        @UsedByNative("MaterialProvider.cpp")
        Material resolveMaterial(@NonNull MaterialConfig materialConfig, @NonNull UvMap uvMap, @NonNull String name);

        @UsedByNative("MaterialProvider.cpp")
        MaterialInstance instantiateMaterial(@NonNull Material material, @NonNull MaterialConfig materialConfig, @NonNull UvMap uvMap);
    }

    private long mNativeObject;

    /**
     * Constructs a specified material provider using the supplied {@link Engine}.
     *
     * @param engine the engine used to create materials
     * @param materialSource material source
     * @param externalSource external source
     */
    public MaterialProvider(Engine engine, MaterialSource materialSource, ExternalSource externalSource) {
        if (materialSource == MaterialSource.GENERATE_SHADERS) {
            throw new IllegalArgumentException("Shaders generation not available on Android");
        }
        if (materialSource == MaterialSource.EXTERNAL && externalSource == null) {
            throw new IllegalArgumentException("MaterialSource.EXTERNAL requires an instance of ExternalSource");
        }

        long nativeEngine = engine.getNativeObject();
        long nativeMaterialSource = materialSource.getNativeValue();
        mNativeObject = nCreateMaterialProvider(nativeEngine, nativeMaterialSource, externalSource);
    }

    /**
     * Constructs a specified material provider using the supplied {@link Engine}.
     *
     * @param engine the engine used to create materials
     * @param materialSource material source
     */
    public MaterialProvider(Engine engine, MaterialSource materialSource) {
        this(engine, materialSource, null);
    }

    /**
     * Constructs an ubershader loader using the supplied {@link Engine}.
     *
     * @param engine the engine used to create materials
     */
    public MaterialProvider(Engine engine) {
        this(engine, MaterialSource.LOAD_UBERSHADERS);
    }

    /**
     * Frees memory associated with the native material provider.
     * */
    public void destroy() {
        nDestroyMaterialProvider(mNativeObject);
        mNativeObject = 0;
    }

    /**
     * Destroys all cached materials.
     *
     * This is not called automatically when MaterialProvider is destroyed, which allows
     * clients to take ownership of the cache if desired.
     */
    public void destroyMaterials() {
        nDestroyMaterials(mNativeObject);
    }

    long getNativeObject() {
        return mNativeObject;
    }

    @UsedByNative("MaterialProvider.cpp")
    private static Material newMaterial(long nativeObject) {
        return new Material(nativeObject);
    }

    @UsedByNative("MaterialProvider.cpp")
    private static long getMaterialNativeObject(Material material) {
        return material.getNativeObject();
    }

    @UsedByNative("MaterialProvider.cpp")
    private static long getMaterialInstanceNativeObject(MaterialInstance materialInstance) {
        return materialInstance.getNativeObject();
    }

    private static native long nCreateMaterialProvider(long nativeEngine, long nativeMaterialSource, ExternalSource externalSource);
    private static native void nDestroyMaterialProvider(long nativeProvider);
    private static native void nDestroyMaterials(long nativeProvider);
}
