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

#ifndef TNT_UTILS_SINGLEINSTANCECOMPONENTMANAGER_H
#define TNT_UTILS_SINGLEINSTANCECOMPONENTMANAGER_H

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/StructureOfArrays.h>

#include <tsl/robin_map.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

class EntityManager;

/*
 * Helper class to create single instance component managers.
 *
 * This handles the component's storage as a structure-of-arrays, as well
 * as the garbage collection.
 *
 * This is intended to be used as base class for a real component manager. When doing so,
 * and the real component manager is a public API, make sure to forward the public methods
 * to the implementation.
 *
 */
template <typename ... Elements>
class SingleInstanceComponentManager {
private:

    // this is just to avoid using std::default_random_engine, since we're in a public header.
    class default_random_engine {
        uint32_t mState = 1u; // must be 0 < seed < 0x7fffffff
    public:
        inline uint32_t operator()() noexcept {
            return mState = uint32_t((uint64_t(mState) * 48271u) % 0x7fffffffu);
        }
    };

protected:
    static constexpr size_t ENTITY_INDEX = sizeof ... (Elements);

public:
    using SoA = StructureOfArrays<Elements ..., Entity>;

    using Instance = EntityInstanceBase::Type;

    SingleInstanceComponentManager() noexcept {
        // We always start with a dummy entry because index=0 is reserved. The component
        // at index = 0, is guaranteed to be default-initialized.
        // Sub-classes can use this to their advantage.
        mData.push_back();
    }

    SingleInstanceComponentManager(SingleInstanceComponentManager&& rhs) noexcept {/* = default */}
    SingleInstanceComponentManager& operator=(SingleInstanceComponentManager&& rhs) noexcept {/* = default */}
    ~SingleInstanceComponentManager() noexcept = default;

    // not copyable
    SingleInstanceComponentManager(SingleInstanceComponentManager const& rhs) = delete;
    SingleInstanceComponentManager& operator=(SingleInstanceComponentManager const& rhs) = delete;


    // returns true if the given Entity has a component of this Manager
    bool hasComponent(Entity e) const noexcept {
        return getInstance(e) != 0;
    }

    // Get instance of this Entity to be used to retrieve components
    UTILS_NOINLINE
    Instance getInstance(Entity e) const noexcept {
        auto const& map = mInstanceMap;
        // find() generates quite a bit of code
        auto pos = map.find(e);
        return pos != map.end() ? pos->second : 0;
    }

    // returns the number of components (i.e. size of each arrays)
    size_t getComponentCount() const noexcept {
        // The array as an extra dummy component at index 0, so the visible count is 1 less.
        return mData.size() - 1;
    }

    bool empty() const noexcept {
        return getComponentCount() == 0;
    }

    // returns a pointer to the Entity array. This is basically the list
    // of entities this component manager handles.
    // The pointer becomes invalid when adding or removing a component.
    Entity const* getEntities() const noexcept {
        return begin<ENTITY_INDEX>();
    }

    Entity getEntity(Instance i) const noexcept {
        return elementAt<ENTITY_INDEX>(i);
    }

    // Add a component to the given Entity. If the entity already has a component from this
    // manager, this function is a no-op.
    // This invalidates all pointers components.
    inline Instance addComponent(Entity e);

    // Removes a component from the given entity.
    // This invalidates all pointers components.
    inline Instance removeComponent(Entity e);

    // trigger one round of garbage collection. this is intended to be called on a regular
    // basis. This gc gives up after it cannot randomly free 'ratio' component in a row.
    void gc(const EntityManager& em, size_t ratio = 4) noexcept {
        gc(em, ratio, [this](Entity e) {
                    removeComponent(e);
                });
    }

    // return the first instance
    Instance begin() const noexcept { return 1u; }

    // return the past-the-last instance
    Instance end() const noexcept { return Instance(begin() + getComponentCount()); }

    // return a pointer to the first element of the ElementIndex'th array
    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex>* begin() noexcept {
        return mData.template data<ElementIndex>() + 1;
    }

    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex> const* begin() const noexcept {
        return mData.template data<ElementIndex>() + 1;
    }

    // return a pointer to the past-the-end element of the ElementIndex'th array
    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex>* end() noexcept {
        return begin<ElementIndex>() + getComponentCount();
    }

    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex> const* end() const noexcept {
        return begin<ElementIndex>() + getComponentCount();
    }

