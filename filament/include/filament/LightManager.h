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

#ifndef TNT_FILAMENT_LIGHTMANAGER_H
#define TNT_FILAMENT_LIGHTMANAGER_H

#include <filament/FilamentAPI.h>
#include <filament/Color.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/EntityInstance.h>

#include <math/vec3.h>

#include <math.h>

namespace filament {

class Engine;

namespace details {
class FEngine;
class FLightManager;
} // namespace details


/**
 * LightManager allows to create a light source in the scene, such as a sun or street lights.
 *
 * At least one light must be added to a scene in order to see anything
 * (unless the Material.Shading.UNLIT is used).
 *
 *
 * Creation and destruction
 * ========================
 *
 * A Light component is created using the LightManager::Builder and destroyed by calling
 * LightManager::destroy(utils::Entity).
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *  utils::Entity sun = utils::EntityManager.get().create();
 *
 *  filament::LightManager::Builder(Type::SUN)
 *              .castShadows(true)
 *              .build(*engine, sun);
 *
 *  engine->getLightManager().destroy(sun);
 * ~~~~~~~~~~~
 *
 *
 * Light types
 * ===========
 *
 * Lights come in three flavors:
 * - directional lights
 * - point lights
 * - spot lights
 *
 *
 * Directional lights
 * ------------------
 *
 * Directional lights have a direction, but don't have a position. All light rays are
 * parallel and come from infinitely far away and from everywhere. Typically a directional light
 * is used to simulate the sun.
 *
 * Directional lights are able to cast shadows.
 *
 * To create a directional light use Type.DIRECTIONAL or Type.SUN, both are similar, but the later
 * also draws a sun's disk in the sky and its reflection on glossy objects.
 *
 * @warning Currently, only a single directional light is supported. If several directional lights
 * are added to the scene, the dominant one will be used.
 *
 * @see Builder.direction(), Builder.sunAngularRadius()
 *
 * Point lights
 * ------------
 *
 * Unlike directional lights, point lights have a position but emit light in all directions.
 * The intensity of the light diminishes with the inverse square of the distance to the light.
 * Builder.falloff() controls distance beyond which the light has no more influence.
 *
 * A scene can have multiple point lights.
 *
 * @see Builder.position(), Builder.falloff()
 *
 * Spot lights
 * -----------
 *
 * Spot lights are similar to point lights but the light it emits is limited to a cone defined by
 * Builder.spotLightCone() and the light's direction.
 *
 * A spot light is therefore defined by a position, a direction and inner and outer cones. The
 * spot light's influence is limited to inside the outer cone. The inner cone defines the light's
 * falloff attenuation.
 *
 * A physically correct spot light is a little difficult to use because changing the outer angle
 * of the cone changes the illumination levels, as the same amount of light is spread over a
 * changing volume. The coupling of illumination and the outer cone means that an artist cannot
 * tweak the influence cone of a spot light without also changing the perceived illumination.
 * It therefore makes sense to provide artists with a parameter to disable this coupling. This
 * is the difference between Type.SPOT and Type.FOCUSED_SPOT.
 *
 * @see Builder.position(), Builder.direction(), Builder.falloff(), Builder.spotLightCone()
 *
 * Performance considerations
 * ==========================
 *
 * Generally, adding lights to the scene hurts performance, however filament is designed to be
 * able to handle hundreds of lights in a scene under certain conditions. Here are some tips
 * to keep performances high.
 *
 * 1. Prefer spot lights to point lights and use the smallest outer cone angle possible.
 *
 * 2. Use the smallest possible falloff distance for point and spot lights.
 *    Performance is very sensitive to overlapping lights. The falloff distance essentially
 *    defines a sphere of influence for the light, so try to position point and spot lights
 *    such that they don't overlap too much.
 *
 *    On the other hand, a scene can contain hundreds of non overlapping lights without
 *    incurring a significant overhead.
 *
 */
class UTILS_PUBLIC LightManager : public FilamentAPI {
    struct BuilderDetails;

public:
    using Instance = utils::EntityInstance<LightManager>;

