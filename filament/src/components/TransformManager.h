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

#ifndef TNT_FILAMENT_DETAILS_TRANSFORMMANAGER_H
#define TNT_FILAMENT_DETAILS_TRANSFORMMANAGER_H

#include "upcast.h"

#include <filament/TransformManager.h>

#include <utils/compiler.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Entity.h>
#include <utils/Slice.h>

#include <math/mat4.h>

namespace filament {

class UTILS_PRIVATE FTransformManager : public TransformManager {
public:
    using Instance = TransformManager::Instance;

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
        return Instance(mManager.getInstance(e));
    }

    void create(utils::Entity entity);

    void create(utils::Entity entity, Instance parent, const math::mat4f& localTransform);

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

    const math::mat4f& getTransform(Instance ci) const noexcept {
        return mManager[ci].local;
    }

    const math::mat4f& getWorldTransform(Instance ci) const noexcept {
        return mManager[ci].world;
    }

private:
    struct Sim;

    void validateNode(Instance i) noexcept;
    void removeNode(Instance i) noexcept;
    void updateNode(Instance i) noexcept;
    void updateNodeTransform(Instance i) noexcept;
    void insertNode(Instance i, Instance p) noexcept;
    void swapNode(Instance i, Instance j) noexcept;
    static void transformChildren(Sim& manager, Instance firstChild) noexcept;

    friend class TransformManager::children_iterator;

    enum {
        LOCAL,          // local transform (relative to parent), world if no parent
        WORLD,          // world transform
        PARENT,         // instance to the parent
        FIRST_CHILD,    // instance to our first child
        NEXT,           // instance to our next sibling
        PREV,           // instance to our previous sibling
    };

    using Base = utils::SingleInstanceComponentManager<
            math::mat4f,
            math::mat4f,
            Instance,
            Instance,
            Instance,
            Instance
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        typename Base::SoA& getSoA() { return mData; }

        struct Proxy {
            // all of this gets inlined
            UTILS_ALWAYS_INLINE
            Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
                    : local{ sim, i } { }

            union {
                // this specific usage of union is permitted. All fields are identical
                Field<LOCAL>        local;
                Field<WORLD>        world;
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
};

FILAMENT_UPCAST(TransformManager)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_TRANSFORMMANAGER_H
