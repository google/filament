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
#include <utils/EntityInstance.h>
#include <utils/EntityManager.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>

#include <tsl/robin_map.h>

#include <atomic>
#include <utility>
#include <vector>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {

class EntityManager;

/**
 * Base class for SingleInstanceComponentManager to handle callbacks without template bloat.
 */
class UTILS_PUBLIC SingleInstanceComponentManagerBase {
public:
    using ChangeCallback = Invocable<void(Slice<const Entity>)>;
    using Instance = EntityInstanceBase::Type;

    static constexpr bool USE_SORTED_DIRTY_ARRAY = false;

    /**
     * Registers a callback to be triggered when components are added, removed, or modified.
     * @param token A unique identifier for the listener (e.g., 'this' pointer).
     * @param callback The callback to invoke.
     * @note Registering the same token multiple times will result in multiple
     *       registrations and the callback being invoked multiple times.
     */
    void registerChangeCallback(void const* token, ChangeCallback callback) noexcept;

    /**
     * Unregisters a callback.
     * @param token The token used during registration.
     * @note If the token is not registered, this operation is a safe no-op.
     *       If the token was registered multiple times, all instances will be removed.
     */
    void unregisterChangeCallback(void const* token) noexcept;

    /**
     * Flushes any pending notifications to listeners.
     * This is called automatically when the internal buffer is full,
     * but can also be called manually (e.g., at the end of a frame).
     */
    void flushNotifications() noexcept;

    /**
     * Registers a bitset to be updated when entities change.
     * @param bitset A pointer to the PagedArenaBitset to register.
     */
    void registerBitset(PagedArenaBitset* bitset);

    /**
     * Unregisters a bitset.
     * @param bitset A pointer to the PagedArenaBitset to unregister.
     */
    void unregisterBitset(PagedArenaBitset const* bitset);

    /**
     * @brief Checks if a newly allocated/recycled Entity collides with an active but logically-dead component payload (a pending zombie).
     *
     * @details Under Filament's asynchronous Epoch-Based Reclamation (EBR) system, when an entity is globally destroyed,
     * its corresponding component payload is immediately cilled logically (e.g., removed from the active bitset) inside
     * @p catchupGarbage(), but its physical slot inside the dense payload array is not freed until the next @p gc() call.
     *
     * If the central EntityManager recycles the dead entity's index and reallocates it to a brand new entity
     * (@p newEntity, which inherits the same index but with a higher generation number) *before* this component
     * manager has physically run its garbage collector, the old "zombie" component payload will still be residing at
     * that exact index slot inside the dense payload array, creating a catastrophic collision.
     *
     * To resolve this, concrete component managers MUST call @p popPendingZombie() inside their component allocation paths.
     * If a collision is detected:
     * 1. The old zombie's identity is extracted in @p outZombie.
     * 2. The zombie's bit is immediately popped and removed from @p mPendingDestruction.
     * 3. The caller MUST immediately and synchronously invoke the C++ payload destructor and array swap-and-pop
     *    compaction logic on @p outZombie (e.g., by calling @p removeComponent(outZombie)) before allocating
     *    the component for @p newEntity.
     *
     * @param[in] newEntity The newly created Entity to check for recycled index collisions.
     * @param[out] outZombie Reference to store the pre-existing logically dead Entity that was collided with.
     * @return True if a pending zombie component was found and popped (requiring immediate destruction), false otherwise.
     */
    bool popPendingZombie(Entity newEntity, Entity& outZombie) noexcept;

    /**
     * Records a change for the given entity.
     * Flushes notifications if the internal buffer becomes full.
     */
    void notifyChange(Entity e) noexcept;

    // Non-templated Presence & Instance queries
    bool hasComponent(Entity const e) const noexcept {
        return getInstance(e) != 0;
    }

    Instance getInstance(Entity const e) const noexcept {
        auto const pos = mInstanceMap.find(e);
        return pos != mInstanceMap.end() ? pos->second : 0;
    }

