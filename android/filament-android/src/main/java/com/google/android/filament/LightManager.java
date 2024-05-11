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

/**
 * LightManager allows you to create a light source in the scene, such as a sun or street lights.
 * <p>
 * At least one light must be added to a scene in order to see anything
 * (unless the {@link Material.Shading#UNLIT} is used).
 * </p>
 *
 * <h1><u>Creation and destruction</u></h1>
 * <p>
 * A Light component is created using the {@link LightManager.Builder} and destroyed by calling
 * {@link LightManager#destroy}.
 * </p>
 * <pre>
 *  Engine engine = Engine.create();
 *  int sun = EntityManager.get().create();
 *
 *  LightManager.Builder(Type.SUN)
 *              .castShadows(true)
 *              .build(engine, sun);
 *
 *  engine.getLightManager().destroy(sun);
 * </pre>
 *
 * <h1><u>Light types</u></h1>
 *
 * Lights come in three flavors:
 * <ul>
 * <li>directional lights</li>
 * <li>point lights</li>
 * <li>spot lights</li>
 * </ul>
 *
 *
 * <h2><u>Directional lights</u></h2>
 * <p>
 * Directional lights have a direction, but don't have a position. All light rays are
 * parallel and come from infinitely far away and from everywhere. Typically a directional light
 * is used to simulate the sun.
 * </p>
 * <p>
 * Directional lights and spot lights are able to cast shadows.
 * </p>
 * <p>
 * To create a directional light use {@link Type#DIRECTIONAL} or {@link Type#SUN}, both are similar,
 * but the later also draws a sun's disk in the sky and its reflection on glossy objects.
 * </p>
 * <p>
 * <b>warning:</b> Currently, only a single directional light is supported. If several directional lights
 * are added to the scene, the dominant one will be used.
 * </p>
 *
 * <h2><u>Point lights</u></h2>
 *
 * Unlike directional lights, point lights have a position but emit light in all directions.
 * The intensity of the light diminishes with the inverse square of the distance to the light.
 * {@link Builder#falloff} controls the distance beyond which the light has no more influence.
 * <p>
 * A scene can have multiple point lights.
 * </p>
 *
 * <h2><u>Spot lights</u></h2>
 * <p>
 * Spot lights are similar to point lights but the light they emit is limited to a cone defined by
 * {@link Builder#spotLightCone} and the light's direction.
 * </p>
 * <p>
 * A spot light is therefore defined by a position, a direction and inner and outer cones. The
 * spot light's influence is limited to inside the outer cone. The inner cone defines the light's
 * falloff attenuation.
 * </p>
 * A physically correct spot light is a little difficult to use because changing the outer angle
 * of the cone changes the illumination levels, as the same amount of light is spread over a
 * changing volume. The coupling of illumination and the outer cone means that an artist cannot
 * tweak the influence cone of a spot light without also changing the perceived illumination.
 * It therefore makes sense to provide artists with a parameter to disable this coupling. This
 * is the difference between {@link Type#FOCUSED_SPOT} (physically correct) and {@link Type#SPOT}
 * (decoupled).
 * </p>
 *
 * <h1><u>Performance considerations</u></h1>
 * <p>
 * Generally, adding lights to the scene hurts performance, however filament is designed to be
 * able to handle hundreds of lights in a scene under certain conditions. Here are some tips
 * to keep good performance.
 * </p>
 * <ul>
 * <li> Prefer spot lights to point lights and use the smallest outer cone angle possible.</li>
 * <li> Use the smallest possible falloff distance for point and spot lights.
 *    Performance is very sensitive to overlapping lights. The falloff distance essentially
 *    defines a sphere of influence for the light, so try to position point and spot lights
 *    such that they don't overlap too much.</li>
 *    On the other hand, a scene can contain hundreds of non overlapping lights without
 *    incurring a significant overhead.
 * </ul>
 */
public class LightManager {
    private static final Type[] sTypeValues = Type.values();

    private long mNativeObject;

    LightManager(long nativeLightManager) {
        mNativeObject = nativeLightManager;
    }

    /**
     * Returns the number of components in the LightManager, note that components are not
     * guaranteed to be active. Use the {@link EntityManager#isAlive} before use if needed.
     *
     * @return number of component in the LightManager
     */
    public int getComponentCount() {
        return nGetComponentCount(mNativeObject);
    }

    /**
     * Returns whether a particular Entity is associated with a component of this LightManager
     * @param entity An Entity.
     * @return true if this Entity has a component associated with this manager.
     */
    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    /**
     * Gets an Instance representing the Light component associated with the given Entity.
     * @param entity An Entity.
     * @return An Instance object, which represents the Light component associated with the Entity entity.
     *         The instance is 0 if the component doesn't exist.
     * @see #hasComponent
     */
    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    /**
     * Destroys this component from the given entity
     * @param entity An Entity.
     */
    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    /**
     * Denotes the type of the light being created.
     */
    public enum Type {
        /** Directional light that also draws a sun's disk in the sky. */
        SUN,

