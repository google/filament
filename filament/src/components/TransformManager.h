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
#include <utils/Entity.h>
#include <utils/Mutex.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>

#include <math/mat4.h>

#include <atomic>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

namespace filament {

class UTILS_PRIVATE FTransformManager : public TransformManager {
public:
    using Instance = Instance;

    explicit FTransformManager(utils::EntityManager& em) noexcept;
    ~FTransformManager() noexcept;

    // free-up all resources
    void terminate() noexcept;


    /*
    * Component Manager APIs
    */

    bool hasComponent(utils::Entity const e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity const e) const noexcept {
        return { mManager.getInstance(e) };
    }

    size_t getComponentCount() const noexcept {
        return mManager.getComponentCount();
    }

    bool empty() const noexcept {
        return mManager.empty();
    }

    utils::Entity getEntity(Instance const i) const noexcept {
        return mManager.getEntity(i);
    }

    utils::Entity const* getEntities() const noexcept {
        return mManager.getEntities();
    }

    utils::Slice<const utils::Entity> getAllEntities() const noexcept {
        return { mManager.getEntities(), mManager.getComponentCount() };
    }

    const utils::PagedArenaBitset& getEntityBitset() const noexcept {
        return mManager.getEntityBitset();
    }

    void setAccurateTranslationsEnabled(bool enable) noexcept;

    bool isAccurateTranslationsEnabled() const noexcept {
        return mAccurateTranslations;
    }

    void create(utils::Entity entity);

    void create(utils::Entity entity, Instance parent, const math::mat4f& localTransform);

    void create(utils::Entity entity, Instance parent, const math::mat4& localTransform);

    void destroyComponents(utils::Entity const* entities, size_t count) noexcept;

    void destroy(utils::Entity e) noexcept {
        destroyComponents(&e, 1);
    }

    void setParent(Instance i, Instance parent) noexcept;

    utils::Entity getParent(Instance i) const noexcept;

    size_t getChildCount(Instance i) const noexcept;

    size_t getChildren(Instance i, utils::Entity* children, size_t count) const noexcept;

    children_iterator getChildrenBegin(Instance parent) const noexcept;

    children_iterator getChildrenEnd(Instance parent) const noexcept;

    children_range getChildrenRange(Instance parent) const noexcept;

    void openLocalTransformTransaction() noexcept;

    void commitLocalTransformTransaction() noexcept;

    void gc() noexcept;

    // Evaluates any pending dirty world transforms and notifies registered change callbacks
    // and bitsets.
    // Note: Because world transforms are evaluated lazily on demand, if
    // ensureWorldTransformsUpToDate() is not called on the main thread after mutations (as
    // FScene::prepare() does before dispatching worker jobs), the commit—and therefore any
    // registered ChangeCallback or PagedArenaBitset updates—will execute on whichever thread
    // first queries a world transform.
    UTILS_ALWAYS_INLINE
    void ensureWorldTransformsUpToDate() const noexcept {
        // `memory_order_acquire` pairs with `mHasDirtyTransforms.store(false, memory_order_release)`
        // in `commitDirtyTransformsLocked()` / `computeAllWorldTransforms()` (double-checked locking):
        // if a concurrent reader thread observes `false` here and skips `commitDirtyTransforms()`
        // (never acquiring `mCommitLock`), the acquire load guarantees that all `world` / `worldLo`
        // writes performed by the committing thread are visible before the caller reads `mManager[ci].world`.
        if (UTILS_UNLIKELY(mHasDirtyTransforms.load(std::memory_order_acquire))) {
            commitDirtyTransforms();
        }
    }

    // Change notifications (registered ChangeCallbacks and PagedArenaBitsets)
    //
    // Only changes occurring after registration are reported.
    //
    // Threading invariant (internal; the public API is covered by Engine's "not thread-safe"
    // rule): change notifications are dispatched by the lazy commit *after* world transforms are
    // published, so a reader on another thread can observe up-to-date transforms while the
    // committing thread is still updating bitsets / invoking callbacks. Therefore:
    //
    // - Code that consumes notifications (reads a registered bitset, or relies on callbacks
    //   having run) must not run concurrently with readers that may trigger a lazy commit.
    //   The consuming thread must call ensureWorldTransformsUpToDate() (or flushNotifications())
    //   *before* any other reader starts, e.g. before dispatching jobs, as FScene::prepare()
    //   does. Otherwise, callbacks and bitset updates may run on a worker thread.
    //
    // - Callbacks may query world transforms (they observe a fully committed hierarchy).
    //   Mutating the TransformManager from a callback is subject to the usual rule that
    //   mutations must not run concurrently with readers, which matters because callbacks may
    //   run on a worker thread.
    //
    // See FTransformManager::commitDirtyTransforms() for details.
    void registerChangeCallback(void const* token,
            utils::SingleInstanceComponentManagerBase::ChangeCallback callback) noexcept {
        ensureWorldTransformsUpToDate();
        mManager.registerChangeCallback(token, std::move(callback));
    }
    void unregisterChangeCallback(void const* token) noexcept {
        mManager.unregisterChangeCallback(token);
    }
    void flushNotifications() noexcept {
        ensureWorldTransformsUpToDate();
        mManager.flushNotifications();
    }
    void registerBitset(utils::PagedArenaBitset* bitset) {
        ensureWorldTransformsUpToDate();
        mManager.registerBitset(bitset);
    }
    void unregisterBitset(utils::PagedArenaBitset const* bitset) {
        mManager.unregisterBitset(bitset);
    }

