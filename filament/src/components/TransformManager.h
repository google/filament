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

#ifndef TNT_FILAMENT_COMPONENTS_TRANSFORMMANAGER_H
#define TNT_FILAMENT_COMPONENTS_TRANSFORMMANAGER_H

#include "downcast.h"

#include <filament/TransformManager.h>

#include <utils/compiler.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Entity.h>
#include <utils/Slice.h>

#include <math/mat4.h>

namespace filament {

class UTILS_PRIVATE FTransformManager : public TransformManager {
public:
    using Instance = Instance;

    FTransformManager() noexcept;
    ~FTransformManager() noexcept;

    // free-up all resources
    void terminate() noexcept;


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

    void setAccurateTranslationsEnabled(bool enable) noexcept;

    bool isAccurateTranslationsEnabled() const noexcept {
        return mAccurateTranslations;
    }

    void create(utils::Entity entity);

    void create(utils::Entity entity, Instance parent, const math::mat4f& localTransform);

    void create(utils::Entity entity, Instance parent, const math::mat4& localTransform);

    void destroy(utils::Entity e) noexcept;

    void setParent(Instance i, Instance newParent) noexcept;

    utils::Entity getParent(Instance i) const noexcept;

    size_t getChildCount(Instance i) const noexcept;

    size_t getChildren(Instance i, utils::Entity* children, size_t count) const noexcept;

    children_iterator getChildrenBegin(Instance parent) const noexcept;

    children_iterator getChildrenEnd(Instance parent) const noexcept;

    void openLocalTransformTransaction() noexcept;

    void commitLocalTransformTransaction() noexcept;

    void gc(utils::EntityManager& em) noexcept;

    utils::Slice<const math::mat4f> getWorldTransforms() const noexcept {
        return mManager.slice<WORLD>();
    }

    void setTransform(Instance ci, const math::mat4f& model) noexcept;

    void setTransform(Instance ci, const math::mat4& model) noexcept;

    const math::mat4f& getTransform(Instance ci) const noexcept {
        return mManager[ci].local;
    }

    const math::mat4f& getWorldTransform(Instance ci) const noexcept {
        return mManager[ci].world;
    }

    math::mat4 getTransformAccurate(Instance ci) const noexcept {
        math::mat4f const& local = mManager[ci].local;
        math::float3 const localTranslationLo = mManager[ci].localTranslationLo;
        math::mat4 r(local);
        r[3].xyz += localTranslationLo;
        return r;
    }

    math::mat4 getWorldTransformAccurate(Instance ci) const noexcept {
        math::mat4f const& world = mManager[ci].world;
        math::float3 const worldTranslationLo = mManager[ci].worldTranslationLo;
        math::mat4 r(world);
        r[3].xyz += worldTranslationLo;
        return r;
    }

private:
    struct Sim;

    void validateNode(Instance i) noexcept;
    void removeNode(Instance i) noexcept;
    void updateNode(Instance i) noexcept;
    void updateNodeTransform(Instance i) noexcept;
    void insertNode(Instance i, Instance p) noexcept;
    void swapNode(Instance i, Instance j) noexcept;
    void transformChildren(Sim& manager, Instance firstChild) noexcept;

    void computeAllWorldTransforms() noexcept;

    static void computeWorldTransform(math::mat4f& outWorld, math::float3& inoutWorldTranslationLo,
            math::mat4f const& pt, math::mat4f const& local,
            math::float3 const& ptTranslationLo, math::float3 const& localTranslationLo,
            bool accurate);

    friend class children_iterator;

    enum {
        LOCAL,          // local transform (relative to parent), world if no parent
        WORLD,          // world transform
        LOCAL_LO,       // accurate local translation
        WORLD_LO,       // accurate world translation
        PARENT,         // instance to the parent
        FIRST_CHILD,    // instance to our first child
        NEXT,           // instance to our next sibling
        PREV,           // instance to our previous sibling
    };

    using Base = utils::SingleInstanceComponentManager<
            math::mat4f,    // local
            math::mat4f,    // world
            math::float3,   // accurate local translation
            math::float3,   // accurate world translation
            Instance,       // parent
            Instance,       // firstChild
            Instance,       // next
            Instance        // prev
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        SoA& getSoA() { return mData; }

        struct Proxy {
            // all of these gets inlined
            UTILS_ALWAYS_INLINE
            Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
                    : local{ sim, i } { }

            union {
                // this specific usage of union is permitted. All fields are identical
                Field<LOCAL>        local;
                Field<WORLD>        world;
                Field<LOCAL_LO>     localTranslationLo;
                Field<WORLD_LO>     worldTranslationLo;
                Field<PARENT>       parent;
                Field<FIRST_CHILD>  firstChild;
                Field<NEXT>         next;
                Field<PREV>         prev;
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
    bool mLocalTransformTransactionOpen = false;
    bool mAccurateTranslations = false;
};

FILAMENT_DOWNCAST(TransformManager)

} // namespace filament

#endif // TNT_FILAMENT_COMPONENTS_TRANSFORMMANAGER_H