        /** Directional light, emits light in a given direction. */
        DIRECTIONAL,

        /** Point light, emits light from a position, in all directions. */
        POINT,

        /** Physically correct spot light. */
        FOCUSED_SPOT,

        /** Spot light with coupling of outer cone and illumination disabled. */
        SPOT

    }


    /**
     * Control the quality / performance of the shadow map associated to this light
     */
    public static class ShadowOptions {
        /** Size of the shadow map in texels. Must be a power-of-two and larger or equal to 8. */
        public int mapSize = 1024;

        /**
         * Number of shadow cascades to use for this light. Must be between 1 and 4 (inclusive).
         * A value greater than 1 turns on cascaded shadow mapping (CSM).
         * Only applicable to Type.SUN or Type.DIRECTIONAL lights.
         *
         * <p>
         * When using shadow cascades, {@link ShadowOptions#cascadeSplitPositions} must also be set.
         * </p>
         *
         * @see ShadowOptions#cascadeSplitPositions
         */
        @IntRange(from = 1, to = 4)
        public int shadowCascades = 1;

        /**
         * The split positions for shadow cascades.
         *
         * <p>
         * Cascaded shadow mapping (CSM) partitions the camera frustum into cascades. These values
         * determine the planes along the camera's Z axis to split the frustum. The camera near
         * plane is represented by 0.0f and the far plane represented by 1.0f.
         * </p>
         *
         * <p>
         * For example, if using 4 cascades, these values would set a uniform split scheme:
         * { 0.25f, 0.50f, 0.75f }
         * </p>
         *
         * <p>
         * For N cascades, N - 1 split positions will be read from this array.
         * </p>
         *
         * <p>
         * Filament provides utility methods inside {@link ShadowCascades} to help set these values.
         * For example, to use a uniform split scheme:
         * </p>
         *
         * <pre>
         * LightManager.ShadowCascades.computeUniformSplits(options.cascadeSplitPositions, 4);
         * </pre>
         *
         * @see ShadowCascades#computeUniformSplits
         * @see ShadowCascades#computeLogSplits
         * @see ShadowCascades#computePracticalSplits
         */
        @NonNull
        @Size(min = 3)
        public float[] cascadeSplitPositions = { 0.125f, 0.25f, 0.50f };

        /** Constant bias in world units (e.g. meters) by which shadows are moved away from the
         * light. 1mm by default.
         * This is ignored when the View's ShadowType is set to VSM.
         */
        public float constantBias = 0.001f;

        /** Amount by which the maximum sampling error is scaled. The resulting value is used
         * to move the shadow away from the fragment normal. Should be 1.0.
         * This is ignored when the View's ShadowType is set to VSM.
         */
        public float normalBias = 1.0f;

        /** Distance from the camera after which shadows are clipped. This is used to clip
         * shadows that are too far and wouldn't contribute to the scene much, improving
         * performance and quality. This value is always positive.
         * Use 0.0f to use the camera far distance.
         * This only affect directional lights.
         */
        public float shadowFar = 0.0f;

        /** Optimize the quality of shadows from this distance from the camera. Shadows will
         * be rendered in front of this distance, but the quality may not be optimal.
         * This value is always positive. Use 0.0f to use the camera near distance.
         * The default of 1m works well with many scenes. The quality of shadows may drop
         * rapidly when this value decreases.
         */
        public float shadowNearHint = 1.0f;

        /** Optimize the quality of shadows in front of this distance from the camera. Shadows
         * will be rendered behind this distance, but the quality may not be optimal.
         * This value is always positive. Use std::numerical_limits<float>::infinity() to
         * use the camera far distance.
         */
        public float shadowFarHint = 100.0f;

        /**
         * Controls whether the shadow map should be optimized for resolution or stability.
         * When set to true, all resolution enhancing features that can affect stability are
         * disabling, resulting in significantly lower resolution shadows, albeit stable ones.
         *
         * Setting this flag to true always disables LiSPSM (see below).
         */
        public boolean stable = false;

        /**
         * LiSPSM, or light-space perspective shadow-mapping is a technique allowing to better
         * optimize the use of the shadow-map texture. When enabled the effective resolution of
         * shadows is greatly improved and yields result similar to using cascades without the
         * extra cost. LiSPSM comes with some drawbacks however, in particular it is incompatible
         * with blurring because it effectively affects the blur kernel size.
         *
         * Blurring is only an issue when using ShadowType.VSM with a large blur or with
         * ShadowType.PCSS however.
         *
         * If these blurring artifacts become problematic, this flag can be used to disable LiSPSM.
         */
        public boolean lispsm = false;

