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
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import com.google.android.filament.proguard.UsedByNative;
import com.google.android.filament.Engine.FeatureLevel;

import java.nio.Buffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

/**
 * A Filament Material defines the visual appearance of an object. Materials function as a
 * templates from which {@link MaterialInstance}s can be spawned. Use {@link Builder} to construct
 * a Material object.
 *
 * @see <a href="https://google.github.io/filament/Materials.html">Filament Materials Guide</a>
 */
@UsedByNative("AssetLoader.cpp")
public class Material {
    static final class EnumCache {
        private EnumCache() { }

        static final Shading[] sShadingValues = Shading.values();
        static final Interpolation[] sInterpolationValues = Interpolation.values();
        static final BlendingMode[] sBlendingModeValues = BlendingMode.values();
        static final RefractionMode[] sRefractionModeValues = RefractionMode.values();
        static final RefractionType[] sRefractionTypeValues = RefractionType.values();
        static final ReflectionMode[] sReflectionModeValues = ReflectionMode.values();
        static final FeatureLevel[] sFeatureLevelValues = FeatureLevel.values();
        static final VertexDomain[] sVertexDomainValues = VertexDomain.values();
        static final CullingMode[] sCullingModeValues = CullingMode.values();
        static final VertexBuffer.VertexAttribute[] sVertexAttributeValues =
                VertexBuffer.VertexAttribute.values();
    }

    private long mNativeObject;
    private final MaterialInstance mDefaultInstance;

    private Set<VertexBuffer.VertexAttribute> mRequiredAttributes;

    /** Supported shading models */
    public enum Shading {
        /**
         * No lighting applied, emissive possible
         *
         * @see
         * <a href="https://google.github.io/filament/Materials.html#materialmodels/unlitmodel">
         * Unlit model</a>
         */
        UNLIT,

        /**
         * Default, standard lighting
         *
         * @see
         * <a href="https://google.github.io/filament/Materials.html#materialmodels/litmodel">
         * Lit model</a>
         */
        LIT,

        /**
         * Subsurface lighting model
         *
         * @see
         * <a href="https://google.github.io/filament/Materials.html#materialmodels/subsurfacemodel">
         * Subsurface model</a>
         */
        SUBSURFACE,

        /**
         * Cloth lighting model
         *
         * @see
         * <a href="https://google.github.io/filament/Materials.html#materialmodels/clothmodel">
         * Cloth model</a>
         */
        CLOTH,

        /**
         * Legacy lighting model
         *
         * @see
         * <a href="https://google.github.io/filament/Materials.html#materialmodels/specularglossiness">
         * Specular glossiness</a>
         */
        SPECULAR_GLOSSINESS
    }

    /**
     * Attribute interpolation types in the fragment shader
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:interpolation">
     * Vertex and attributes: interpolation</a>
     */
    public enum Interpolation {
        /** Default, smooth interpolation */
        SMOOTH,

        /** Flat interpolation */
        FLAT
    }

    /**
     * Supported blending modes
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:blending">
     * Blending and transparency: blending</a>
     */
    public enum BlendingMode {
        /** Material is opaque. */
        OPAQUE,

        /**
         * Material is transparent and color is alpha-pre-multiplied.
         * Affects diffuse lighting only.
         */
        TRANSPARENT,

        /** Material is additive (e.g.: hologram). */
        ADD,

        /** Material is masked (i.e. alpha tested). */
        MASKED,

        /**
         * Material is transparent and color is alpha-pre-multiplied.
         * Affects specular lighting.
         */
        FADE,

        /** Material darkens what's behind it. */
        MULTIPLY,

        /** Material brightens what's behind it. */
        SCREEN,
    }

    /**
     * Supported refraction modes
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:refraction">
     * Blending and transparency: refractionMode</a>
     */
    public enum RefractionMode {
        NONE,
        CUBEMAP,
        SCREEN_SPACE
    }

    /**
     * Supported refraction types
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:refractiontype">
     * Blending and transparency: refractionType</a>
     */
    public enum RefractionType {
        SOLID,
        THIN
    }

    /**
     * Supported reflection modes
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/lighting:reflections">
     * Lighting: reflections</a>
     */
    public enum ReflectionMode {
        DEFAULT,
        SCREEN_SPACE
    }