    /**
     * Returns whether a particular Entity is associated with a component of this LightManager
     * @param e An Entity.
     * @return true if this Entity has a component associated with this manager.
     */
    bool hasComponent(utils::Entity e) const noexcept;

    /**
     * Gets an Instance representing the Light component associated with the given Entity.
     * @param e An Entity.
     * @return An Instance object, which represents the Light component associated with the Entity e.
     * @note Use Instance::isValid() to make sure the component exists.
     * @see hasComponent()
     */
    Instance getInstance(utils::Entity e) const noexcept;

    // destroys this component from the given entity
    void destroy(utils::Entity e) noexcept;


    //! Denotes the type of the light being created.
    enum class Type : uint8_t {
        SUN,            //!< Directional light that also draws a sun's disk in the sky.
        DIRECTIONAL,    //!< Directional light, emits light in a given direction.
        POINT,          //!< Point light, emits light from a position, in all directions.
        FOCUSED_SPOT,   //!< Spot light with coupling of outer cone and illumination disabled.
        SPOT,           //!< Physically correct spot light.
    };

    /**
     * Control the quality / performance of the shadow map associated to this light
     */
    struct ShadowOptions {
        /** size of the shadow map in texels. Must be a power-of-two. */
        uint32_t mapSize = 1024;

        /** constant bias in world units (e.g. meters) by which shadow are moved away from the
         * light. Value must be between 0 and 2.
         */
        float constantBias = 0.01f;

        /** Amount by which the maximum sampling error is scaled. The resulting value is used
         * to move the shadow away from the fragment normal. Must be between 0 and 3.
         */
        float normalBias = 0.4f;

        /** Distance from the camera after which shadows are clipped. this is used to clip
         * shadows that are too far and wouldn't contribute to the scene much, improving
         * performance and quality. This value is always positive.
         * Use 0.0f to use the camera far distance.
         */
        float shadowFar = 0.0f;

        /** Optimize the quality of shadows from this distance from the camera. Shadows will
         * be rendered in front of this distance, but the quality may not be optimal.
         * This value is always positive. Use 0.0f to use the camera near distance.
         * The default of 1m works well with many scenes. The quality of shadows may drop
         * rapidly when this value decreases.
         */
        float shadowNearHint = 1.0f;

        /** Optimize the quality of shadows in front of this distance from the camera. Shadows
         * will be rendered behind this distance, but the quality may not be optimal.
         * This value is always positive. Use std::numerical_limits<float>::infinity() to
         * use the camera far distance.
         */
        float shadowFarHint = 100.0f;
    };

    //! Use Builder to construct a Light object instance
    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        /**
         * Creates a light builder and set the light's #Type.
         *
         * @param type #Type of Light object to create.
         */
        explicit Builder(Type type) noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Whether this Light casts shadows (disabled by default)
         *
         * @param enable Enables or disables casting shadows from this Light.
         *
         * @return This Builder, for chaining calls.
         *
         * @warning
         * - Only a Type.DIRECTIONAL or Type.SUN light can cast shadows
         */
        Builder& castShadows(bool enable) noexcept;

        Builder& shadowOptions(const ShadowOptions& options) noexcept;

        /**
         * Whether this light casts light (enabled by default)
         *
         * @param enable Enables or disables lighting from this Light.
         *
         * @return This Builder, for chaining calls.
         *
         * @note
         * In some situations it can be useful to have a light in the scene that doesn't
         * actually emit light, but does cast shadows.
         */
        Builder& castLight(bool enable) noexcept;

        /**
         * Sets the initial position of the light in world space.
         *
         * @param position Light's position in world space. The default is at the origin.
         *
         * @return This Builder, for chaining calls.
         *
         * @note
         * The Light's position is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)
         */
        Builder& position(const math::float3& position) noexcept;