        /**
         * Constant bias in depth-resolution units by which shadows are moved away from the
         * light. The default value of 0.5 is used to round depth values up.
         * Generally this value shouldn't be changed or at least be small and positive.
         * This is ignored when the View's ShadowType is set to VSM.
         */
        float polygonOffsetConstant = 0.5f;

        /**
         * Bias based on the change in depth in depth-resolution units by which shadows are moved
         * away from the light. The default value of 2.0 works well with SHADOW_SAMPLING_PCF_LOW.
         * Generally this value is between 0.5 and the size in texel of the PCF filter.
         * Setting this value correctly is essential for LISPSM shadow-maps.
         * This is ignored when the View's ShadowType is set to VSM.
         */
        float polygonOffsetSlope = 2.0f;

        /**
         * Whether screen-space contact shadows are used. This applies regardless of whether a
         * Renderable is a shadow caster.
         * Screen-space contact shadows are typically useful in large scenes.
         * (off by default)
         */
        public boolean screenSpaceContactShadows = false;

        /**
         * Number of ray-marching steps for screen-space contact shadows (8 by default).
         *<p>
         * <b>CAUTION:</b> this parameter is ignored for all lights except the directional/sun light,
         *                 all other lights use the same value set for the directional/sun light.
         *</p>
         */
        public int stepCount = 8;

        /**
         * Maximum shadow-occluder distance for screen-space contact shadows (world units).
         * (30 cm by default)
         *<p>
         * <b>CAUTION:</b> this parameter is ignored for all lights except the directional/sun light,
         *                 all other lights use the same value set for the directional/sun light.
         *</p>
         */
        public float maxShadowDistance = 0.3f;

        /*
         * Options prefixed with 'vsm' are available when the View's ShadowType is set to VSM.
         *
         * @see View#setShadowType
         */

        /**
         * When elvsm is set to true, "Exponential Layered VSM without Layers" are used. It is
         * an improvement to the default EVSM which suffers important light leaks. Enabling
         * ELVSM for a single shadowmap doubles the memory usage of all shadow maps.
         * ELVSM is mostly useful when large blurs are used.
         */
        public boolean elvsm = false;

        /**
         * Blur width for the VSM blur. Zero do disable.
         * The maximum value is 125.
         */
        public float blurWidth = 0.0f;

        /**
         * Light bulb radius used for soft shadows. Currently this is only used when DPCF is
         * enabled. (2cm by default).
         */
        public float shadowBulbRadius = 0.02f;

        /**
         * Transforms the shadow direction. Must be a unit quaternion.
         * The default is identity.
         * Ignored if the light type isn't directional. For artistic use. Use with caution.
         * The quaternion is stored as the imaginary part in the first 3 elements and the real
         * part in the last element of the transform array.
         */
        @NonNull
        @Size(min = 4, max = 4)
        public float[] transform = { 0.0f, 0.0f, 0.0f, 1.0f };
    }

    public static class ShadowCascades {
        /**
         * Utility method to compute {@link ShadowOptions#cascadeSplitPositions} according to a
         * uniform split scheme.
         *
         * @param splitPositions    a float array of at least size (cascades - 1) to write the split
         *                          positions into
         * @param cascades          the number of shadow cascades, at most 4
         */
        public static void computeUniformSplits(@NonNull @Size(min = 1) float[] splitPositions,
                @IntRange(from = 1, to = 4) int cascades) {
            if (splitPositions.length < cascades - 1) {
                throw new ArrayIndexOutOfBoundsException(
                        String.format("splitPositions array length must be at least %d", cascades - 1));
            }
            nComputeUniformSplits(splitPositions, cascades);
        }

        /**
         * Utility method to compute {@link ShadowOptions#cascadeSplitPositions} according to a
         * logarithmic split scheme.
         *
         * @param splitPositions    a float array of at least size (cascades - 1) to write the split
         *                          positions into
         * @param cascades          the number of shadow cascades, at most 4
         * @param near              the camera near plane
         * @param far               the camera far plane
         */
        public static void computeLogSplits(@NonNull @Size(min = 1) float[] splitPositions,
                @IntRange(from = 1, to = 4) int cascades, float near, float far) {
            if (splitPositions.length < cascades - 1) {
                throw new ArrayIndexOutOfBoundsException(
                        String.format("splitPositions array length must be at least %d", cascades - 1));
            }
            nComputeLogSplits(splitPositions, cascades, near, far);
        }