    utils::Slice<const math::mat4f> getWorldTransforms() const noexcept {
        ensureWorldTransformsUpToDate();
        return mManager.slice<WORLD>();
    }

    void setTransform(Instance ci, const math::mat4f& model) noexcept;

    void setTransform(Instance ci, const math::mat4& model) noexcept;

    const math::mat4f& getTransform(Instance const ci) const noexcept {
        return mManager[ci].local;
    }

    const math::mat4f& getWorldTransform(Instance const ci) const noexcept {
        ensureWorldTransformsUpToDate();
        return mManager[ci].world;
    }

    math::mat4 getTransformAccurate(Instance const ci) const noexcept {
        math::mat4f const& local = mManager[ci].local;
        math::float3 const localTranslationLo = mManager[ci].localTranslationLo;
        math::mat4 r(local);
        r[3].xyz += localTranslationLo;
        return r;
    }

    math::mat4 getWorldTransformAccurate(Instance const ci) const noexcept {
        ensureWorldTransformsUpToDate();
        math::mat4f const& world = mManager[ci].world;
        math::float3 const worldTranslationLo = mManager[ci].worldTranslationLo;
        math::mat4 r(world);
        r[3].xyz += worldTranslationLo;
        return r;
    }

private:
    struct Sim;

    void createImpl(utils::Entity entity, Instance parent, std::variant<math::mat4, math::mat4f> localTransform);

    void validateNode(Instance i) noexcept;
    void removeNode(Instance i) noexcept;
    void updateNode(Instance i) noexcept;
    UTILS_ALWAYS_INLINE
    void updateNodeTransform(Instance i) noexcept;
    UTILS_ALWAYS_INLINE
    void updateNodeTransform(Instance i, math::mat4f const& local,
            math::float3 const& localLo, bool zeroLo) noexcept;
    // Safe without mCommitLock because TransformManager's API contract forbids mutations
    // (create, destroy, setParent, setTransform) concurrently with other mutations or with
    // const reader queries; mCommitLock only serializes lazy commits across concurrent readers.
    UTILS_ALWAYS_INLINE
    void markNodeDirty(Instance i) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS;
    UTILS_NOINLINE
    void commitDirtyTransforms() const noexcept;
    void commitDirtyTransformsLocked() noexcept UTILS_REQUIRES(mCommitLock);
    // Lock-free by design; see the rationale in TransformManager.cpp.
    void recyclePendingNotifications(std::vector<utils::Entity>&& buffer) noexcept
            UTILS_NO_THREAD_SAFETY_ANALYSIS;
    void insertNode(Instance i, Instance parent) noexcept;
    void swapNode(Instance i, Instance j) noexcept;
    void transformChildren(Sim& manager, Instance parent, bool notifyParent = true) noexcept UTILS_REQUIRES(mCommitLock);

    void computeAllWorldTransforms() noexcept;

    UTILS_ALWAYS_INLINE
    static void computeWorldTransform(math::mat4f& outWorld,
            math::mat4f const& pt, math::mat4f const& local) noexcept;

    UTILS_ALWAYS_INLINE
    static void computeWorldTransformAccurate(math::mat4f& outWorld,
            math::float3& inoutWorldTranslationLo,
            math::mat4f const& pt, math::mat4f const& local,
            math::float3 const& ptTranslationLo, math::float3 const& localTranslationLo) noexcept;

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
        DIRTY,          // non-zero if local transform changed and world transform is pending;
                        // guarded by mCommitLock during concurrent lazy commits, or accessed
                        // lock-free during single-writer mutations (see markNodeDirty).
    };

    using Base = utils::SingleInstanceComponentManager<
            math::mat4f,    // local
            math::mat4f,    // world
            math::float3,   // accurate local translation
            math::float3,   // accurate world translation
            Instance,       // parent
            Instance,       // firstChild
            Instance,       // next
            Instance,       // prev
            uint8_t         // dirty (guarded by mCommitLock during concurrent reads)
    >;

    struct Sim : public Base {
        explicit Sim(utils::EntityManager& em) noexcept : Base(em, "TransformManager") {}
        using Base::gc;
        using Base::swap;
        using Base::data;

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
                Field<DIRTY>        dirty; // guarded by mCommitLock during concurrent reads
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
    mutable utils::Mutex mCommitLock;
    mutable std::vector<Instance> mDirtyInstances UTILS_GUARDED_BY(mCommitLock);
    mutable std::vector<utils::Entity> mPendingNotifications UTILS_GUARDED_BY(mCommitLock);
    mutable std::atomic<bool> mHasDirtyTransforms{ false };
    bool mLocalTransformTransactionOpen = false;
    bool mAccurateTranslations = false;
};

FILAMENT_DOWNCAST(TransformManager)

} // namespace filament

#endif // TNT_FILAMENT_COMPONENTS_TRANSFORMMANAGER_H