    /**
     * Supported types of vertex domains
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:vertexdomain">
     * Vertex and attributes: vertexDomain</a>
     */
    public enum VertexDomain {
        /** Vertices are in object space, default. */
        OBJECT,

        /** Vertices are in world space. */
        WORLD,

        /** Vertices are in view space. */
        VIEW,

        /** Vertices are in normalized device space. */
        DEVICE
    }

    /**
     * Face culling Mode
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling">
     * Rasterization: culling</a>
     */
    public enum CullingMode {
        /** No culling. Front and back faces are visible. */
        NONE,

        /** Front face culling. Only back faces are visible. */
        FRONT,

        /** Back face culling. Only front faces are visible. */
        BACK,

        /** Front and back culling. Geometry is not visible. */
        FRONT_AND_BACK
    }

    public enum CompilerPriorityQueue {
        HIGH,
        LOW
    }

    public static class UserVariantFilterBit {
        /** Directional lighting */
        public static int DIRECTIONAL_LIGHTING = 0x01;
        /** Dynamic lighting */
        public static int DYNAMIC_LIGHTING = 0x02;
        /** Shadow receiver */
        public static int SHADOW_RECEIVER = 0x04;
        /** Skinning */
        public static int SKINNING = 0x08;
        /** Fog */
        public static int FOG = 0x10;
        /** Variance shadow maps */
        public static int VSM = 0x20;
        /** Screen-space reflections */
        public static int SSR = 0x40;
        /** Instanced stereo rendering */
        public static int STE = 0x80;
        public static int ALL = 0xFF;
    }

    @UsedByNative("Material.cpp")
    public static class Parameter {
        private static final Type[] sTypeValues = Type.values();

        public enum Type {
            BOOL,
            BOOL2,
            BOOL3,
            BOOL4,
            FLOAT,
            FLOAT2,
            FLOAT3,
            FLOAT4,
            INT,
            INT2,
            INT3,
            INT4,
            UINT,
            UINT2,
            UINT3,
            UINT4,
            MAT3,
            MAT4,
            SAMPLER_2D,
            SAMPLER_2D_ARRAY,
            SAMPLER_CUBEMAP,
            SAMPLER_EXTERNAL,
            SAMPLER_3D,
            SUBPASS_INPUT
        }

        public enum Precision {
            LOW,
            MEDIUM,
            HIGH,
            DEFAULT
        }

        @SuppressWarnings("unused")
        @UsedByNative("Material.cpp")
        private static final int SAMPLER_OFFSET = Type.MAT4.ordinal() + 1;

        @SuppressWarnings("unused")
        @UsedByNative("Material.cpp")
        private static final int SUBPASS_OFFSET = Type.SAMPLER_3D.ordinal() + 1;

        @NonNull
        public final String name;
        @NonNull
        public final Type type;
        @NonNull
        public final Precision precision;
        @IntRange(from = 1)
        public final int count;

        private Parameter(@NonNull String name, @NonNull Type type, @NonNull Precision precision,
                @IntRange(from = 1) int count) {
            this.name = name;
            this.type = type;
            this.precision = precision;
            this.count = count;
        }

        @SuppressWarnings("unused")
        @UsedByNative("Material.cpp")
        private static void add(@NonNull List<Parameter> parameters, @NonNull String name,
                @IntRange(from = 0) int type, @IntRange(from = 0) int precision,
                @IntRange(from = 1) int count) {
            parameters.add(
                    new Parameter(name, sTypeValues[type], Precision.values()[precision], count));
        }
    }

    public Material(long nativeMaterial) {
        mNativeObject = nativeMaterial;
        long nativeDefaultInstance = nGetDefaultInstance(nativeMaterial);
        mDefaultInstance = new MaterialInstance(this, nativeDefaultInstance);
    }

    public static class Builder {
        private Buffer mBuffer;
        private int mSize;
        private int mShBandCount = 0;

        /**
         * Specifies the material data. The material data is a binary blob produced by
         * libfilamat or by matc.
         *
         * @param buffer  buffer containing material data
         * @param size    size of the material data in bytes
         */
        @NonNull
        public Builder payload(@NonNull Buffer buffer, @IntRange(from = 0) int size) {
            mBuffer = buffer;
            mSize = size;
            return this;
        }