    PagedArenaBitset const& getEntityBitset() const noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
        return mEntities;
    }

    /**
     * @brief Returns true if this component manager supports deferred/amortized physical array compactions.
     */
    bool isAmortizationSupported() const noexcept { return mAmortizationSupported; }

    /**
     * @brief Temporarily suspends (sleeps) this component manager, unregistering its watermark.
     *
     * @details When a manager is suspended, it no longer participates in the EBR system, allowing
     * the central EntityManager to safely reclaim/recycle epochs without being blocked by this manager.
     *
     * @warning **Strict Rules in Suspended State:**
     * 1. **NO Component Operations**: While suspended, you MUST NOT call any component operations
     *    (e.g. @p addComponent, @p removeComponent, @p getInstance, @p gc, etc.) or access elements in the component arrays.
     * 2. **Entity Destruction Risk**: Because the manager's watermark is unregistered, any entities managed
     *    by this component manager that are destroyed while the manager is suspended can have their
     *    underlying indices recycled and completely reused for newly created entities *before* the manager
     *    has a chance to garbage collect them. This will lead to severe data corruption if accessed!
     * 3. **Precondition for Suspend**: A manager should ideally be completely empty (having 0 components)
     *    or guarantee that no entities it currently holds components for can be destroyed during its suspension.
     *    Typically, @p suspend() is called when the associated scene, world, or render context is inactive or shutdown.
     */
    void suspend() noexcept {
        if (!mSuspended) {
            mEntityManager.unregisterWatermark(&mWatermark);
            mSuspended = true;
        }
    }

    /**
     * @brief Resumes this component manager from a suspended state, re-registering its watermark.
     *
     * @details Re-registers the manager's watermark with the EBR system.
     * Upon resuming, the manager will automatically participate in the next GC cycle and catch up
     * on all missed garbage epochs that are still present on the timeline.
     */
    void resume() noexcept {
        if (mSuspended) {
            mEntityManager.registerWatermark(&mWatermark, mName, &mEntities, &mEbrEntitiesLock);
            mSuspended = false;
        }
    }

    ImmutableCString const& getName() const noexcept { return mName; }

    uint64_t getWatermark() const noexcept { return mWatermark.load(std::memory_order_relaxed); }

    void checkZombieCollisionPanic(Entity newEntity) noexcept;

protected:
    struct CompactionResult {
        size_t lastIndex;
        Entity swappedEntity;
    };

    using SoaAllocator = Instance (*)(void* context, Entity e);
    using SoaDeallocator = CompactionResult (*)(void* context, size_t index);
    using EvictionDispatcher = void (*)(void* ctx, void* extra, Entity const* entities, size_t count);

    /**
     * @brief Constructs a new SingleInstanceComponentManagerBase.
     *
     * @param[in] em Reference to the global EntityManager timeline coordinator.
     * @param[in] name Descriptive diagnostic name for this component manager.
     * @param[in] amortizationSupported Set to true to opt-in to multi-frame physical destruction amortization.
     *
     * @note **CRITICAL CONCURRENCY CONTRACT**:
     * If @p amortizationSupported is set to **true**, the physical component payload array is no longer guaranteed
     * to be purged synchronously inside the current frame. This exposes the manager to Just-In-Time (JIT)
     * recycled index collisions.
     *
     * **IT BECOMES THE STRICT RESPONSIBILITY** of the concrete Component Manager implementation to invoke
     * @p popPendingZombie() and synchronously purge any colliding zombie component payloads inside their
     * @p create() allocation paths BEFORE assigning the new component.
     *
     * If @p amortizationSupported is **false** (the default), physical payloads are purged synchronously per frame,
     * relaxing this implementation requirement (suitable for test harnesses and micro-benchmarks).
     */
    explicit SingleInstanceComponentManagerBase(EntityManager& em, ImmutableCString name,
            bool amortizationSupported = false) noexcept;
    ~SingleInstanceComponentManagerBase() noexcept;

    SingleInstanceComponentManagerBase(const SingleInstanceComponentManagerBase&) = delete;
    SingleInstanceComponentManagerBase& operator=(const SingleInstanceComponentManagerBase&) = delete;

    SingleInstanceComponentManagerBase(SingleInstanceComponentManagerBase&& rhs) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS;
    SingleInstanceComponentManagerBase& operator=(SingleInstanceComponentManagerBase&& rhs) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS;

    Instance addComponentImpl(Entity e, void* context, SoaAllocator allocator) noexcept;

    void removeComponentsImpl(Entity const* entities, size_t count, void* context, SoaDeallocator deallocator) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS;

    void gcImpl(uint32_t maxDestructions,
            void* context, void* extraArg, EvictionDispatcher dispatcher) noexcept;

    /*
     * EBR GC: Periodically queries missed garbage from the global EntityManager
     * and marks dead entities logically dead in mPendingDestruction without moving memory.
     */
    void catchupGarbage() noexcept;

    tsl::robin_map<Entity, Instance, Entity::Hasher> mInstanceMap;
    PagedArenaBitset mPendingDestruction;

    /*
     * mEbrEntitiesLock protects mEntities during global EBR timeline GCs.
     *
     * CONCURRENCY RATIONALE:
     * Component Managers are strictly single-threaded and are only mutated/accessed on the main
     * engine thread. However, the global EntityManager timeline (advanceEpoch() -> getExpiredEpochsLocked())
     * may be executed concurrently from background helper threads (e.g. background asset loaders).
     *
     * To prevent static/inactive assets from stalling global EBR timeline GCs, the EntityManager
     * automatically leap-forwards watermarks by scanning the intersection of each manager's mEntities
     * and the timeline's garbage. To perform this intersection test safely without data races or
     * requiring expensive locks on the hot rendering paths, the EntityManager acquires this lock
     * dynamically while scanning, while mutations (addComponent, removeComponent) lock it briefly.
     *
     * Read-only accesses from the main engine thread (e.g. mEntities.empty() or getEntityBitset())
     * do not require acquiring this lock, as no concurrent writes can occur (mutations are strictly
     * confined to the main thread).
     */
    mutable utils::Mutex mEbrEntitiesLock;
    PagedArenaBitset mEntities UTILS_GUARDED_BY(mEbrEntitiesLock);