        /**
         * Utility method to compute {@link ShadowOptions#cascadeSplitPositions} according to a
         * practical split scheme.
         *
         * <p>
         * The practical split scheme uses uses a lambda value to interpolate between the logrithmic
         * and uniform split schemes. Start with a lambda value of 0.5f and adjust for your scene.
         * </p>
         *
         * See: Zhang et al 2006, "Parallel-split shadow maps for large-scale virtual environments"
         *
         * @param splitPositions    a float array of at least size (cascades - 1) to write the split
         *                          positions into
         * @param cascades          the number of shadow cascades, at most 4
         * @param near              the camera near plane
         * @param far               the camera far plane
         * @param lambda            a float in the range [0, 1] that interpolates between log and
         *                          uniform split schemes
         */
        public static void computePracticalSplits(@NonNull @Size(min = 1) float[] splitPositions,
              @IntRange(from = 1, to = 4) int cascades, float near, float far, float lambda) {
            if (splitPositions.length < cascades - 1) {
                throw new ArrayIndexOutOfBoundsException(
                        String.format("splitPositions array length must be at least %d", cascades - 1));
            }
            nComputePracticalSplits(splitPositions, cascades, near, far, lambda);
        }
    }

    /** Typical efficiency of an incandescent light bulb (2.2%) */
    public static final float EFFICIENCY_INCANDESCENT = 0.0220f;

    /** Typical efficiency of an halogen light bulb (7.0%) */
    public static final float EFFICIENCY_HALOGEN      = 0.0707f;

    /** Typical efficiency of a fluorescent light bulb (8.7%) */
    public static final float EFFICIENCY_FLUORESCENT  = 0.0878f;

    /** Typical efficiency of a LED light bulb (11.7%) */
    public static final float EFFICIENCY_LED          = 0.1171f;