        /**
         * Sets the quality of the indirect lights computations. This is only taken into account
         * if this material is lit and in the surface domain. This setting will affect the
         * IndirectLight computation if one is specified on the Scene and Spherical Harmonics
         * are used for the irradiance.
         *
         * @param shBandCount Number of spherical harmonic bands. Must be 1, 2 or 3 (default).
         * @return Reference to this Builder for chaining calls.
         * @see IndirectLight
         */
        @NonNull
        public Builder sphericalHarmonicsBandCount(@IntRange(from = 0) int shBandCount) {
            mShBandCount = shBandCount;
            return this;
        }

        /**
         * Creates and returns the Material object.
         *
         * @param engine reference to the Engine instance to associate this Material with
         *
         * @return the newly created object
         *
         * @exception IllegalStateException if the material could not be created
         */
        @NonNull
        public Material build(@NonNull Engine engine) {
            long nativeMaterial = nBuilderBuild(engine.getNativeObject(),
                mBuffer, mSize, mShBandCount);
            if (nativeMaterial == 0) throw new IllegalStateException("Couldn't create Material");
            return new Material(nativeMaterial);
        }
    }


    /**
     * Asynchronously ensures that a subset of this Material's variants are compiled. After issuing
     * several compile() calls in a row, it is recommended to call {@link Engine#flush}
     * such that the backend can start the compilation work as soon as possible.
     * The provided callback is guaranteed to be called on the main thread after all specified
     * variants of the material are compiled. This can take hundreds of milliseconds.
     *<p>
     * If all the material's variants are already compiled, the callback will be scheduled as
     * soon as possible, but this might take a few dozen millisecond, corresponding to how
     * many previous frames are enqueued in the backend. This also varies by backend. Therefore,
     * it is recommended to only call this method once per material shortly after creation.
     *</p>
     *<p>
     * If the same variant is scheduled for compilation multiple times, the first scheduling
     * takes precedence; later scheduling are ignored.
     *</p>
     *<p>
     * caveat: A consequence is that if a variant is scheduled on the low priority queue and later
     * scheduled again on the high priority queue, the later scheduling is ignored.
     * Therefore, the second callback could be called before the variant is compiled.
     * However, the first callback, if specified, will trigger as expected.
     *</p>
     *<p>
     * The callback is guaranteed to be called. If the engine is destroyed while some material
     * variants are still compiling or in the queue, these will be discarded and the corresponding
     * callback will be called. In that case however the Material pointer passed to the callback
     * is guaranteed to be invalid (either because it's been destroyed by the user already, or,
     * because it's been cleaned-up by the Engine).
     *</p>
     *<p>
     * {@link UserVariantFilterBit#ALL} should be used with caution. Only variants that an application
     * needs should be included in the variants argument. For example, the STE variant is only used
     * for stereoscopic rendering. If an application is not planning to render in stereo, this bit
     * should be turned off to avoid unnecessary material compilations.
     *</p>
     * @param priority      Which priority queue to use, LOW or HIGH.
     * @param variants      Variants to include to the compile command.
     * @param handler       An {@link java.util.concurrent.Executor Executor}. On Android this can also be a {@link android.os.Handler Handler}.
     * @param callback      callback called on the main thread when the compilation is done on
     *                      by backend.
     */
    public void compile(@NonNull CompilerPriorityQueue priority,
                        int variants,
                        @Nullable Object handler,
                        @Nullable Runnable callback) {
        nCompile(getNativeObject(), priority.ordinal(), variants, handler, callback);
    }

    /**
     * Creates a new instance of this material. Material instances should be freed using
     * {@link Engine#destroyMaterialInstance(MaterialInstance)}.
     *
     * @return the new instance
     */
    @NonNull
    public MaterialInstance createInstance() {
        long nativeInstance = nCreateInstance(getNativeObject());
        if (nativeInstance == 0) throw new IllegalStateException("Couldn't create MaterialInstance");
        return new MaterialInstance(this, nativeInstance);
    }