    // return a Slice<>
    template<size_t ElementIndex>
    Slice<typename SoA::template TypeAt<ElementIndex>> slice() noexcept {
        return { begin<ElementIndex>(), end<ElementIndex>() };
    }

    template<size_t ElementIndex>
    Slice<const typename SoA::template TypeAt<ElementIndex>> slice() const noexcept {
        return { begin<ElementIndex>(), end<ElementIndex>() };
    }

    // return a reference to the index'th element of the ElementIndex'th array
    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex>& elementAt(Instance index) noexcept {
        assert(index);
        return data<ElementIndex>()[index];
    }

    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex> const& elementAt(Instance index) const noexcept {
        assert(index);
        return data<ElementIndex>()[index];
    }

    // returns a pointer to the RAW ARRAY of components including the first dummy component
    // Use with caution.
    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex> const* raw_array() const noexcept {
        return data<ElementIndex>();
    }

    // We need our own version of Field because mData is private
    template<size_t E>
    struct Field : public SoA::template Field<E> {
        Field(SingleInstanceComponentManager& soa, EntityInstanceBase::Type i) noexcept
                : SoA::template Field<E>{ soa.mData, i } {
        }
        using SoA::template Field<E>::operator =;
    };

protected:
    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex>* data() noexcept {
        return mData.template data<ElementIndex>();
    }

    template<size_t ElementIndex>
    typename SoA::template TypeAt<ElementIndex> const* data() const noexcept {
        return mData.template data<ElementIndex>();
    }

    // swap only internals
    void swap(Instance i, Instance j) noexcept {
        assert(i);
        assert(j);
        if (i && j) {
            // update the index map
            auto& map = mInstanceMap;
            Entity& ei = elementAt<ENTITY_INDEX>(i);
            Entity& ej = elementAt<ENTITY_INDEX>(j);
            std::swap(ei, ej);
            if (ei) {
                map[ei] = i;
            }
            if (ej) {
                map[ej] = j;
            }
        }
    }

    template<typename REMOVE>
    void gc(const EntityManager& em, size_t ratio,
            REMOVE removeComponent) noexcept {
        Entity const* entities = getEntities();
        size_t count = getComponentCount();
        size_t aliveInARow = 0;
        default_random_engine& rng = mRng;
        #pragma nounroll
        while (count && aliveInARow < ratio) {
            // note: using the modulo favorizes lower number
            size_t i = rng() % count;
            if (UTILS_LIKELY(em.isAlive(entities[i]))) {
                ++aliveInARow;
                continue;
            }
            aliveInARow = 0;
            count--;
            removeComponent(entities[i]);
        }
    }

protected:
    SoA mData;

private:
    // maps an entity to an instance index
    tsl::robin_map<Entity, Instance> mInstanceMap;
    default_random_engine mRng;
};

// Keep these outside of the class because CLion has trouble parsing them
template<typename ... Elements>
typename SingleInstanceComponentManager<Elements ...>::Instance
SingleInstanceComponentManager<Elements ...>::addComponent(Entity e) {
    Instance ci = 0;
    if (!e.isNull()) {
        if (!hasComponent(e)) {
            // this is like a push_back(e);
            mData.push_back().template back<ENTITY_INDEX>() = e;
            // index 0 is used when the component doesn't exist
            ci = Instance(mData.size() - 1);
            mInstanceMap[e] = ci;
        } else {
            // if the entity already has this component, just return its instance
            ci = mInstanceMap[e];
        }
    }
    assert(ci != 0);
    return ci;
}

// Keep these outside of the class because CLion has trouble parsing them
template <typename ... Elements>
typename SingleInstanceComponentManager<Elements ...>::Instance
SingleInstanceComponentManager<Elements ... >::removeComponent(Entity e) {
    auto& map = mInstanceMap;
    auto pos = map.find(e);
    if (UTILS_LIKELY(pos != map.end())) {
        size_t index = pos->second;
        assert(index != 0);
        size_t last = mData.size() - 1;
        if (last != index) {
            // move the last item to where we removed this component, as to keep
            // the array tightly packed.
            mData.forEach([index, last](auto* p) {
                p[index] = std::move(p[last]);
            });

            Entity lastEntity = mData.template elementAt<ENTITY_INDEX>(index);
            map[lastEntity] = index;
        }
        mData.pop_back();
        map.erase(pos);
        return last;
    }
    return 0;
}


} // namespace filament

#endif // TNT_UTILS_SINGLEINSTANCECOMPONENTMANAGER_H
