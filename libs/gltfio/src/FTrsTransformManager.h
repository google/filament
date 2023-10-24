/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef GLTFIO_FTRSTRANSFORMMANAGER_H
#define GLTFIO_FTRSTRANSFORMMANAGER_H

#include "downcast.h"
#include "gltfio/math.h"
#include "math/quat.h"
#include "utils/debug.h"

#include <gltfio/TrsTransformManager.h>

#include <utils/compiler.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Entity.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Slice.h>

namespace filament::gltfio {

class UTILS_PRIVATE FTrsTransformManager : public TrsTransformManager {
public:
    using Instance = TrsTransformManager::Instance;

    FTrsTransformManager() noexcept {}

    ~FTrsTransformManager() noexcept {
        assert_invariant(mManager.getComponentCount() == 0);
    }

    void terminate() noexcept;

    bool hasComponent(utils::Entity e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity e) const noexcept {
        return Instance(mManager.getInstance(e));
    }

    void create(utils::Entity entity) {
        create(entity, float3{}, quatf{}, float3{1});
    }

    void create(utils::Entity entity, const float3& translation,
                const quatf& rotation, const float3& scale) {
        if (UTILS_UNLIKELY(mManager.hasComponent(entity))) {
            destroy(entity);
        }
        UTILS_UNUSED_IN_RELEASE Instance ci = mManager.addComponent(entity);
        assert_invariant(ci);

        if (ci) {
            setTrs(ci, translation, rotation, scale);
        }
    }

    void destroy(utils::Entity e) noexcept {
        if (Instance const ci = mManager.getInstance(e); ci) {
            mManager.removeComponent(e);
        }
    }

    void gc(utils::EntityManager& em) noexcept {
        mManager.gc(em, [this](Entity e) {
            destroy(e);
        });
    }

    void setTranslation(Instance ci, const float3& translation) noexcept {
        assert_invariant(ci.isValid());
        mManager[ci].translation = translation;
    }

    const float3& getTranslation(Instance ci) const noexcept {
        return mManager[ci].translation;
    }

    void setRotation(Instance ci, const quatf& rotation) noexcept {
        assert_invariant(ci.isValid());
        mManager[ci].rotation = rotation;
    }

    const quatf& getRotation(Instance ci) const noexcept {
        return mManager[ci].rotation;
    }

    void setScale(Instance ci, const float3& scale) noexcept {
        assert_invariant(ci.isValid());
        mManager[ci].scale = scale;
    }

    const float3& getScale(Instance ci) const noexcept {
        return mManager[ci].scale;
    }

    void setTrs(Instance ci, const float3& translation,
            const quatf& rotation, const float3& scale) noexcept {
        setTranslation(ci, translation);
        setRotation(ci, rotation);
        setScale(ci, scale);
    }

    const mat4f getTransform(Instance ci) const noexcept {
        return composeMatrix(getTranslation(ci), getRotation(ci), getScale(ci));
    }

private:
    enum {
        TRANSLATION,
        ROTATION,
        SCALE,
    };

    using Base = utils::SingleInstanceComponentManager<
            float3,
            quatf,
            float3>;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        typename Base::SoA& getSoA() { return mData; }

        struct Proxy {
            UTILS_ALWAYS_INLINE
            Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept :
                    translation{ sim, i } { }

            union {
                Field<TRANSLATION>   translation;
                Field<ROTATION>      rotation;
                Field<SCALE>         scale;
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
};

FILAMENT_DOWNCAST(TrsTransformManager)

} // namespace filament::gltfio

#endif // GLTFIO_FTRSTRANSFORMMANAGER_H
