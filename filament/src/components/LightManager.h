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

#ifndef TNT_FILAMENT_COMPONENTS_LIGHTMANAGER_H
#define TNT_FILAMENT_COMPONENTS_LIGHTMANAGER_H

#include "downcast.h"

#include "backend/DriverApiForward.h"

#include <filament/LightManager.h>

#include <utils/Entity.h>
#include <utils/SingleInstanceComponentManager.h>

#include <math/mat4.h>

namespace filament {

class FEngine;
class FScene;

class FLightManager : public LightManager {
public:
    using Instance = Instance;

    explicit FLightManager(FEngine& engine) noexcept;
    ~FLightManager();

    void init(FEngine& engine) noexcept;

    void terminate() noexcept;

    void gc(utils::EntityManager& em) noexcept;

    /*
     * Component Manager APIs
     */

    bool hasComponent(utils::Entity e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity e) const noexcept {
        return { mManager.getInstance(e) };
    }

    size_t getComponentCount() const noexcept {
        return mManager.getComponentCount();
    }

    bool empty() const noexcept {
        return mManager.empty();
    }

    utils::Entity getEntity(Instance i) const noexcept {
        return mManager.getEntity(i);
    }

    utils::Entity const* getEntities() const noexcept {
        return mManager.getEntities();
    }

    void create(const Builder& builder, utils::Entity entity);

    void destroy(utils::Entity e) noexcept;

    void prepare(backend::DriverApi& driver) const noexcept;

    struct LightType {
        Type type : 3;
        bool shadowCaster : 1;
        bool lightCaster : 1;
    };

    struct SpotParams {
        float radius = 0;
        float outerClamped = 0;
        float cosOuterSquared = 1;
        float sinInverse = std::numeric_limits<float>::infinity();
        float luminousPower = 0;
        math::float2 scaleOffset = {};
    };

    enum class IntensityUnit {
        LUMEN_LUX,  // intensity specified in lumens (for punctual lights) or lux (for directional)
        CANDELA     // intensity specified in candela (only applicable to punctual lights)
    };

    struct ShadowParams { // TODO: get rid of this struct
        ShadowOptions options;
    };

    UTILS_NOINLINE void setLocalPosition(Instance i, const math::float3& position) noexcept;
    UTILS_NOINLINE void setLocalDirection(Instance i, math::float3 direction) noexcept;
    UTILS_NOINLINE void setLightChannel(Instance i, unsigned int channel, bool enable) noexcept;
    UTILS_NOINLINE void setColor(Instance i, const LinearColor& color) noexcept;
    UTILS_NOINLINE void setSpotLightCone(Instance i, float inner, float outer) noexcept;
    UTILS_NOINLINE void setIntensity(Instance i, float intensity, IntensityUnit unit) noexcept;
    UTILS_NOINLINE void setFalloff(Instance i, float radius) noexcept;
    UTILS_NOINLINE void setShadowCaster(Instance i, bool shadowCaster) noexcept;
    UTILS_NOINLINE void setSunAngularRadius(Instance i, float angularRadius) noexcept;
    UTILS_NOINLINE void setSunHaloSize(Instance i, float haloSize) noexcept;
    UTILS_NOINLINE void setSunHaloFalloff(Instance i, float haloFalloff) noexcept;

    UTILS_NOINLINE bool getLightChannel(Instance i, unsigned int channel) const noexcept;

    LightType const& getLightType(Instance i) const noexcept {
        return mManager[i].lightType;
    }

    Type getType(Instance i) const noexcept {
        return getLightType(i).type;
    }

    bool isShadowCaster(Instance i) const noexcept {
        return getLightType(i).shadowCaster;
    }

    bool isLightCaster(Instance i) const noexcept {
        return getLightType(i).lightCaster;
    }

    bool isPointLight(Instance i) const noexcept {
        return getType(i) == Type::POINT; 
    }

    bool isSpotLight(Instance i) const noexcept {
        Type const type = getType(i);
        return type == Type::FOCUSED_SPOT || type == Type::SPOT;
    }

    bool isDirectionalLight(Instance i) const noexcept {
        Type const type = getType(i);
        return type == Type::DIRECTIONAL || type == Type::SUN;
    }

    bool isIESLight(Instance i) const noexcept {
        return false;   // TODO: change this when we support IES lights
    }

    bool isSunLight(Instance i) const noexcept {
        return getType(i) == Type::SUN; 
    }

    uint32_t getShadowMapSize(Instance i) const noexcept {
        return getShadowParams(i).options.mapSize;
    }