private:
    EntityManager& mEntityManager;
    ImmutableCString mName;
    bool mAmortizationSupported = false;
    bool mSuspended = true;
    PagedArenaBitset mCollapsedGarbage;
    std::atomic<uint64_t> mWatermark{0};
    std::vector<const PagedArenaBitset*> mMissedGarbage;

    struct CallbackInfo {
        void const* token;
        ChangeCallback callback;
    };

    static constexpr size_t MAX_DIRTY_COUNT = 16;
    Entity mDirtyEntities[MAX_DIRTY_COUNT];
    size_t mDirtyCount = 0;
    std::vector<CallbackInfo> mChangeCallbacks;
    std::vector<PagedArenaBitset*> mBitsets;
};

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
class UTILS_PUBLIC SingleInstanceComponentManager : public SingleInstanceComponentManagerBase {
    // this is just to avoid using std::default_random_engine, since we're in a public header.
    class default_random_engine {
        uint32_t mState = 1u; // must be 0 < seed < 0x7fffffff
    public:
        uint32_t operator()() noexcept {
            return mState = uint32_t((uint64_t(mState) * 48271u) % 0x7fffffffu);
        }
    };

protected:
    static constexpr size_t ENTITY_INDEX = sizeof ... (Elements);

public:
    using SoA = StructureOfArrays<Elements ..., Entity>;
    using Structure = typename SoA::Structure;
    using Instance = EntityInstanceBase::Type;

    explicit SingleInstanceComponentManager(EntityManager& em, ImmutableCString name,
            bool const amortizationSupported = false) noexcept
            : SingleInstanceComponentManagerBase(em, std::move(name), amortizationSupported) {
        mData.push_back(Structure{});
    }

    ~SingleInstanceComponentManager() noexcept = default;

    SingleInstanceComponentManager(SingleInstanceComponentManager const& rhs) = delete;
    SingleInstanceComponentManager& operator=(SingleInstanceComponentManager const& rhs) = delete;

    SingleInstanceComponentManager(SingleInstanceComponentManager&& rhs) noexcept
            : SingleInstanceComponentManagerBase(std::move(rhs)),
              mData(std::move(rhs.mData)) {
    }

    SingleInstanceComponentManager& operator=(SingleInstanceComponentManager&& rhs) noexcept {
        if (UTILS_LIKELY(this != &rhs)) {
            SingleInstanceComponentManagerBase::operator=(std::move(rhs));
            mData = std::move(rhs.mData);
        }
        return *this;
    }

    // Returns the number of components (i.e. size of each array)
    size_t getComponentCount() const noexcept {
        // The array as an extra dummy component at index 0, so the visible count is 1 less.
        return mData.size() - 1;
    }