    /**
     * Creates a new instance of this material with a specified name. Material instances should be
     * freed using {@link Engine#destroyMaterialInstance(MaterialInstance)}.
     *
     * @param name arbitrary label to associate with the given material instance
     *
     * @return the new instance
     */
    @NonNull
    public MaterialInstance createInstance(@NonNull String name) {
        long nativeInstance = nCreateInstanceWithName(getNativeObject(), name);
        if (nativeInstance == 0) throw new IllegalStateException("Couldn't create MaterialInstance");
        return new MaterialInstance(this, nativeInstance);
    }

    /** Returns the material's default instance. */
    @NonNull
    public MaterialInstance getDefaultInstance() {
        return mDefaultInstance;
    }

    /**
     * Returns the name of this material. The material name is used for debugging purposes.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/general:name">
     * General: name</a>
     */
    public String getName() {
        return nGetName(getNativeObject());
    }

    /**
     * Returns the shading model of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialmodels">
     * Material Models</a>
     */
    public Shading getShading() {
        return EnumCache.sShadingValues[nGetShading(getNativeObject())];
    }

    /**
     * Returns the interpolation mode of this material. This affects how variables are interpolated.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:interpolation">
     * Vertex and attributes: interpolation</a>
     */
    public Interpolation getInterpolation() {
        return EnumCache.sInterpolationValues[nGetInterpolation(getNativeObject())];
    }

    /**
     * Returns the blending mode of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:blending">
     * Blending and transparency: blending</a>
     */
    public BlendingMode getBlendingMode() {
        return EnumCache.sBlendingModeValues[nGetBlendingMode(getNativeObject())];
    }

    /**
     * Returns the refraction mode of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:refraction">
     * Blending and transparency: refraction</a>
     */
    public RefractionMode getRefractionMode() {
        return EnumCache.sRefractionModeValues[nGetRefractionMode(getNativeObject())];
    }

    /**
     * Returns the refraction type of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:refractiontype">
     * Blending and transparency: refractionType</a>
     */
    public RefractionType getRefractionType() {
        return EnumCache.sRefractionTypeValues[nGetRefractionType(getNativeObject())];
    }

    /**
     * Returns the reflection mode of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/lighting:reflections">
     * Lighting: reflections</a>
     */
    public ReflectionMode getReflectionMode() {
        return EnumCache.sReflectionModeValues[nGetReflectionMode(getNativeObject())];
    }

    /**
     * Returns the minimum required feature level for this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/general:featurelevel">
     * General: featureLevel</a>
     */
    public FeatureLevel getFeatureLevel() {
        return EnumCache.sFeatureLevelValues[nGetFeatureLevel(getNativeObject())];
    }

    /**
     * Returns the vertex domain of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:vertexdomain">
     * Vertex and attributes: vertexDomain</a>
     */
    public VertexDomain getVertexDomain() {
        return EnumCache.sVertexDomainValues[nGetVertexDomain(getNativeObject())];
    }

    /**
     * Returns the default culling mode of this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling">
     * Rasterization: culling</a>
     */
    public CullingMode getCullingMode() {
        return EnumCache.sCullingModeValues[nGetCullingMode(getNativeObject())];
    }

    /**
     * Indicates whether instances of this material will, by default, write to the color buffer.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:colorwrite">
     * Rasterization: colorWrite</a>
     */
    public boolean isColorWriteEnabled() {
        return nIsColorWriteEnabled(getNativeObject());
    }

    /**
     * Indicates whether instances of this material will, by default, write to the depth buffer.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:depthwrite">
     * Rasterization: depthWrite</a>
     */
    public boolean isDepthWriteEnabled() {
        return nIsDepthWriteEnabled(getNativeObject());
    }

    /**
     * Indicates whether instances of this material will, by default, use depth testing.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:depthculling">
     * Rasterization: depthCulling</a>
     */
    public boolean isDepthCullingEnabled() {
        return nIsDepthCullingEnabled(getNativeObject());
    }

    /**
     * Indicates whether this material is double-sided.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:doublesided">
     * Rasterization: doubleSided</a>
     */
    public boolean isDoubleSided() {
        return nIsDoubleSided(getNativeObject());
    }