    ShadowParams const& getShadowParams(Instance i) const noexcept {
        return mManager[i].shadowParams;
    }

    float getShadowConstantBias(Instance i) const noexcept {
        return getShadowParams(i).options.constantBias;
    }

    float getShadowNormalBias(Instance i) const noexcept {
        return getShadowParams(i).options.normalBias;
    }

    float getShadowFar(Instance i) const noexcept {
        return getShadowParams(i).options.shadowFar;
    }

    const math::float3& getColor(Instance i) const noexcept {
        return mManager[i].color;
    }

    float getIntensity(Instance i) const noexcept {
        return mManager[i].intensity;
    }

    float getSunAngularRadius(Instance i) const noexcept {
        return mManager[i].sunAngularRadius;
    }

    float getSunHaloSize(Instance i) const noexcept {
        return mManager[i].sunHaloSize;
    }

    float getSunHaloFalloff(Instance i) const noexcept {
        return mManager[i].sunHaloFalloff;
    }

    float getSquaredFalloffInv(Instance i) const noexcept {
        return mManager[i].squaredFallOffInv;
    }

    float getFalloff(Instance i) const noexcept {
        return getRadius(i);
    }

    SpotParams const& getSpotParams(Instance i) const noexcept {
        return mManager[i].spotParams;
    }

    float getSpotLightInnerCone(Instance i) const noexcept;

    float getCosOuterSquared(Instance i) const noexcept {
        return getSpotParams(i).cosOuterSquared;
    }

    float getSinInverse(Instance i) const noexcept {
        return getSpotParams(i).sinInverse;
    }

    float getRadius(Instance i) const noexcept {
        return getSpotParams(i).radius;
    }

    uint8_t getLightChannels(Instance i) const noexcept {
        return mManager[i].channels;
    }

    const math::float3& getLocalPosition(Instance i) const noexcept {
        return mManager[i].position;
    }

    const math::float3& getLocalDirection(Instance i) const noexcept {
        return mManager[i].direction;
    }

    const ShadowOptions& getShadowOptions(Instance i) const noexcept {
        return getShadowParams(i).options;
    }

    void setShadowOptions(Instance i, ShadowOptions const& options) noexcept;

private:
    friend class FScene;

    enum {
        LIGHT_TYPE,         // light type
        POSITION,           // position in local-space (i.e. pre-transform)
        DIRECTION,          // direction in local-space (i.e. pre-transform)
        COLOR,              // color
        SHADOW_PARAMS,      // state needed for shadowing
        SPOT_PARAMS,        // state needed for spotlights
        SUN_ANGULAR_RADIUS, // state for the directional light sun
        SUN_HALO_SIZE,      // state for the directional light sun
        SUN_HALO_FALLOFF,   // state for the directional light sun
        INTENSITY,
        FALLOFF,
        CHANNELS,
    };

    using Base = utils::SingleInstanceComponentManager<  // 120 bytes
            LightType,      //  1
            math::float3,   // 12
            math::float3,   // 12
            math::float3,   // 12
            ShadowParams,   // 12
            SpotParams,     // 24
            float,          //  4
            float,          //  4
            float,          //  4
            float,          //  4
            float,          //  4
            uint8_t         //  1
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        struct Proxy {
            // all of this gets inlined
            UTILS_ALWAYS_INLINE
            Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
                    : lightType{ sim, i } { }

            union {
                // this specific usage of union is permitted. All fields are identical
                Field<LIGHT_TYPE>           lightType;
                Field<POSITION>             position;
                Field<DIRECTION>            direction;
                Field<COLOR>                color;
                Field<SHADOW_PARAMS>        shadowParams;
                Field<SPOT_PARAMS>          spotParams;
                Field<SUN_ANGULAR_RADIUS>   sunAngularRadius;
                Field<SUN_HALO_SIZE>        sunHaloSize;
                Field<SUN_HALO_FALLOFF>     sunHaloFalloff;
                Field<INTENSITY>            intensity;
                Field<FALLOFF>              squaredFallOffInv;
                Field<CHANNELS>             channels;
            };
        };

        UTILS_ALWAYS_INLINE Proxy operator[](Instance i) noexcept {
            return { *this, i };
        }
        UTILS_ALWAYS_INLINE const Proxy operator[](Instance i) const noexcept {
            return { const_cast<Sim&>(*this), i };
        }
    };

    Sim mManager;
    FEngine& mEngine;
};

FILAMENT_DOWNCAST(LightManager)


} // namespace filament

#endif // TNT_FILAMENT_COMPONENTS_LIGHTMANAGER_H