    /**
     *  Use Builder to construct a Light object instance
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Creates a light builder and set the light's {@link Type}.
         *
         * @param type {@link Type} of Light object to create.
         */
        public Builder(@NonNull Type type) {
            mNativeBuilder = nCreateBuilder(type.ordinal());
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Enables or disables a light channel. Light channel 0 is enabled by default.
         *
         * @param channel Light channel to enable or disable, between 0 and 7.
         * @param enable Whether to enable or disable the light channel.
         */
        @NonNull
        public Builder lightChannel(@IntRange(from = 0, to = 7) int channel, boolean enable) {
            nBuilderLightChannel(mNativeBuilder, channel, enable);
            return this;
        }

        /**
         * Whether this Light casts shadows (disabled by default)
         *
         * @param enable Enables or disables casting shadows from this Light.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder castShadows(boolean enable) {
            nBuilderCastShadows(mNativeBuilder, enable);
            return this;
        }

        /**
         * Sets the shadow map options for this light.
         * @param options A {@link ShadowOptions} instance
         * @return This Builder, for chaining calls.
         * @see ShadowOptions
         */
        @NonNull
        public Builder shadowOptions(@NonNull ShadowOptions options) {
            nBuilderShadowOptions(mNativeBuilder,
                    options.mapSize, options.shadowCascades, options.cascadeSplitPositions,
                    options.constantBias, options.normalBias, options.shadowFar, options.shadowNearHint,
                    options.shadowFarHint, options.stable, options.lispsm,
                    options.polygonOffsetConstant, options.polygonOffsetSlope,
                    options.screenSpaceContactShadows,
                    options.stepCount, options.maxShadowDistance,
                    options.elvsm, options.blurWidth, options.shadowBulbRadius, options.transform);
            return this;
        }

        /**
         * Whether this light casts light (enabled by default)
         *
         * <p>
         * In some situations it can be useful to have a light in the scene that doesn't
         * actually emit light, but does cast shadows.
         * </p>
         *
         * @param enabled Enables or disables lighting from this Light.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder castLight(boolean enabled) {
            nBuilderCastLight(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Sets the initial position of the light in world space.
         * <p>
         * <b>note:</b> The Light's position is ignored for directional lights
         * ({@link Type#DIRECTIONAL} or {@link Type#SUN})
         * </p>
         *
         * @param x Light's position x coordinate in world space. The default is 0.
         * @param y Light's position y coordinate in world space. The default is 0.
         * @param z Light's position z coordinate in world space. The default is 0.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder position(float x, float y, float z) {
            nBuilderPosition(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets the initial direction of a light in world space.
         * <p>
         * The light direction is specified in world space and should be a unit vector.
         * </p>
         * <p>
         * <b>note:</b> The Light's direction is ignored for  {@link Type#POINT} lights.
         * </p>
         *
         * @param x light's direction x coordinate (default is 0)
         * @param y light's direction y coordinate (default is -1)
         * @param z light's direction z coordinate (default is 0)
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder direction(float x, float y, float z) {
            nBuilderDirection(mNativeBuilder, x, y, z);
            return this;
        }

        /**
         * Sets the initial color of a light.
         * <p>
         * The light color is specified in the linear sRGB color-space. The default is white.
         * </p>
         *
         * @param linearR red component of the color (default is 1)
         * @param linearG green component of the color (default is 1)
         * @param linearB blue component of the color (default is 1)
         * @return This Builder, for chaining calls.
         * @see #setColor
         */
        @NonNull
        public Builder color(float linearR, float linearG, float linearB) {
            nBuilderColor(mNativeBuilder, linearR, linearG, linearB);
            return this;
        }

        /**
         * Sets the initial intensity of a light.
         *
         * This method overrides any prior calls to #intensity or #intensityCandela.
         *
         * @param intensity This parameter depends on the {@link Type}, for directional lights,
         *                  it specifies the illuminance in <i>lux</i> (or <i>lumen/m^2</i>).
         *                  For point lights and spot lights, it specifies the luminous power
         *                  in <i>lumen</i>. For example, the sun's illuminance is about 100,000
         *                  lux.
         *
         * @return This Builder, for chaining calls.
         *
         * @see #setIntensity
         */
        @NonNull
        public Builder intensity(float intensity) {
            nBuilderIntensity(mNativeBuilder, intensity);
            return this;
        }

        /**
         * Sets the initial intensity of a light in watts.
         *
         *<pre>
         *  Lightbulb type  | Efficiency
         * -----------------+------------
         *     Incandescent |  2.2%
         *         Halogen  |  7.0%
         *             LED  |  8.7%
         *     Fluorescent  | 10.7%
         *</pre>
         *
         *
         * This call is equivalent to:
         *<pre>
         * Builder.intensity(efficiency * 683 * watts);
         *</pre>
         *
         * This method overrides any prior calls to #intensity or #intensityCandela.
         *
         * @param watts         Energy consumed by a lightbulb. It is related to the energy produced
         *                      and ultimately the brightness by the efficiency parameter.
         *                      This value is often available on the packaging of commercial
         *                      lightbulbs.
         *
         * @param efficiency    Efficiency in percent. This depends on the type of lightbulb used.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder intensity(float watts, float efficiency) {
            nBuilderIntensity(mNativeBuilder, watts, efficiency);
            return this;
        }

        /**
         * Sets the initial intensity of a spot or point light in candela.
         *
         * @param intensity Luminous intensity in <i>candela</i>.
         *
         * This method is equivalent to calling the plain intensity method for directional lights
         * (Type.DIRECTIONAL or Type.SUN).
         *
         * This method overrides any prior calls to #intensity or #intensityCandela.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder intensityCandela(float intensity) {
            nBuilderIntensityCandela(mNativeBuilder, intensity);
            return this;
        }

        /**
         * Set the falloff distance for point lights and spot lights.
         *<p>
         * At the falloff distance, the light has no more effect on objects.
         *</p>
         *
         *<p>
         * The falloff distance essentially defines a <b>sphere of influence</b> around the light,
         * and therefore has an impact on performance. Larger falloffs might reduce performance
         * significantly, especially when many lights are used.
         *</p>
         *
         *<p>
         * Try to avoid having a large number of light's spheres of influence overlap.
         *</p>
         *
         * The Light's falloff is ignored for directional lights
         * ({@link Type#DIRECTIONAL} or {@link Type#SUN})
         *
         * @param radius Falloff distance in world units. Default is 1 meter.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder falloff(float radius) {
            nBuilderFalloff(mNativeBuilder, radius);
            return this;
        }

        /**
         * Defines a spot light's angular falloff attenuation.
         * <p>
         * A spot light is defined by a position, a direction and two cones, inner and outer.
         * These two cones are used to define the angular falloff attenuation of the spot light
         * and are defined by the angle from the center axis to where the falloff begins (i.e.
         * cones are defined by their half-angle).
         * </p>
         * <p>
         * <b>note:</b> The spot light cone is ignored for directional and point lights.
         * </p>
         *
         * @param inner inner cone angle in <i>radian</i> between 0 and pi/2
         * @param outer outer cone angle in <i>radian</i> between inner and pi/2
         * @return This Builder, for chaining calls.
         *
         * @see Type#SPOT
         * @see Type#FOCUSED_SPOT
         */
        @NonNull
        public Builder spotLightCone(float inner, float outer) {
            nBuilderSpotLightCone(mNativeBuilder, inner, outer);
            return this;
        }

        /**
         * Defines the angular radius of the sun, in degrees, between 0.25° and 20.0°
         *
         * The Sun as seen from Earth has an angular size of 0.526° to 0.545°
         *
         * @param angularRadius sun's radius in degree. Default is 0.545°.
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder sunAngularRadius(float angularRadius) {
            nBuilderAngularRadius(mNativeBuilder, angularRadius);
            return this;
        }

        /**
         * Defines the halo radius of the sun. The radius of the halo is defined as a
         * multiplier of the sun angular radius.
         *
         * @param haloSize radius multiplier. Default is 10.0.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder sunHaloSize(float haloSize) {
            nBuilderHaloSize(mNativeBuilder, haloSize);
            return this;
        }

        /**
         * Defines the halo falloff of the sun. The falloff is a dimensionless number
         * used as an exponent.
         *
         * @param haloFalloff halo falloff. Default is 80.0.
         *
         * @return This Builder, for chaining calls.
         */
        @NonNull
        public Builder sunHaloFalloff(float haloFalloff) {
            nBuilderHaloFalloff(mNativeBuilder, haloFalloff);
            return this;
        }

        /**
         * Adds the Light component to an entity.
         *
         * <p>
         * If this component already exists on the given entity, it is first destroyed as if
         * {@link #destroy} was called.
         * </p>
         *
         * <b>warning:</b>
         * Currently, only 2048 lights can be created on a given Engine.
         *
         * @param engine Reference to the {@link Engine} to associate this light with.
         * @param entity Entity to add the light component to.
         */
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
        return sTypeValues[nGetType(mNativeObject, i)];
    }