    /**
     * Indicates whether instances of this material will use alpha to coverage.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:alphatocoverage">
     * Rasterization: alphaToCoverage</a>
     */
    public boolean isAlphaToCoverageEnabled() {
        return nIsAlphaToCoverageEnabled(getNativeObject());
    }

    /**
     * Returns the alpha mask threshold used when the blending mode is set to masked.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:maskthreshold">
     * Blending and transparency: maskThreshold</a>
     */
    public float getMaskThreshold() {
        return nGetMaskThreshold(getNativeObject());
    }

    /**
     * Returns the screen-space variance for specular-antialiasing. This value is between 0 and 1.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/anti-aliasing:specularantialiasingvariance">
     * Anti-aliasing: specularAntiAliasingVariance</a>
     */
    public float getSpecularAntiAliasingVariance() {
        return nGetSpecularAntiAliasingVariance(getNativeObject());
    }

    /**
     * Returns the clamping threshold for specular-antialiasing. This value is between 0 and 1.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/anti-aliasing:specularantialiasingthreshold">
     * Anti-aliasing: specularAntiAliasingThreshold</a>
     */
    public float getSpecularAntiAliasingThreshold() {
        return nGetSpecularAntiAliasingThreshold(getNativeObject());
    }

    /**
     * Returns a set of {@link VertexBuffer.VertexAttribute}s that are required by this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:requires">
     * Vertex and attributes: requires</a>
     */
    public Set<VertexBuffer.VertexAttribute> getRequiredAttributes() {
        if (mRequiredAttributes == null) {
            int bitSet = nGetRequiredAttributes(getNativeObject());
            mRequiredAttributes = EnumSet.noneOf(VertexBuffer.VertexAttribute.class);
            VertexBuffer.VertexAttribute[] values = EnumCache.sVertexAttributeValues;
            for (int i = 0; i < values.length; i++) {
                if ((bitSet & (1 << i)) != 0) {
                    mRequiredAttributes.add(values[i]);
                }
            }
            mRequiredAttributes = Collections.unmodifiableSet(mRequiredAttributes);
        }
        return mRequiredAttributes;
    }

    /**
     * Returns a bit set representing the set of {@link VertexBuffer.VertexAttribute}s that are
     * required by this material. Use {@link #getRequiredAttributes()} to get these as a Set object.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/vertexandattributes:requires">
     * Vertex and attributes: requires</a>
     */
    int getRequiredAttributesAsInt() {
        return nGetRequiredAttributes(getNativeObject());
    }

    /**
     * Returns the number of parameters declared by this material.
     * The returned value can be 0.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/general:parameters">
     * General: parameters</a>
     */
    public int getParameterCount() {
        return nGetParameterCount(getNativeObject());
    }

    /**
     * Returns a list of Parameter objects representing this material's parameters.
     * The list may be empty if the material has no declared parameters.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/general:parameters">
     * General: parameters</a>
     */
    public List<Parameter> getParameters() {
        int count = getParameterCount();
        List<Parameter> parameters = new ArrayList<>(count);
        if (count > 0) nGetParameters(getNativeObject(), parameters, count);
        return parameters;
    }

    /**
     * Indicates whether a parameter of the given name exists on this material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/general:parameters">
     * General: parameters</a>
     */
    public boolean hasParameter(@NonNull String name) {
        return nHasParameter(getNativeObject(), name);
    }