        /**
         * Sets the initial direction of a light in world space.
         *
         * @param direction Light's direction in world space. Should be a unit vector.
         *                  The default is {0,-1,0}.
         *
         * @return This Builder, for chaining calls.
         *
         * @note
         * The Light's direction is ignored for Type.POINT lights.
         */
        Builder& direction(const math::float3& direction) noexcept;

        /**
         * Sets the initial color of a light.
         *
         * @param color Color of the light specified in the linear sRGB color-space.
         *              The default is white {1,1,1}.
         *
         * @return This Builder, for chaining calls.
         */
        Builder& color(const LinearColor& color) noexcept;

        /**
         * Sets the initial intensity of a light.
         * @param intensity This parameter depends on the Light.Type:
         *                  - For directional lights, it specifies the illuminance in *lux*
         *                  (or *lumen/m^2*).
         *                  - For point lights and spot lights, it specifies the luminous power
         *                  in *lumen*.
         *
         * @return This Builder, for chaining calls.
         *
         * For example, the sun's illuminance is about 100,000 lux.
         *
         */
        Builder& intensity(float intensity) noexcept;

        /**
         * Sets the initial intensity of a light in watts.
         *
         * @param watts         Energy consumed by a lightbulb. It is related to the energy produced
         *                      and ultimately the brightness by the \p efficiency parameter.
         *                      This value is often available on the packaging of commercial
         *                      lightbulbs.
         *
         * @param efficiency    Efficiency in percent. This depends on the type of lightbulb used.
         *
         *  Lightbulb type  | Efficiency
         * ----------------:|-----------:
         *     Incandescent |  2.2%
         *         Halogen  |  7.0%
         *             LED  |  8.7%
         *     Fluorescent  | 10.7%
         *
         * @return This Builder, for chaining calls.
         *
         *
         * @note
         * This call is equivalent to `Builder::intensity(efficiency * 683 * watts);`
         */
        Builder& intensity(float watts, float efficiency) noexcept;

        /**
         * Set the falloff distance for point lights and spot lights.
         *
         * At the falloff distance, the light has no more effect on objects.
         *
         * The falloff distance essentially defines a *sphere of influence* around the light, and
         * therefore has an impact on performance. Larger falloffs might reduce performance
         * significantly, especially when many lights are used.
         *
         * Try to avoid having a large number of light's spheres of influence overlap.
         *
         * @param radius Falloff distance in world units. Default is 1 meter.
         *
         * @return This Builder, for chaining calls.
         *
         * @note
         * The Light's falloff is ignored for directional lights (Type.DIRECTIONAL or Type.SUN)
         */
        Builder& falloff(float radius) noexcept;

        /**
         * Defines a spot light'st angular falloff attenuation.
         *
         * A spot light is defined by a position, a direction and two cone angles,
         * \p inner and \p outer. These two angles are used to define the angular falloff
         * attenuation of the spot light.
         *
         * @param inner inner cone angle in *radians* between 0 and @f$ \pi @f$

         * @param outer outer cone angle in *radians* between 0 and @f$ \pi @f$

         * @return This Builder, for chaining calls.
         *
         * @note
         * The spot light cone is ignored for directional and point lights.
         *
         * @see Type.SPOT, Type.FOCUSED_SPOT
         */
        Builder& spotLightCone(float inner, float outer) noexcept;

        /**
         * Defines the angular radius of the sun, in degrees, between 0.25° and 20.0°
         *
         * The Sun as seen from Earth has an angular size of 0.526° to 0.545°
         *
         * @param angularRadius sun's radius in degree. Default is 0.545°.

         * @return This Builder, for chaining calls.
         */
        Builder& sunAngularRadius(float angularRadius) noexcept;

        /**
         * Defines the halo radius of the sun. The radius of the halo is defined as a
         * multiplier of the sun angular radius.
         *
         * @param haloSize radius multiplier. Default is 10.0.
         *
         * @return This Builder, for chaining calls.
         */
        Builder& sunHaloSize(float haloSize) noexcept;