    /**
     * Helper function that returns if a light is a directional light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of directional light
     */
    boolean isDirectional(@EntityInstance int i) {
        Type type = getType(i);
        return type == Type.DIRECTIONAL || type == Type.SUN;
    }

    /**
     * Helper function that returns if a light is a point light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of point light
     */
    boolean isPointLight(@EntityInstance int i) {
        return getType(i) == Type.POINT;
    }

    /**
     * Helper function that returns if a light is a spot light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of spot light
     */
    boolean isSpotLight(@EntityInstance int i) {
        Type type = getType(i);
        return type == Type.SPOT || type == Type.FOCUSED_SPOT;
    }

    /**
     * Enables or disables a light channel.
     * Light channel 0 is enabled by default.
     *
     * @param i        Instance of the component obtained from getInstance().
     * @param channel  Light channel to set
     * @param enable   true to enable, false to disable
     *
     * @see Builder#lightChannel
     */
    public void setLightChannel(@EntityInstance int i, @IntRange(from = 0, to = 7) int channel, boolean enable) {
        nSetLightChannel(mNativeObject, i, channel, enable);
    }

    /**
     * Returns whether a light channel is enabled on a specified renderable.
     * @param i        Instance of the component obtained from getInstance().
     * @param channel  Light channel to query
     * @return         true if the light channel is enabled, false otherwise
     */
    public boolean getLightChannel(@EntityInstance int i, @IntRange(from = 0, to = 7) int channel) {
        return nGetLightChannel(mNativeObject, i, channel);
    }

    /**
     * Dynamically updates the light's position.
     *
     * <p>
     * <b>note:</b> The Light's position is ignored for directional lights
     * ({@link Type#DIRECTIONAL} or {@link Type#SUN})
     * </p>
     *
     * @param i        Instance of the component obtained from getInstance().
     * @param x Light's position x coordinate in world space. The default is 0.
     * @param y Light's position y coordinate in world space. The default is 0.
     * @param z Light's position z coordinate in world space. The default is 0.
     *
     * @see Builder#position
     */
    public void setPosition(@EntityInstance int i, float x, float y, float z) {
        nSetPosition(mNativeObject, i, x, y, z);
    }

    /**
     * returns the light's position in world space
     * @param i     Instance of the component obtained from getInstance().
     * @param out   An array of 3 float to receive the result or null.
     * @return      An array of 3 float containing the light's position coordinates.
     */
    @NonNull
    public float[] getPosition(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetPosition(mNativeObject, i, out);
        return out;
    }

    /**
     * Dynamically updates the light's direction
     *
     * <p>
     * The light direction is specified in world space and should be a unit vector.
     * </p>
     * <p>
     * <b>note:</b> The Light's direction is ignored for  {@link Type#POINT} lights.
     * </p>
     *
     * @param i Instance of the component obtained from getInstance().
     * @param x light's direction x coordinate (default is 0)
     * @param y light's direction y coordinate (default is -1)
     * @param z light's direction z coordinate (default is 0)
     *
     * @see Builder#direction
     */
    public void setDirection(@EntityInstance int i, float x, float y, float z) {
        nSetDirection(mNativeObject, i, x, y, z);
    }

    /**
     * returns the light's direction in world space
     * @param i     Instance of the component obtained from getInstance().
     * @param out   An array of 3 float to receive the result or null.
     * @return      An array of 3 float containing the light's direction.
     */
    @NonNull
    public float[] getDirection(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetDirection(mNativeObject, i, out);
        return out;
    }

    /**
     * Dynamically updates the light's hue as linear sRGB
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param linearR red component of the color (default is 1)
     * @param linearG green component of the color (default is 1)
     * @param linearB blue component of the color (default is 1)
     *
     * @see Builder#color
     * @see #getInstance
     */
    public void setColor(@EntityInstance int i, float linearR, float linearG, float linearB) {
        nSetColor(mNativeObject, i, linearR, linearG, linearB);
    }

    /**
     * Returns the light color
     * @param i     Instance of the component obtained from getInstance().
     * @return      An array of 3 float containing the light's color in linear sRGB
     */
    @NonNull
    public float[] getColor(@EntityInstance int i, @Nullable @Size(min = 3) float[] out) {
        out = Asserts.assertFloat3(out);
        nGetColor(mNativeObject, i, out);
        return out;
    }