    /**
     * Sets the value of a bool parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, boolean x) {
        mDefaultInstance.setParameter(name, x);
    }

    /**
     * Sets the value of a float parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, float x) {
        mDefaultInstance.setParameter(name, x);
    }

    /**
     * Sets the value of an int parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, int x) {
        mDefaultInstance.setParameter(name, x);
    }

    /**
     * Sets the value of a bool2 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, boolean x, boolean y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    /**
     * Sets the value of a float2 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, float x, float y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    /**
     * Sets the value of an int2 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, int x, int y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    /**
     * Sets the value of a bool3 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, boolean x, boolean y, boolean z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    /**
     * Sets the value of a float3 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, float x, float y, float z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    /**
     * Sets the value of a int3 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, int x, int y, int z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    /**
     * Sets the value of a bool4 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, boolean x, boolean y, boolean z, boolean w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    /**
     * Sets the value of a float4 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, float x, float y, float z, float w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    /**
     * Sets the value of a int4 parameter on this material's default instance.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, int x, int y, int z, int w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    /**
     * Set a bool parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 bool4s:
     * <pre>{@code
     *     boolean[] a = new boolean[4 * 4];
     *     material.setDefaultParameter("param", MaterialInstance.BooleanElement.BOOL4, a, 0, 4);
     * }</pre>
     * </p>
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.BooleanElement type, @NonNull @Size(min = 1) boolean[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    /**
     * Set an int parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 int4s:
     * <pre>{@code
     *     int[] a = new int[4 * 4];
     *     material.setDefaultParameter("param", MaterialInstance.IntElement.INT4, a, 0, 4);
     * }</pre>
     * </p>
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.IntElement type, @NonNull @Size(min = 1) int[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    /**
     * Set a float parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 float4s:
     * <pre>{@code
     *     float[] a = new float[4 * 4];
     *     material.setDefaultParameter("param", MaterialInstance.FloatElement.FLOAT4, a, 0, 4);
     * }</pre>
     * </p>
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.FloatElement type, @NonNull @Size(min = 1) float[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name the name of the material color parameter
     * @param type whether the color is specified in the linear or sRGB space
     * @param r    red component
     * @param g    green component
     * @param b    blue component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, @NonNull Colors.RgbType type,
            float r, float g, float b) {
        mDefaultInstance.setParameter(name, type, r, g, b);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name the name of the material color parameter
     * @param type whether the color is specified in the linear or sRGB space
     * @param r    red component
     * @param g    green component
     * @param b    blue component
     * @param a    alpha component
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name, @NonNull Colors.RgbaType type,
            float r, float g, float b, float a) {
        mDefaultInstance.setParameter(name, type, r, g, b, a);
    }

    /**
     * Sets a texture and sampler parameter on this material's default instance.
     *
     * @param name The name of the material texture parameter
     * @param texture The texture to set as parameter
     * @param sampler The sampler to be used with this texture
     *
     * @see Material#getDefaultInstance()
     */
    public void setDefaultParameter(@NonNull String name,
            @NonNull Texture texture, @NonNull TextureSampler sampler) {
        mDefaultInstance.setParameter(name, texture, sampler);
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Material");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nBuilderBuild(long nativeEngine, @NonNull Buffer buffer, int size, int shBandCount);
    private static native long nCreateInstance(long nativeMaterial);
    private static native long nCreateInstanceWithName(long nativeMaterial, @NonNull String name);
    private static native long nGetDefaultInstance(long nativeMaterial);

    private static native void nCompile(long nativeMaterial, int priority, int variants, Object handler, Runnable runnable);
    private static native String nGetName(long nativeMaterial);
    private static native int nGetShading(long nativeMaterial);
    private static native int nGetInterpolation(long nativeMaterial);
    private static native int nGetBlendingMode(long nativeMaterial);
    private static native int nGetVertexDomain(long nativeMaterial);
    private static native int nGetCullingMode(long nativeMaterial);
    private static native boolean nIsColorWriteEnabled(long nativeMaterial);
    private static native boolean nIsDepthWriteEnabled(long nativeMaterial);
    private static native boolean nIsDepthCullingEnabled(long nativeMaterial);
    private static native boolean nIsDoubleSided(long nativeMaterial);
    private static native boolean nIsAlphaToCoverageEnabled(long nativeMaterial);
    private static native float nGetMaskThreshold(long nativeMaterial);
    private static native float nGetSpecularAntiAliasingVariance(long nativeMaterial);
    private static native float nGetSpecularAntiAliasingThreshold(long nativeMaterial);
    private static native int nGetRefractionMode(long nativeMaterial);
    private static native int nGetRefractionType(long nativeMaterial);
    private static native int nGetReflectionMode(long nativeMaterial);
    private static native int nGetFeatureLevel(long nativeMaterial);


    private static native int nGetParameterCount(long nativeMaterial);
    private static native void nGetParameters(long nativeMaterial,
            @NonNull List<Parameter> parameters, @IntRange(from = 1) int count);
    private static native int nGetRequiredAttributes(long nativeMaterial);

    private static native boolean nHasParameter(long nativeMaterial, @NonNull String name);
}
