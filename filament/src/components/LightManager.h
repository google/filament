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

#ifndef TNT_FILAMENT_DETAILS_LIGHTMANAGER_H
#define TNT_FILAMENT_DETAILS_LIGHTMANAGER_H

#include "upcast.h"

#include "private/backend/DriverApiForward.h"

#include <filament/LightManager.h>

#include <utils/Entity.h>
#include <utils/SingleInstanceComponentManager.h>

#include <math/mat4.h>

namespace filament {
namespace details {

class FEngine;
class FScene;

class FLightManager : public LightManager {
public:
    using Instance = LightManager::Instance;

    explicit FLightManager(FEngine& engine) noexcept;
    ~FLightManager();

    void init(FEngine& engine) noexcept;

    void terminate() noexcept;

    bool hasComponent(utils::Entity e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity e) const noexcept {
        return mManager.getInstance(e);
    }

    void create(const FLightManager::Builder& builder, utils::Entity entity);

    void destroy(utils::Entity e) noexcept;

    void prepare(backend::DriverApi& driver) const noexcept;

    void gc(utils::EntityManager& em) noexcept {
        mManager.gc(em);
    }

    struct LightType {
        Type type : 3;
        bool shadowCaster : 1;
        bool lightCaster : 1;
    };

    struct SpotParams {
        float radius = 0;
        float cosOuterSquared = 1;
        float sinInverse = std::numeric_limits<float>::infinity();
        float luminousPower = 0;
        math::float2 scaleOffset = {};
    };

    struct ShadowParams {
        LightManager::ShadowOptions options;
    };

    UTILS_NOINLINE void setLocalPosition(Instance i, const math::float3& position) noexcept;
    UTILS_NOINLINE void setLocalDirection(Instance i, math::float3 direction) noexcept;
    UTILS_NOINLINE void setColor(Instance i, const LinearColor& color) noexcept;
    UTILS_NOINLINE void setSpotLightCone(Instance i, float inner, float outer) noexcept;
    UTILS_NOINLINE void setIntensity(Instance i, float intensity) noexcept;
    UTILS_NOINLINE void setFalloff(Instance i, float radius) noexcept;
    UTILS_NOINLINE void setSunAngularRadius(Instance i, float angularRadius) noexcept;
    UTILS_NOINLINE void setSunHaloSize(Instance i, float haloSize) noexcept;
    UTILS_NOINLINE void setSunHaloFalloff(Instance i, float haloFalloff) noexcept;

    constexpr LightType const& getLightType(Instance i) const noexcept {
        return mManager[i].lightType;
    }

    constexpr Type getType(Instance i) const noexcept {
        return getLightType(i).type;
    }

    constexpr bool isShadowCaster(Instance i) const noexcept {
        return getLightType(i).shadowCaster;
    }

    constexpr bool isLightCaster(Instance i) const noexcept {
        return getLightType(i).lightCaster;
    }

    constexpr bool isPointLight(Instance i) const noexcept { 
        return getType(i) == Type::POINT; 
    }

    constexpr bool isSpotLight(Instance i) const noexcept {
        Type type = getType(i);
        return type == Type::FOCUSED_SPOT || type == Type::SPOT;
    }

    constexpr bool isDirectionalLight(Instance i) const noexcept {
        Type type = getType(i);
        return type == Type::DIRECTIONAL || type == Type::SUN;
    }

    constexpr bool isIESLight(Instance i) const noexcept {
        return false;   // TODO: change this when we support IES lights
    }

    constexpr bool isSunLight(Instance i) const noexcept {
        return getType(i) == Type::SUN; 
    }

    constexpr uint32_t getShadowMapSize(Instance i) const noexcept {
        return getShadowParams(i).options.mapSize;
    }

    constexpr ShadowParams const& getShadowParams(Instance i) const noexcept {
        return mManager[i].shadowParams;
    }

    constexpr float getShadowConstantBias(Instance i) const noexcept {
        return getShadowParams(i).options.constantBias;
    }

    constexpr float getShadowNormalBias(Instance i) const noexcept {
        return getShadowParams(i).options.normalBias;
    }

    constexpr float getShadowFar(Instance i) const noexcept {
        return getShadowParams(i).options.shadowFar;
    }

    constexpr const math::float3& getColor(Instance i) const noexcept {
        return mManager[i].color;
    }

    constexpr float getIntensity(Instance i) const noexcept {
        return mManager[i].intensity;
    }

    constexpr float getSunAngularRadius(Instance i) const noexcept {
        return mManager[i].sunAngularRadius;
    }

    constexpr float getSunHaloSize(Instance i) const noexcept {
        return mManager[i].sunHaloSize;
    }

    constexpr float getSunHaloFalloff(Instance i) const noexcept {
        return mManager[i].sunHaloFalloff;
    }

    constexpr float getSquaredFalloffInv(Instance i) const noexcept {
        return mManager[i].squaredFallOffInv;
    }

    constexpr SpotParams const& getSpotParams(Instance i) const noexcept {
        return mManager[i].spotParams;
    }

    constexpr float getCosOuterSquared(Instance i) const noexcept {
        return getSpotParams(i).cosOuterSquared;
    }

    constexpr float getSinInverse(Instance i) const noexcept {
        return getSpotParams(i).sinInverse;
    }

    constexpr float getRadius(Instance i) const noexcept {
        return getSpotParams(i).radius;
    }

    constexpr const math::float3& getLocalPosition(Instance i) const noexcept {
        return mManager[i].position;
    }

    constexpr const math::float3& getLocalDirection(Instance i) const noexcept {
        return mManager[i].direction;
    }

    const ShadowOptions& getShadowOptions(Instance i) const noexcept {
        return getShadowParams(i).options;
    }

    void setShadowOptions(Instance i, ShadowOptions const& options) noexcept {
        static_cast<ShadowParams&>(mManager[i].shadowParams).options = options;
    }

private:
    friend class FScene;

    enum {
        LIGHT_TYPE,         // light type
        POSITION,           // position in local-space (i.e. pre-transform)
        DIRECTION,          // direction in local-space (i.e. pre-transform)
        COLOR,              // color
        SHADOW_PARAMS,      // state needed for shadowing
        SPOT_PARAMS,        // state needed for spot lights
        SUN_ANGULAR_RADIUS, // state for the directional light sun
        SUN_HALO_SIZE,      // state for the directional light sun
        SUN_HALO_FALLOFF,   // state for the directional light sun
        INTENSITY,
        FALLOFF,
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
            float           //  4
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        struct Proxy {
            // all of this gets inlined
            UTILS_ALWAYS_INLINE
            constexpr Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
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
            };
        };

        UTILS_ALWAYS_INLINE constexpr Proxy operator[](Instance i) noexcept {
            return { *this, i };
        }
        UTILS_ALWAYS_INLINE constexpr const Proxy operator[](Instance i) const noexcept {
            return { const_cast<Sim&>(*this), i };
        }
    };

    Sim mManager;
    FEngine& mEngine;
};

FILAMENT_UPCAST(LightManager)


} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_LIGHTMANAGER_H