    /**
     * Dynamically updates the light's intensity. The intensity can be negative.
     *
     * @param i         Instance of the component obtained from getInstance().
     * @param intensity This parameter depends on the {@link Type}, for directional lights,
     *                  it specifies the illuminance in <i>lux</i> (or <i>lumen/m^2</i>).
     *                  For point lights and spot lights, it specifies the luminous power
     *                  in <i>lumen</i>. For example, the sun's illuminance is about 100,000
     *                  lux.
     *
     * @see Builder#intensity
     */
    public void setIntensity(@EntityInstance int i, float intensity) {
        nSetIntensity(mNativeObject, i, intensity);
    }

    /**
     * Dynamically updates the light's intensity in candela. The intensity can be negative.
     *
     * This method is equivalent to calling setIntensity for directional lights (Type.DIRECTIONAL
     * or Type.SUN).
     *
     * @param i         Instance of the component obtained from getInstance().
     * @param intensity Luminous intensity in <i>candela</i>.
     *
     * @see Builder#intensityCandela
     */
    public void setIntensityCandela(@EntityInstance int i, float intensity) {
        nSetIntensityCandela(mNativeObject, i, intensity);
    }

    /**
     * Dynamically updates the light's intensity. The intensity can be negative.
     *
     *<pre>
     *  Lightbulb type  | Efficiency
     * -----------------+------------
     *     Incandescent |  2.2%
     *         Halogen  |  7.0%
     *             LED  |  8.7%
     *     Fluorescent  | 10.7%
     *</pre>
     *
     *
     * This call is equivalent to:
     *<pre>
     * Builder.intensity(efficiency * 683 * watts);
     *</pre>
     *
     * @param i             Instance of the component obtained from getInstance().
     *
     * @param watts         Energy consumed by a lightbulb. It is related to the energy produced
     *                      and ultimately the brightness by the efficiency parameter.
     *                      This value is often available on the packaging of commercial
     *                      lightbulbs.
     *
     * @param efficiency    Efficiency in percent. This depends on the type of lightbulb used.
     */
    public void setIntensity(@EntityInstance int i, float watts, float efficiency) {
        nSetIntensity(mNativeObject, i , watts, efficiency);
    }

    /**
     * returns the light's luminous intensity in <i>lumens</i>.
     *<p>
     * <b>note:</b> for {@link Type#FOCUSED_SPOT} lights, the returned value depends on the outer cone angle.
     *</p>
     *
     * @param i     Instance of the component obtained from getInstance().
     *
     * @return luminous intensity in <i>lumen</i>.
     */
    public float getIntensity(@EntityInstance int i) {
        return nGetIntensity(mNativeObject, i);
    }

    /**
     * Set the falloff distance for point lights and spot lights.
     *
     * @param i       Instance of the component obtained from getInstance().
     * @param falloff falloff distance in world units. Default is 1 meter.
     *
     * @see Builder#falloff
     */
    public void setFalloff(@EntityInstance int i, float falloff) {
        nSetFalloff(mNativeObject, i, falloff);
    }

    /**
     * returns the falloff distance of this light.
     * @param i     Instance of the component obtained from getInstance().
     * @return      the falloff distance of this light.
     */
    public float getFalloff(@EntityInstance int i) {
        return nGetFalloff(mNativeObject, i);
    }

    /**
     * Dynamically updates a spot light's cone as angles
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param inner inner cone angle in *radians* between 0 and pi/2
     * @param outer outer cone angle in *radians* between inner and pi/2
     *
     * @see Builder#spotLightCone
     */
    public void setSpotLightCone(@EntityInstance int i, float inner, float outer) {
        nSetSpotLightCone(mNativeObject, i, inner, outer);
    }

    /**
     * Dynamically updates the angular radius of a Type.SUN light
     *
     * The Sun as seen from Earth has an angular size of 0.526° to 0.545°
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param angularRadius sun's radius in degrees. Default is 0.545°.
     */
    public void setSunAngularRadius(@EntityInstance int i, float angularRadius) {
        nSetSunAngularRadius(mNativeObject, i, angularRadius);
    }

    /**
     * returns the angular radius if the sun in degrees.
     * @param i     Instance of the component obtained from getInstance().
     * @return the angular radius if the sun in degrees.
     */
    public float getSunAngularRadius(@EntityInstance int i) {
        return nGetSunAngularRadius(mNativeObject, i);
    }

    /**
     * Dynamically updates the halo radius of a Type.SUN light. The radius
     * of the halo is defined as a multiplier of the sun angular radius.
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param haloSize radius multiplier. Default is 10.0.
     */
    public void setSunHaloSize(@EntityInstance int i, float haloSize) {
        nSetSunHaloSize(mNativeObject, i, haloSize);
    }

