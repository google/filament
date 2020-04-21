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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

public class LightManager {
    private long mNativeObject;

    LightManager(long nativeLightManager) {
        mNativeObject = nativeLightManager;
    }

    public int getComponentCount() {
        return nGetComponentCount(mNativeObject);
    }

    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    public enum Type {
        SUN,
        DIRECTIONAL,
        POINT,
        FOCUSED_SPOT,
        SPOT
    }

    public static class ShadowOptions {
        public int mapSize = 1024;
        public float constantBias = 0.05f;
        public float normalBias = 0.4f;
        public float shadowFar = 0.0f;
        public float shadowNearHint = 1.0f;
        public float shadowFarHint = 100.0f;
        public boolean stable = true;
        public boolean screenSpaceContactShadows = false;
        public int stepCount = 8;
        public float maxShadowDistance = 0.3f;
    }

    public static final float EFFICIENCY_INCANDESCENT = 0.0220f;
    public static final float EFFICIENCY_HALOGEN      = 0.0707f;
    public static final float EFFICIENCY_FLUORESCENT  = 0.0878f;
    public static final float EFFICIENCY_LED          = 0.1171f;

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder(@NonNull Type type) {
            mNativeBuilder = nCreateBuilder(type.ordinal());
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder castShadows(boolean enable) {
            nBuilderCastShadows(mNativeBuilder, enable);
            return this;
        }

        @NonNull
        public Builder shadowOptions(@NonNull ShadowOptions options) {
            nBuilderShadowOptions(mNativeBuilder,
                    options.mapSize, options.constantBias, options.normalBias, options.shadowFar,
                    options.shadowNearHint, options.shadowFarHint, options.stable,
                    options.screenSpaceContactShadows, options.stepCount, options.maxShadowDistance);
            return this;
        }

        @NonNull
        public Builder castLight(boolean enabled) {
            nBuilderCastLight(mNativeBuilder, enabled);
            return this;
        }

        @NonNull
        public Builder position(float x, float y, float z) {
            nBuilderPosition(mNativeBuilder, x, y, z);
            return this;
        }

        @NonNull
        public Builder direction(float x, float y, float z) {
            nBuilderDirection(mNativeBuilder, x, y, z);
            return this;
        }

        @NonNull
        public Builder color(float linearR, float linearG, float linearB) {
            nBuilderColor(mNativeBuilder, linearR, linearG, linearB);
            return this;
        }

        @NonNull
        public Builder intensity(float intensity) {
            nBuilderIntensity(mNativeBuilder, intensity);
            return this;
        }

        @NonNull
        public Builder intensity(float watts, float efficiency) {
            nBuilderIntensity(mNativeBuilder, watts, efficiency);
            return this;
        }

        @NonNull
        public Builder falloff(float radius) {
            nBuilderFalloff(mNativeBuilder, radius);
            return this;
        }

        @NonNull
        public Builder spotLightCone(float inner, float outer) {
            nBuilderSpotLightCone(mNativeBuilder, inner, outer);
            return this;
        }

        @NonNull
        public Builder sunAngularRadius(float angularRadius) {
            nBuilderAngularRadius(mNativeBuilder, angularRadius);
            return this;
        }

        @NonNull
        public Builder sunHaloSize(float haloSize) {
            nBuilderHaloSize(mNativeBuilder, haloSize);
            return this;
        }

        @NonNull
        public Builder sunHaloFalloff(float haloFalloff) {
            nBuilderHaloFalloff(mNativeBuilder, haloFalloff);
            return this;
        }

        public void build(@NonNull Engine engine, @Entity int entity) {
            if (!nBuilderBuild(mNativeBuilder, engine.getNativeObject(), entity)) {
                throw new IllegalStateException(
                    "Couldn't create Light component for entity " + entity + ", see log.");
            }
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

    @NonNull
    public Type getType(@EntityInstance int i) {
        return Type.values()[nGetType(mNativeObject, i)];
    }

    public void setPosition(@EntityInstance int i, float x, float y, float z) {
        nSetPosition(mNativeObject, i, x, y, z);
    }

    @NonNull
    public float[] getPosition(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetPosition(mNativeObject, i, out);
        return out;
    }

    public void setDirection(@EntityInstance int i, float x, float y, float z) {
        nSetDirection(mNativeObject, i, x, y, z);
    }

    @NonNull
    public float[] getDirection(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetDirection(mNativeObject, i, out);
        return out;
    }

    public void setColor(@EntityInstance int i, float linearR, float linearG, float linearB) {
        nSetColor(mNativeObject, i, linearR, linearG, linearB);
    }

    @NonNull
    public float[] getColor(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetColor(mNativeObject, i, out);
        return out;
    }

    public void setIntensity(@EntityInstance int i, float intensity) {
        nSetIntensity(mNativeObject, i , intensity);
    }

    public void setIntensity(@EntityInstance int i, float watts, float efficiency) {
        nSetIntensity(mNativeObject, i , watts, efficiency);
    }

    public float getIntensity(@EntityInstance int i) {
        return nGetIntensity(mNativeObject, i);
    }

    public void setFalloff(@EntityInstance int i, float falloff) {
        nSetFalloff(mNativeObject, i, falloff);
    }

    public float getFalloff(@EntityInstance int i) {
        return nGetFalloff(mNativeObject, i);
    }

    public void setSpotLightCone(@EntityInstance int i, float inner, float outer) {
        nSetSpotLightCone(mNativeObject, i, inner, outer);
    }

    public void setSunAngularRadius(@EntityInstance int i, float angularRadius) {
        nSetSunAngularRadius(mNativeObject, i, angularRadius);
    }

    public float getSunAngularRadius(@EntityInstance int i) {
        return nGetSunAngularRadius(mNativeObject, i);
    }

    public void setSunHaloSize(@EntityInstance int i, float haloSize) {
        nSetSunHaloSize(mNativeObject, i, haloSize);
    }

    public float getSunHaloSize(@EntityInstance int i) {
        return nGetSunHaloSize(mNativeObject, i);
    }

    public void setSunHaloFalloff(@EntityInstance int i, float haloFalloff) {
        nSetSunHaloFalloff(mNativeObject, i, haloFalloff);
    }

    public float getSunHaloFalloff(@EntityInstance int i) {
        return nGetSunHaloFalloff(mNativeObject, i);
    }

    public void setShadowCaster(@EntityInstance int i, boolean shadowCaster) {
        nSetShadowCaster(mNativeObject, i, shadowCaster);
    }

    public boolean isShadowCaster(@EntityInstance int i) {
        return nIsShadowCaster(mNativeObject, i);
    }

    public long getNativeObject() {
        return mNativeObject;
    }

    private static native int nGetComponentCount(long nativeLightManager);
    private static native boolean nHasComponent(long nativeLightManager, int entity);
    private static native int nGetInstance(long nativeLightManager, int entity);
    private static native void nDestroy(long nativeLightManager, int entity);

    private static native long nCreateBuilder(int lightType);
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native boolean nBuilderBuild(long nativeBuilder, long nativeEngine, int entity);
    private static native void nBuilderCastShadows(long nativeBuilder, boolean enable);
    private static native void nBuilderShadowOptions(long nativeBuilder, int mapSize, float constantBias, float normalBias, float shadowFar, float shadowNearHint, float shadowFarhint, boolean stable, boolean screenSpaceContactShadows, int stepCount, float maxShadowDistance);
    private static native void nBuilderCastLight(long nativeBuilder, boolean enabled);
    private static native void nBuilderPosition(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderDirection(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderColor(long nativeBuilder, float linearR, float linearG, float linearB);
    private static native void nBuilderIntensity(long nativeBuilder, float intensity);
    private static native void nBuilderIntensity(long nativeBuilder, float watts, float efficiency);
    private static native void nBuilderFalloff(long nativeBuilder, float radius);
    private static native void nBuilderSpotLightCone(long nativeBuilder, float inner, float outer);
    private static native void nBuilderAngularRadius(long nativeBuilder, float angularRadius);
    private static native void nBuilderHaloSize(long nativeBuilder, float haloSize);
    private static native void nBuilderHaloFalloff(long nativeBuilder, float haloFalloff);

    private static native int nGetType(long nativeLightManager, int i);
    private static native void nSetPosition(long nativeLightManager, int i, float x, float y, float z);
    private static native void nGetPosition(long nativeLightManager, int i, float[] out);
    private static native void nSetDirection(long nativeLightManager, int i, float x, float y, float z);
    private static native void nGetDirection(long nativeLightManager, int i, float[] out);
    private static native void nSetColor(long nativeLightManager, int i, float linearR, float linearG, float linearB);
    private static native void nGetColor(long nativeLightManager, int i, float[] out);
    private static native void nSetIntensity(long nativeLightManager, int i, float intensity);
    private static native void nSetIntensity(long nativeLightManager, int i, float watts, float efficiency);
    private static native float nGetIntensity(long nativeLightManager, int i);
    private static native void nSetFalloff(long nativeLightManager, int i, float falloff);
    private static native float nGetFalloff(long nativeLightManager, int i);
    private static native void nSetSpotLightCone(long nativeLightManager, int i, float inner, float outer);
    private static native void nSetSunAngularRadius(long nativeLightManager, int i, float angularRadius);
    private static native float nGetSunAngularRadius(long nativeLightManager, int i);
    private static native void nSetSunHaloSize(long nativeLightManager, int i, float haloSize);
    private static native float nGetSunHaloSize(long nativeLightManager, int i);
    private static native void nSetSunHaloFalloff(long nativeLightManager, int i, float haloFalloff);
    private static native float nGetSunHaloFalloff(long nativeLightManager, int i);
    private static native void nSetShadowCaster(long nativeLightManager, int i, boolean shadowCaster);
    private static native boolean nIsShadowCaster(long nativeLightManager, int i);
}