    bool empty() const noexcept {
        return getComponentCount() == 0;
    }

    Entity const* getEntities() const noexcept {
        return data<ENTITY_INDEX>() + 1;
    }

    Entity getEntity(Instance const i) const noexcept {
        return elementAt<ENTITY_INDEX>(i);
    }

    // Add a component to the given Entity. If the entity already has a component from this
    // manager, this function is a no-op.
    // This invalidates all pointers components.
    Instance addComponent(Entity e);

    // Removes a component from the given entity.
    // This invalidates all pointers components.
    Instance removeComponent(Entity e);
    void removeComponents(Entity const* entities, size_t count) noexcept;

    // return the first instance
    Instance begin() noexcept {
        catchupGarbage();
        return 1u;
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

    const PagedArenaBitset& getEntityBitset() const noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
        return mEntities;
    }

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
    void swap(Instance const i, Instance const j) noexcept {
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

    template<typename MFClass, typename ReturnType>
    using BatchDestroyFunc = ReturnType (MFClass::*)(Entity const*, size_t);

    template<typename MFClass, typename ReturnType, typename Arg>
    using BatchDestroyArgFunc = ReturnType (MFClass::*)(Entity const*, size_t, Arg);

    template<typename MFClass, typename ReturnType>
    using SingleDestroyFunc = ReturnType (MFClass::*)(Entity);

    template<typename MFClass, typename ReturnType, typename Arg>
    using SingleDestroyArgFunc = ReturnType (MFClass::*)(Entity, Arg);

    template<typename Class, typename MFClass, typename ReturnType>
    void gc(Class* outerInstance, uint32_t const maxDestructions,
            BatchDestroyFunc<MFClass, ReturnType> const destroyFunc) noexcept {
        auto pmf = destroyFunc;

        auto dispatcher = [](void* ctx, void* ext, Entity const* entities, size_t const count) noexcept {
            auto outer = static_cast<Class*>(ctx);
            auto funcPtr = static_cast<BatchDestroyFunc<MFClass, ReturnType>*>(ext);
            (outer->**funcPtr)(entities, count);
        };

        gcImpl(maxDestructions, outerInstance, &pmf, dispatcher);
    }

    template<typename Class, typename MFClass, typename ReturnType, typename Arg>
    void gc(Class* outerInstance, uint32_t const maxDestructions,
            BatchDestroyArgFunc<MFClass, ReturnType, Arg> const destroyFunc,
            std::remove_reference_t<Arg>& extra) noexcept {
        struct Context {
            BatchDestroyArgFunc<MFClass, ReturnType, Arg> pmf;
            std::remove_reference_t<Arg>* arg;
        } ctxStruct { destroyFunc, &extra };

        auto dispatcher = [](void* ctx, void* ext, Entity const* entities, size_t const count) noexcept {
            auto outer = static_cast<Class*>(ctx);
            auto context = static_cast<Context*>(ext);
            (outer->*(context->pmf))(entities, count, *(context->arg));
        };

        gcImpl(maxDestructions, outerInstance, &ctxStruct, dispatcher);
    }

    static constexpr uint32_t DEFAULT_AMORTIZED_DESTRUCTIONS = 50;

    template<typename Class, typename MFClass, typename ReturnType>
    void gc(Class* outerInstance, BatchDestroyFunc<MFClass, ReturnType> destroyFunc) noexcept {
        gc(outerInstance, DEFAULT_AMORTIZED_DESTRUCTIONS, destroyFunc);
    }

    template<typename Class, typename MFClass, typename ReturnType, typename Arg>
    void gc(Class* outerInstance, BatchDestroyArgFunc<MFClass, ReturnType, Arg> destroyFunc,
            std::remove_reference_t<Arg>& extra) noexcept {
        gc(outerInstance, DEFAULT_AMORTIZED_DESTRUCTIONS, destroyFunc, extra);
    }

    template<typename Class, typename MFClass, typename ReturnType>
    void gc(Class* outerInstance, uint32_t const maxDestructions,
            SingleDestroyFunc<MFClass, ReturnType> const destroyFunc) noexcept {
        auto pmf = destroyFunc;

        auto dispatcher = [](void* ctx, void* ext, Entity const* entities, size_t const count) noexcept {
            auto outer = static_cast<Class*>(ctx);
            auto funcPtr = static_cast<SingleDestroyFunc<MFClass, ReturnType>*>(ext);
            for (size_t i = 0; i < count; ++i) {
                (outer->**funcPtr)(entities[i]);
            }
        };

        gcImpl(maxDestructions, outerInstance, &pmf, dispatcher);
    }

    template<typename Class, typename MFClass, typename ReturnType, typename Arg>
    void gc(Class* outerInstance, uint32_t const maxDestructions,
            SingleDestroyArgFunc<MFClass, ReturnType, Arg> const destroyFunc,
            std::remove_reference_t<Arg>& extra) noexcept {
        struct Context {
            SingleDestroyArgFunc<MFClass, ReturnType, Arg> pmf;
            std::remove_reference_t<Arg>* arg;
        } ctxStruct { destroyFunc, &extra };

        auto dispatcher = [](void* ctx, void* ext, Entity const* entities, size_t const count) noexcept {
            auto outer = static_cast<Class*>(ctx);
            auto context = static_cast<Context*>(ext);
            for (size_t i = 0; i < count; ++i) {
                (outer->*(context->pmf))(entities[i], *(context->arg));
            }
        };

        gcImpl(maxDestructions, outerInstance, &ctxStruct, dispatcher);
    }

    template<typename Class, typename MFClass, typename ReturnType>
    void gc(Class* outerInstance, SingleDestroyFunc<MFClass, ReturnType> destroyFunc) noexcept {
        gc(outerInstance, DEFAULT_AMORTIZED_DESTRUCTIONS, destroyFunc);
    }

    template<typename Class, typename MFClass, typename ReturnType, typename Arg>
    void gc(Class* outerInstance, SingleDestroyArgFunc<MFClass, ReturnType, Arg> destroyFunc,
            std::remove_reference_t<Arg>& extra) noexcept {
        gc(outerInstance, DEFAULT_AMORTIZED_DESTRUCTIONS, destroyFunc, extra);
    }

    SoA mData;

private:
    Instance removeComponentsHelper(Entity const* entities, size_t count) noexcept;
};

// Keep these outside of the class because CLion has trouble parsing them
template<typename ... Elements>
typename SingleInstanceComponentManager<Elements ...>::Instance
SingleInstanceComponentManager<Elements ...>::addComponent(Entity e) {
    auto allocator = [](void* ctx, Entity ent) noexcept -> Instance {
        auto self = static_cast<SingleInstanceComponentManager*>(ctx);
        self->mData.push_back(Structure{}).template back<ENTITY_INDEX>() = ent;
        return Instance(self->mData.size() - 1);
    };
    return addComponentImpl(e, this, allocator);
}

template <typename ... Elements>
typename SingleInstanceComponentManager<Elements ...>::Instance
SingleInstanceComponentManager<Elements ... >::removeComponentsHelper(Entity const* entities, size_t const count) noexcept {
    Instance lastIndex = 0;
    struct Context {
        SingleInstanceComponentManager* self;
        Instance* pLastIndex;
    } ctx { this, &lastIndex };

    auto deallocator = [](void* context, size_t index) noexcept -> CompactionResult {
        auto ctxPtr = static_cast<Context*>(context);
        auto self = ctxPtr->self;
        size_t last = self->mData.size() - 1;
        *(ctxPtr->pLastIndex) = last;

        Entity swappedEntity;
        if (last != index) {
            self->mData.forEach([index, last](auto* p) {
                p[index] = std::move(p[last]);
            });
            swappedEntity = self->mData.template elementAt<ENTITY_INDEX>(index);
        }
        self->mData.pop_back();
        return { last, swappedEntity };
    };

    removeComponentsImpl(entities, count, &ctx, deallocator);
    return lastIndex;
}

template <typename ... Elements>
typename SingleInstanceComponentManager<Elements ...>::Instance
SingleInstanceComponentManager<Elements ... >::removeComponent(Entity const e) {
    return removeComponentsHelper(&e, 1);
}

template <typename ... Elements>
void SingleInstanceComponentManager<Elements ... >::removeComponents(Entity const* entities, size_t const count) noexcept {
    removeComponentsHelper(entities, count);
}

} // namespace utils

#endif // TNT_UTILS_SINGLEINSTANCECOMPONENTMANAGER_H