    /**
     * returns the halo size of a Type.SUN light as a multiplier of the
     * sun angular radius.
     * @param i     Instance of the component obtained from getInstance().
     * @return the halo size
     */
    public float getSunHaloSize(@EntityInstance int i) {
        return nGetSunHaloSize(mNativeObject, i);
    }

    /**
     * Dynamically updates the halo falloff of a Type.SUN light. The falloff
     * is a dimensionless number used as an exponent.
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param haloFalloff halo falloff. Default is 80.0.
     */
    public void setSunHaloFalloff(@EntityInstance int i, float haloFalloff) {
        nSetSunHaloFalloff(mNativeObject, i, haloFalloff);
    }

    /**
     * returns the halo falloff of a Type.SUN light as a dimensionless value.
     * @param i     Instance of the component obtained from getInstance().
     * @return the halo falloff
     */
    public float getSunHaloFalloff(@EntityInstance int i) {
        return nGetSunHaloFalloff(mNativeObject, i);
    }

    /**
     * Whether this Light casts shadows (disabled by default)
     *
     * <p>
     * <b>warning:</b> {@link Type#POINT} cannot cast shadows.
     * </p>
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param shadowCaster Enables or disables casting shadows from this Light.
     */
    public void setShadowCaster(@EntityInstance int i, boolean shadowCaster) {
        nSetShadowCaster(mNativeObject, i, shadowCaster);
    }

    /**
     * returns whether this light casts shadows.
     * @param i     Instance of the component obtained from getInstance().
     */
    public boolean isShadowCaster(@EntityInstance int i) {
        return nIsShadowCaster(mNativeObject, i);
    }

    public float getOuterConeAngle(@EntityInstance int i) {
        return nGetOuterConeAngle(mNativeObject, i);
    }

    public float getInnerConeAngle(@EntityInstance int i) {
        return nGetInnerConeAngle(mNativeObject, i);
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
    private static native void nBuilderShadowOptions(long nativeBuilder, int mapSize,
            int cascades, float[] splitPositions,
             float constantBias, float normalBias,
             float shadowFar, float shadowNearHint, float shadowFarhint,
             boolean stable, boolean lispsm,
             float polygonOffsetConstant, float polygonOffsetSlope,
             boolean screenSpaceContactShadows, int stepCount, float maxShadowDistance,
             boolean elvsm, float blurWidth, float shadowBulbRadius, float[] transform);
    private static native void nBuilderCastLight(long nativeBuilder, boolean enabled);
    private static native void nBuilderPosition(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderDirection(long nativeBuilder, float x, float y, float z);
    private static native void nBuilderColor(long nativeBuilder, float linearR, float linearG, float linearB);
    private static native void nBuilderIntensity(long nativeBuilder, float intensity);
    private static native void nBuilderIntensity(long nativeBuilder, float watts, float efficiency);
    private static native void nBuilderIntensityCandela(long nativeBuilder, float intensity);
    private static native void nBuilderFalloff(long nativeBuilder, float radius);
    private static native void nBuilderSpotLightCone(long nativeBuilder, float inner, float outer);
    private static native void nBuilderAngularRadius(long nativeBuilder, float angularRadius);
    private static native void nBuilderHaloSize(long nativeBuilder, float haloSize);
    private static native void nBuilderHaloFalloff(long nativeBuilder, float haloFalloff);
    private static native void nBuilderLightChannel(long nativeBuilder, int channel, boolean enable);

    private static native void nComputeUniformSplits(float[] splitPositions, int cascades);
    private static native void nComputeLogSplits(float[] splitPositions, int cascades, float near, float far);
    private static native void nComputePracticalSplits(float[] splitPositions, int cascades, float near, float far, float lambda);

    private static native int nGetType(long nativeLightManager, int i);
    private static native void nSetPosition(long nativeLightManager, int i, float x, float y, float z);
    private static native void nGetPosition(long nativeLightManager, int i, float[] out);
    private static native void nSetDirection(long nativeLightManager, int i, float x, float y, float z);
    private static native void nGetDirection(long nativeLightManager, int i, float[] out);
    private static native void nSetColor(long nativeLightManager, int i, float linearR, float linearG, float linearB);
    private static native void nGetColor(long nativeLightManager, int i, float[] out);
    private static native void nSetIntensity(long nativeLightManager, int i, float intensity);
    private static native void nSetIntensity(long nativeLightManager, int i, float watts, float efficiency);
    private static native void nSetIntensityCandela(long nativeLightManager, int i, float intensity);
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
    private static native float nGetOuterConeAngle(long nativeLightManager, int i);
    private static native float nGetInnerConeAngle(long nativeLightManager, int i);
    private static native void nSetLightChannel(long nativeLightManager, int i, int channel, boolean enable);
    private static native boolean nGetLightChannel(long nativeLightManager, int i, int channel);
}