        /**
         * Defines the halo falloff of the sun. The falloff is a dimensionless number
         * used as an exponent.
         *
         * @param haloFalloff halo falloff. Default is 80.0.
         *
         * @return This Builder, for chaining calls.
         */
        Builder& sunHaloFalloff(float haloFalloff) noexcept;

        enum Result { Error = -1, Success = 0  };

        /**
         * Adds the Light component to an entity.
         *
         * @param engine Reference to the filament::Engine to associate this light with.
         * @param entity Entity to add the light component to.
         * @return Success if the component was created successfully, Error otherwise.
         *
         * If exceptions are disabled and an error occurs, this function is a no-op.
         *        Success can be checked by looking at the return value.
         *
         * If this component already exists on the given entity, it is first destroyed as if
         * destroy(utils::Entity e) was called.
         *
         * @warning
         * Currently, only 2048 lights can be created on a given Engine.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        Result build(Engine& engine, utils::Entity entity);

    private:
        friend class details::FEngine;
        friend class details::FLightManager;
    };

    static constexpr float EFFICIENCY_INCANDESCENT = 0.0220f;   //!< Typical efficiency of an incandescent light bulb (2.2%)
    static constexpr float EFFICIENCY_HALOGEN      = 0.0707f;   //!< Typical efficiency of an halogen light bulb (7.0%)
    static constexpr float EFFICIENCY_FLUORESCENT  = 0.0878f;   //!< Typical efficiency of a fluorescent light bulb (8.7%)
    static constexpr float EFFICIENCY_LED          = 0.1171f;   //!< Typical efficiency of a LED light bulb (11.7%)

    Type getType(Instance i) const noexcept;

    /**
     * Helper function that returns if a light is a directional light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of directional light
     */
    inline bool isDirectional(Instance i) const noexcept {
        Type type = getType(i);
        return type == Type::DIRECTIONAL || type == Type::SUN;
    }

    /**
     * Helper function that returns if a light is a point light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of point light
     */
    inline bool isPointLight(Instance i) const noexcept {
        return getType(i) == Type::POINT;
    }

    /**
     * Helper function that returns if a light is a spot light
     *
     * @param i     Instance of the component obtained from getInstance().
     * @return      true is this light is a type of spot light
     */
    inline bool isSpotLight(Instance i) const noexcept {
        Type type = getType(i);
        return type == Type::SPOT || type == Type::FOCUSED_SPOT;
    }

    /**
     * Dynamically updates the light's position.
     *
     * @param i        Instance of the component obtained from getInstance().
     * @param position Light's position in world space. The default is at the origin.
     *
     * @see Builder.position()
     */
    void setPosition(Instance i, const math::float3& position) noexcept;

    //! returns the light's position in world space
    const math::float3& getPosition(Instance i) const noexcept;

    /**
     * Dynamically updates the light's direction
     *
     * @param i         Instance of the component obtained from getInstance().
     * @param direction Light's direction in world space. Should be a unit vector.
     *                  The default is {0,-1,0}.
     *
     * @see Builder.direction()
     */
    void setDirection(Instance i, const math::float3& direction) noexcept;

    //! returns the light's direction in world space
    const math::float3& getDirection(Instance i) const noexcept;

    /**
     * Dynamically updates the light's hue as linear sRGB
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param color Color of the light specified in the linear sRGB color-space.
     *              The default is white {1,1,1}.
     *
     * @see Builder.color(), getInstance()
     */
    void setColor(Instance i, const LinearColor& color) noexcept;

    /**
     * @param i     Instance of the component obtained from getInstance().
     * @return the light's color in linear sRGB
     */
    const math::float3& getColor(Instance i) const noexcept;

    /**
     * Dynamically updates the light's intensity. The intensity can be negative.
     *
     * @param i         Instance of the component obtained from getInstance().
     * @param intensity This parameter depends on the Light.Type:
     *                  - For directional lights, it specifies the illuminance in *lux*
     *                  (or *lumen/m^2*).
     *                  - For point lights and spot lights, it specifies the luminous power
     *                  in *lumen*.
     *
     * @see Builder.intensity()
     */
    void setIntensity(Instance i, float intensity) noexcept;

    /**
     * Dynamically updates the light's intensity. The intensity can be negative.
     *
     * @param i             Instance of the component obtained from getInstance().
     * @param watts         Energy consumed by a lightbulb. It is related to the energy produced
     *                      and ultimately the brightness by the \p efficiency parameter.
     *                      This value is often available on the packaging of commercial
     *                      lightbulbs.
     * @param efficiency    Efficiency in percent. This depends on the type of lightbulb used.
     *
     *  Lightbulb type  | Efficiency
     * ----------------:|-----------:
     *     Incandescent |  2.2%
     *         Halogen  |  7.0%
     *             LED  |  8.7%
     *     Fluorescent  | 10.7%
     *
     * @see Builder.intensity(float watts, float efficiency)
     */
    void setIntensity(Instance i, float watts, float efficiency) noexcept {
        setIntensity(i, watts * 683.0f * efficiency);
    }

    /**
     * returns the light's luminous intensity in lumen.
     *
     * @param i     Instance of the component obtained from getInstance().
     *
     * @note for Type.FOCUSED_SPOT lights, the returned value depends on the \p outer cone angle.
     *
     * @return luminous intensity in lumen.
     */
    float getIntensity(Instance i) const noexcept;

    /**
     * Set the falloff distance for point lights and spot lights.
     *
     * @param i      Instance of the component obtained from getInstance().
     * @param radius falloff distance in world units. Default is 1 meter.
     *
     * @see Builder.falloff()
     */
    void setFalloff(Instance i, float radius) noexcept;

    /**
     * returns the falloff distance of this light.
     * @param i     Instance of the component obtained from getInstance().
     * @return the falloff distance of this light.
     */
    float getFalloff(Instance i) const noexcept;

    /**
     * Dynamically updates a spot light's cone as angles
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param inner inner cone angle in *radians* between 0 and @f$ \pi @f$
     * @param outer outer cone angle in *radians* between 0 and @f$ \pi @f$
     *
     * @see Builder.spotLightCone()
     */
    void setSpotLightCone(Instance i, float inner, float outer) noexcept;

    /**
     * Dynamically updates the angular radius of a Type.SUN light
     *
     * The Sun as seen from Earth has an angular size of 0.526° to 0.545°
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param angularRadius sun's radius in degrees. Default is 0.545°.
     */
    void setSunAngularRadius(Instance i, float angularRadius) noexcept;

    /**
     * returns the angular radius if the sun in degrees.
     * @param i     Instance of the component obtained from getInstance().
     * @return the angular radius if the sun in degrees.
     */
    float getSunAngularRadius(Instance i) const noexcept;

    /**
     * Dynamically updates the halo radius of a Type.SUN light. The radius
     * of the halo is defined as a multiplier of the sun angular radius.
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param haloSize radius multiplier. Default is 10.0.
     */
    void setSunHaloSize(Instance i, float haloSize) noexcept;

    /**
     * returns the halo size of a Type.SUN light as a multiplier of the
     * sun angular radius.
     * @param i     Instance of the component obtained from getInstance().
     * @return the halo size
     */
    float getSunHaloSize(Instance i) const noexcept;

    /**
     * Dynamically updates the halo falloff of a Type.SUN light. The falloff
     * is a dimensionless number used as an exponent.
     *
     * @param i     Instance of the component obtained from getInstance().
     * @param haloFalloff halo falloff. Default is 80.0.
     */
    void setSunHaloFalloff(Instance i, float haloFalloff) noexcept;

    /**
     * returns the halo falloff of a Type.SUN light as a dimensionless value.
     * @param i     Instance of the component obtained from getInstance().
     * @return the halo falloff
     */
    float getSunHaloFalloff(Instance i) const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_LIGHTMANAGER_H
