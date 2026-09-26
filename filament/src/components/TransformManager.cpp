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

#include "components/TransformManager.h"

#include <filament/TransformManager.h>

#include <utils/debug.h>
#include <utils/Mutex.h>

#include <math/mat4.h>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>


using namespace utils;
using namespace filament::math;

namespace filament {

namespace {

UTILS_ALWAYS_INLINE
bool isTransformIdentical(mat4f const& a, mat4f const& b) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    uint32x4_t const d0 = veorq_u32(vreinterpretq_u32_f32(vld1q_f32(&a[0].x)),
            vreinterpretq_u32_f32(vld1q_f32(&b[0].x)));
    uint32x4_t const d1 = veorq_u32(vreinterpretq_u32_f32(vld1q_f32(&a[1].x)),
            vreinterpretq_u32_f32(vld1q_f32(&b[1].x)));
    uint32x4_t const d2 = veorq_u32(vreinterpretq_u32_f32(vld1q_f32(&a[2].x)),
            vreinterpretq_u32_f32(vld1q_f32(&b[2].x)));
    uint32x4_t const d3 = veorq_u32(vreinterpretq_u32_f32(vld1q_f32(&a[3].x)),
            vreinterpretq_u32_f32(vld1q_f32(&b[3].x)));
    uint32x4_t const diff = vorrq_u32(vorrq_u32(d0, d1), vorrq_u32(d2, d3));
    return vmaxvq_u32(diff) == 0;
#else
    return std::memcmp(&a, &b, sizeof(mat4f)) == 0;
#endif
}

} // namespace

FTransformManager::FTransformManager(EntityManager& em) noexcept
        : mManager(em) {
}

FTransformManager::~FTransformManager() noexcept = default;

void FTransformManager::terminate() noexcept {
}

void FTransformManager::setAccurateTranslationsEnabled(bool const enable) noexcept {
    if (enable != mAccurateTranslations) {
        mAccurateTranslations = enable;
        // when enabling accurate translations, we have to recompute all world transforms
        if (enable && !mLocalTransformTransactionOpen) {
            computeAllWorldTransforms();
        }
    }
}

// Safe without mCommitLock because TransformManager's API contract forbids mutations
// (create, destroy, setParent, setTransform) concurrently with other mutations or with
// const reader queries; mCommitLock only serializes lazy commits across concurrent readers.
UTILS_ALWAYS_INLINE
void FTransformManager::markNodeDirty(Instance const i) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
    uint8_t& dirty = mManager[i].dirty;
    if (!dirty) {
        dirty = 1;
        mDirtyInstances.push_back(i);
        mHasDirtyTransforms.store(true, std::memory_order_relaxed);
    }
}

inline void FTransformManager::createImpl(Entity const entity, Instance const parent,
        std::variant<mat4, mat4f> localTransform) {
    // this always adds at the end, so all existing instances stay valid
    auto& manager = mManager;

    Entity zombie;
    if (UTILS_UNLIKELY(manager.popPendingZombie(entity, zombie))) {
        destroy(zombie);
    }

    // TODO: try to keep entries sorted with their siblings/parents to improve cache access
    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance const i = manager.addComponent(entity);
    assert_invariant(i);
    assert_invariant(i != parent);

    if (i && i != parent) {
        manager[i].parent = 0;
        manager[i].next = 0;
        manager[i].prev = 0;
        manager[i].firstChild = 0;
        manager[i].dirty = 0;
        insertNode(i, parent);
        if (UTILS_LIKELY(parent)) {
            invalidateTopologyFromParent(parent);
        } else {
            markTopologyDirty();
        }
        std::visit([this, &manager, i](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, mat4f>) {
                manager[i].local = arg;
                manager[i].localTranslationLo = float3{};
            } else {
                mat4f const local = mat4f(arg);
                float3 const localLo = float3{ arg[3].xyz - float3{ arg[3].xyz } };
                manager[i].local = local;
                manager[i].localTranslationLo = localLo;
            }
            if (UTILS_LIKELY(!mLocalTransformTransactionOpen)) {
                markNodeDirty(i);
            }
        }, localTransform);
    }
}

void FTransformManager::create(Entity const entity) {
    createImpl(entity, 0, mat4f{});
}

void FTransformManager::create(Entity const entity, Instance const parent, const mat4f& localTransform) {
    createImpl(entity, parent, localTransform);
}

void FTransformManager::create(Entity const entity, Instance const parent, const mat4& localTransform) {
    createImpl(entity, parent, localTransform);
}

void FTransformManager::setParent(Instance const i, Instance const parent) noexcept {
    validateNode(i);
    if (i) {
        auto& manager = mManager;
        Instance const oldParent = manager[i].parent;
        if (oldParent != parent) {
#ifndef NDEBUG
            // ensure that the new parent isn't one of our descendants
            for (Instance p = parent; p; p = manager[p].parent) {
                assert_invariant(p != i);
            }
#endif
            removeNode(i);
            insertNode(i, parent);
            if (UTILS_LIKELY(oldParent && parent)) {
                invalidateTopologyFromParent(std::min(oldParent, parent));
            } else {
                markTopologyDirty();
            }
            if (UTILS_LIKELY(!mLocalTransformTransactionOpen)) {
                markNodeDirty(i);
            }
            // Note: setParent() doesn't reorder the child after the parent in the array,
            // but that's not a problem because TransformManager doesn't rely on that.
            // Also note that commitLocalTransformTransaction() and defragment() reorder all
            // children after their parent, as an optimization to calculate the world transform.
        }
    }
}

Entity FTransformManager::getParent(Instance i) const noexcept {
    i = mManager[i].parent;
    return i ? mManager.getEntity(i) : Entity();
}

size_t FTransformManager::getChildCount(Instance const i) const noexcept {
    size_t count = 0;
    for (Instance ci = mManager[i].firstChild; ci; ci = mManager[ci].next, ++count) {
    }
    return count;
}

size_t FTransformManager::getChildren(Instance const i, Entity* children,
        size_t const count) const noexcept {
    Instance ci = mManager[i].firstChild;
    size_t numWritten = 0;
    while (ci && numWritten < count) {
        children[numWritten++] = mManager.getEntity(ci);
        ci = mManager[ci].next;
    }
    return numWritten;
}

TransformManager::children_iterator FTransformManager::getChildrenBegin(
        Instance const parent) const noexcept {
    return { *this, mManager[parent].firstChild };
}

TransformManager::children_iterator FTransformManager::getChildrenEnd(Instance) const noexcept {
    return { *this, 0 };
}

TransformManager::children_range FTransformManager::getChildrenRange(
        Instance const parent) const noexcept {
    return { { *this, mManager[parent].firstChild } };
}

void FTransformManager::destroyComponents(Entity const* entities, size_t const count) noexcept {
    ensureWorldTransformsUpToDate();
    auto& manager = mManager;
    // Children orphaned below become roots and need their world transform recomputed. They're
    // recorded by Entity because removeComponent() moves the last component into the destroyed
    // slot, which would invalidate a saved Instance (including those in mDirtyInstances).
    std::vector<Entity> orphans;
    for (size_t k = 0; k < count; ++k) {
        Entity const e = entities[k];
        Instance const i = manager.getInstance(e);
        validateNode(i);
        if (i) {
            Instance const oldParent = manager[i].parent;
            Instance const firstChild = manager[i].firstChild;
            removeNode(i);
            Instance child = firstChild;
            while (child) {
                manager[child].parent = 0;
                orphans.push_back(manager.getEntity(child));
                child = manager[child].next;
            }
            Instance const moved = manager.removeComponent(e);
            if (moved != i) {
                updateNode(i);
            }
            Instance const movedParent = (moved != i) ? Instance(manager[i].parent) : oldParent;
            if (UTILS_LIKELY(oldParent && movedParent && !firstChild)) {
                Instance minParent = std::min(oldParent, movedParent);
                if (moved != i && Instance(manager[i].firstChild)) {
                    minParent = std::min(minParent, i);
                }
                invalidateTopologyFromParent(minParent);
            } else {
                markTopologyDirty();
            }
        }
    }

    // Now that no more components move, mark the orphans dirty so their world transform becomes
    // their local transform (and their descendants follow). Orphans destroyed later in the loop
    // no longer have a component and are skipped. During a transaction, the commit recomputes
    // everything anyway.
    if (UTILS_UNLIKELY(!orphans.empty()) && !mLocalTransformTransactionOpen) {
        for (Entity const e : orphans) {
            Instance const i = manager.getInstance(e);
            if (i) {
                markNodeDirty(i);
            }
        }
    }
}

void FTransformManager::setTransform(Instance const ci, const mat4f& model) noexcept {
    validateNode(ci);
    if (UTILS_LIKELY(ci)) {
        auto& manager = mManager;
        if (UTILS_UNLIKELY(isTransformIdentical(manager[ci].local, model)) &&
                manager[ci].localTranslationLo == float3{}) {
            return;
        }
        // store our local transform
        manager[ci].local = model;
        manager[ci].localTranslationLo = float3{};
        updateNodeTransform(ci, model, float3{}, true);
    }
}

void FTransformManager::setTransform(Instance const ci, const mat4& model) noexcept {
    validateNode(ci);
    if (UTILS_LIKELY(ci)) {
        auto& manager = mManager;
        // store our local transform + accurate translation information
#if defined(__ARM_NEON) && defined(__aarch64__)
        float32x4_t const c0 = vcvt_high_f32_f64(vcvt_f32_f64(vld1q_f64(&model[0].x)), vld1q_f64(&model[0].z));
        float32x4_t const c1 = vcvt_high_f32_f64(vcvt_f32_f64(vld1q_f64(&model[1].x)), vld1q_f64(&model[1].z));
        float32x4_t const c2 = vcvt_high_f32_f64(vcvt_f32_f64(vld1q_f64(&model[2].x)), vld1q_f64(&model[2].z));
        float64x2_t const m3_xy = vld1q_f64(&model[3].x);
        float64x2_t const m3_zw = vld1q_f64(&model[3].z);
        float32x2_t const c3_xy = vcvt_f32_f64(m3_xy);
        float32x4_t const c3 = vcvt_high_f32_f64(c3_xy, m3_zw);
        mat4f local;
        vst1q_f32(&local[0].x, c0);
        vst1q_f32(&local[1].x, c1);
        vst1q_f32(&local[2].x, c2);
        vst1q_f32(&local[3].x, c3);
        float32x2_t const lo_xy = vcvt_f32_f64(vsubq_f64(m3_xy, vcvt_f64_f32(c3_xy)));
        float const lo_z = float(vgetq_lane_f64(m3_zw, 0) - double(vgetq_lane_f32(c3, 2)));
        float3 localLo;
        vst1_f32(&localLo.x, lo_xy);
        localLo.z = lo_z;
#else
        mat4f const local = mat4f(model);
        float3 const localLo = float3{ model[3].xyz - float3{ model[3].xyz } };
#endif
        if (UTILS_UNLIKELY(isTransformIdentical(manager[ci].local, local)) &&
                manager[ci].localTranslationLo == localLo) {
            return;
        }
        manager[ci].local = local;
        manager[ci].localTranslationLo = localLo;
        updateNodeTransform(ci, local, localLo, false);
    }
}

// Called from ensureWorldTransformsUpToDate() only once mHasDirtyTransforms was observed true.
// The commit runs under mCommitLock (concurrent readers may race to commit), but change
// notifications are dispatched *outside* the lock:
//
// - notifyChange() may flush and invoke ChangeCallbacks, which are expected to query world
//   transforms (getWorldTransform(), etc.). Because commitDirtyTransformsLocked() has already
//   cleared mHasDirtyTransforms, those re-entrant queries take the lock-free fast path instead
//   of self-deadlocking on the non-recursive mCommitLock, and they observe a fully committed
//   hierarchy.
//
// - The pending entities are moved into a local vector before dispatch, so the slice passed to
//   notifyChange() stays valid even if a callback causes a nested commit (e.g. a mutation
//   followed by a query), which would clear and refill mPendingNotifications.
//
// - After dispatch, the (now empty) buffer is recycled into mPendingNotifications so its
//   capacity is reused and steady-state frames don't reallocate. This is done without
//   re-acquiring mCommitLock (see recyclePendingNotifications()).
//
// - Because mHasDirtyTransforms is cleared before dispatch, a reader on another thread may see
//   up-to-date world transforms while this thread is still inside notifyChange() (updating
//   registered bitsets and invoking callbacks). Consumers of change notifications (callbacks,
//   registered bitsets) must therefore not run concurrently with readers that may trigger a
//   lazy commit: the consuming thread should call ensureWorldTransformsUpToDate() before any
//   other reader starts, as FScene::prepare() does before dispatching its jobs.
void FTransformManager::commitDirtyTransforms() const noexcept {
    auto* const self = const_cast<FTransformManager*>(this);
    std::vector<Entity> pending;
    {
        LockGuard const lock(self->mCommitLock);
        // Re-check under the lock: another reader may have committed while we were waiting.
        if (!mHasDirtyTransforms.load(std::memory_order_relaxed)) {
            return;
        }
        self->commitDirtyTransformsLocked();
        pending = std::move(self->mPendingNotifications);
    }

    if (!pending.empty()) {
        // Dispatch without holding mCommitLock (see above).
        self->mManager.notifyChange({ pending.data(), pending.size() });
    }
    // Recycle unconditionally so the buffer's capacity isn't lost when nothing was notified.
    self->recyclePendingNotifications(std::move(pending));
}

// Lock-free on purpose: mHasDirtyTransforms is now false, so only a new commit can touch
// mPendingNotifications. A new commit needs a mutation, which the API contract forbids during
// reads, and that external synchronization also publishes the buffer to the next committer.
// A nested commit from a callback has already finished on this thread by now; the capacity
// check keeps whichever buffer is larger in that case.
void FTransformManager::recyclePendingNotifications(
        std::vector<Entity>&& buffer) noexcept UTILS_NO_THREAD_SAFETY_ANALYSIS {
    buffer.clear();
    if (mPendingNotifications.capacity() < buffer.capacity()) {
        mPendingNotifications = std::move(buffer);
    }
}

void FTransformManager::commitDirtyTransformsLocked() noexcept {
    auto& manager = mManager;
    bool const accurate = mAccurateTranslations;
    uint8_t* const UTILS_RESTRICT dirtyFlags = manager.data<DIRTY>();
    Instance const* const UTILS_RESTRICT parents = manager.data<PARENT>();
    Instance const* const UTILS_RESTRICT firstChildren = manager.data<FIRST_CHILD>();
    mat4f* const UTILS_RESTRICT worlds = manager.data<WORLD>();
    mat4f const* const UTILS_RESTRICT locals = manager.data<LOCAL>();
    float3* const UTILS_RESTRICT worldLos = manager.data<WORLD_LO>();
    float3 const* const UTILS_RESTRICT localLos = manager.data<LOCAL_LO>();
    Entity const* const UTILS_RESTRICT entities = manager.getEntities() - 1;

    mPendingNotifications.clear();

    for (Instance const i : mDirtyInstances) {
        if (!dirtyFlags[i]) {
            continue;
        }
        // Walk up to the highest dirty ancestor so the entire dirty subtree is resolved top-down once.
        Instance top = i;
        Instance p = parents[top];
        while (p) {
            if (dirtyFlags[p]) {
                top = p;
            }
            p = parents[p];
        }

        dirtyFlags[top] = 0;
        Instance const topParent = parents[top];
        if (UTILS_LIKELY(!topParent)) {
            worlds[top] = locals[top];
            if (UTILS_UNLIKELY(accurate)) {
                worldLos[top] = localLos[top];
            }
        } else if (UTILS_LIKELY(!accurate)) {
            computeWorldTransform(worlds[top], worlds[topParent], locals[top]);
        } else {
            computeWorldTransformAccurate(
                    worlds[top], worldLos[top],
                    worlds[topParent], locals[top],
                    worldLos[topParent], localLos[top]);
        }

        if (UTILS_UNLIKELY(firstChildren[top])) {
            transformChildren(manager, top, true);
        } else {
            mPendingNotifications.push_back(entities[top]);
        }
    }

    mDirtyInstances.clear();
    // `memory_order_release` publishes all updated `worlds[]` and `worldLos[]` entries before
    // clearing `mHasDirtyTransforms`. Concurrent readers in `ensureWorldTransformsUpToDate()`
    // that observe `false` via `load(memory_order_acquire)` skip `mCommitLock` entirely, so this
    // release/acquire pair is required to prevent them from reading stale `world` matrices on
    // weakly-ordered architectures (e.g. ARM64).
    mHasDirtyTransforms.store(false, std::memory_order_release);
}

void FTransformManager::updateNodeTransform(Instance const i) noexcept {
    auto const& manager = mManager;
    updateNodeTransform(i, manager[i].local, manager[i].localTranslationLo, false);
}

void FTransformManager::updateNodeTransform(Instance const i,
        mat4f const& local, float3 const& localLo, bool const zeroLo) noexcept {
    if (UTILS_UNLIKELY(mLocalTransformTransactionOpen)) {
        return;
    }
    auto& manager = mManager;
    validateNode(i);
    assert_invariant(i);

    // Fast path for childless nodes (flat root nodes or parented leaf nodes) when no ancestor can
    // be dirty: compute the world transform in-place while `local` is still in registers and keep
    // `mHasDirtyTransforms == false`, avoiding `mDirtyInstances` and `mCommitLock` overhead
    // (especially during interleaved `setTransform` / `getWorldTransform` calls).
    // If the node has children (`firstChild != 0`) or `mHasDirtyTransforms` is already true (which
    // implies an ancestor's world transform may be stale), defer evaluation to `commitDirtyTransforms()`.
    if (UTILS_UNLIKELY(Instance(manager[i].firstChild) ||
            mHasDirtyTransforms.load(std::memory_order_relaxed))) {
        markNodeDirty(i);
        return;
    }

    Instance const parent = manager[i].parent;
    bool const accurate = mAccurateTranslations;
    if (UTILS_LIKELY(!parent)) {
        manager[i].world = local;
        if (UTILS_UNLIKELY(accurate)) {
            manager[i].worldTranslationLo = zeroLo ? float3{} : localLo;
        }
    } else if (UTILS_LIKELY(!accurate)) {
        computeWorldTransform(manager[i].world, manager[parent].world, local);
    } else {
        computeWorldTransformAccurate(
                manager[i].world, manager[i].worldTranslationLo,
                manager[parent].world, local,
                manager[parent].worldTranslationLo, zeroLo ? float3{} : localLo);
    }
    manager.notifyChange(manager.getEntity(i));
}

void FTransformManager::openLocalTransformTransaction() noexcept {
    mLocalTransformTransactionOpen = true;
}

void FTransformManager::commitLocalTransformTransaction() noexcept {
    if (mLocalTransformTransactionOpen) {
        mLocalTransformTransactionOpen = false;
        computeAllWorldTransforms();
    }
}

void FTransformManager::computeAllWorldTransforms() noexcept {
    auto& manager = mManager;
    {
        LockGuard const lock(mCommitLock);

        // swapNode() below needs some temporary storage which we provide here
        const bool accurate = mAccurateTranslations;
        auto& soa = manager.getSoA();
        soa.ensureCapacity(soa.size() + 1);

        for (Instance i = manager.begin(), e = manager.end(); i != e; ++i) {
            // Ensure that children are always sorted after their parent.
            while (UTILS_UNLIKELY(Instance(manager[i].parent) > i)) {
                swapNode(i, manager[i].parent);
                markTopologyDirty();
            }
            Instance const parent = manager[i].parent;
            assert_invariant(parent < i);

            manager[i].dirty = 0;
            if (UTILS_LIKELY(!parent)) {
                manager[i].world = manager[i].local;
                if (UTILS_UNLIKELY(accurate)) {
                    manager[i].worldTranslationLo = manager[i].localTranslationLo;
                }
            } else if (UTILS_LIKELY(!accurate)) {
                computeWorldTransform(manager[i].world, manager[parent].world, manager[i].local);
            } else {
                computeWorldTransformAccurate(
                        manager[i].world, manager[i].worldTranslationLo,
                        manager[parent].world, manager[i].local,
                        manager[parent].worldTranslationLo, manager[i].localTranslationLo);
            }
        }

        mDirtyInstances.clear();
        mPendingNotifications.clear();
        // `memory_order_release` publishes all `world` / `worldTranslationLo` writes to concurrent
        // readers that observe `false` via `load(memory_order_acquire)` without taking `mCommitLock`.
        mHasDirtyTransforms.store(false, std::memory_order_release);
    }

    if (!empty()) {
        manager.notifyChange(getAllEntities());
    }
}

// Inserts a parentless node in the hierarchy
void FTransformManager::insertNode(Instance const i, Instance const parent) noexcept {
    auto& manager = mManager;

    assert_invariant(manager[i].parent == Instance{});

    manager[i].parent = parent;
    manager[i].prev = 0;
    manager[i].next = 0;
    if (parent) {
        // we insert ourselves first in the parent's list
        Instance const next = manager[parent].firstChild;
        manager[i].next = next;
        // we're our parent's first child now
        manager[parent].firstChild = i;
        if (next) {
            // and we are the previous sibling of our next sibling
            manager[next].prev = i;
        }
    }

    validateNode(i);
    validateNode(parent);
}

void FTransformManager::swapNode(Instance const i, Instance const j) noexcept {
    validateNode(i);
    validateNode(j);

    auto& manager = mManager;

    // swap the content of the nodes directly
    std::swap(manager.elementAt<LOCAL>(i),    manager.elementAt<LOCAL>(j));
    std::swap(manager.elementAt<LOCAL_LO>(i), manager.elementAt<LOCAL_LO>(j));
    std::swap(manager.elementAt<WORLD>(i),    manager.elementAt<WORLD>(j));
    std::swap(manager.elementAt<WORLD_LO>(i), manager.elementAt<WORLD_LO>(j));
    std::swap(manager.elementAt<DIRTY>(i),    manager.elementAt<DIRTY>(j));
    manager.swap(i, j); // this swaps the data relative to SingleInstanceComponentManager

    // now swap the linked-list references, to do that correctly we must use a temporary
    // node to fix-up the linked-list pointers
    // Here we are guaranteed to have enough capacity for our temporary storage, so we
    // can safely use the item just past the end of the array.
    assert_invariant(manager.getSoA().capacity() >= manager.getSoA().size() + 1);

    const Instance t = manager.end();

    manager[t].parent       = manager[i].parent;
    manager[t].firstChild   = manager[i].firstChild;
    manager[t].next         = manager[i].next;
    manager[t].prev         = manager[i].prev;
    updateNode(t);

    manager[i].parent       = manager[j].parent;
    manager[i].firstChild   = manager[j].firstChild;
    manager[i].next         = manager[j].next;
    manager[i].prev         = manager[j].prev;
    updateNode(i);

    manager[j].parent       = manager[t].parent;
    manager[j].firstChild   = manager[t].firstChild;
    manager[j].next         = manager[t].next;
    manager[j].prev         = manager[t].prev;
    updateNode(j);
}

// removes a node from the graph, but doesn't remove it or its children from the array
// (making everybody orphaned).
void FTransformManager::removeNode(Instance const i) noexcept {
    auto& manager = mManager;
    Instance const parent = manager[i].parent;
    Instance const prev = manager[i].prev;
    Instance const next = manager[i].next;
    if (prev) {
        manager[prev].next = next;
    } else if (parent) {
        // we don't have a previous sibling, which means we're the parent's first child
        // update the parent's first child to our next sibling
        manager[parent].firstChild = next;
    }
    if (next) {
        manager[next].prev = prev;
    }

#ifndef NDEBUG
    // we no longer have a parent or siblings. we don't really have to clear those fields
    // so we only do it in DEBUG mode
    manager[i].parent = 0;
    manager[i].prev = 0;
    manager[i].next = 0;
#endif
}

// update references to this node after it has been moved in the array
void FTransformManager::updateNode(Instance const i) noexcept {
    auto& manager = mManager;
    // update our preview sibling's next reference (to ourselves)
    Instance const parent = manager[i].parent;
    Instance const prev = manager[i].prev;
    Instance const next = manager[i].next;
    if (prev) {
        manager[prev].next = i;
    } else if (parent) {
        // we don't have a previous sibling, which means we're the parent's first child
        // update the parent's first child to us
        manager[parent].firstChild = i;
    }
    if (next) {
        manager[next].prev = i;
    }
    // re-parent our children to us
    Instance child = manager[i].firstChild;
    while (child) {
        assert_invariant(child != i);
        manager[child].parent = i;
        child = manager[child].next;
    }
    validateNode(i);
    validateNode(parent);
    validateNode(prev);
    validateNode(next);
}

void FTransformManager::transformChildren(Sim& manager, Instance parent, bool const notifyParent) noexcept {
    mat4f* const UTILS_RESTRICT worlds = manager.data<WORLD>();
    mat4f const* const UTILS_RESTRICT locals = manager.data<LOCAL>();
    Instance const* const UTILS_RESTRICT firstChildren = manager.data<FIRST_CHILD>();
    uint8_t* const UTILS_RESTRICT dirtyFlags = manager.data<DIRTY>();
    Entity const* const UTILS_RESTRICT entities = manager.getEntities() - 1;
    bool const accurate = mAccurateTranslations;
    float3* const UTILS_RESTRICT worldTranslationLos = manager.data<WORLD_LO>();
    float3 const* const UTILS_RESTRICT localTranslationLos = manager.data<LOCAL_LO>();

    // Fast path when the SoA is BFS-defragmented (!mTopologyDirty):
    // Every depth level of any subtree occupies a single contiguous slice [levelStart, levelEnd)
    // in the SoA, and each parent's children form a contiguous sub-run with parents[c] == p.
    // This eliminates the DFS stack, all nexts[c] pointer-chasing loads, per-node entity batching,
    // and walks memory strictly forward in unit-stride BFS order.
    if (UTILS_LIKELY(!mTopologyDirty)) {
        Instance* const UTILS_RESTRICT parents = manager.data<PARENT>();
        // Write sentinel parent = 0 at manager.end() so parents[c] == p and
        // (parents[c] - parentStart) < parentCount always terminate at the end of the array.
        // The slot at end() exists because defragment() reserves size() + 1 elements before
        // clearing mTopologyDirty, and any operation that grows the array (create()) sets
        // mTopologyDirty again, so the fast path never runs without that spare slot.
        assert_invariant(manager.getSoA().capacity() > manager.getSoA().size());
        parents[manager.end()] = 0;

        if (UTILS_LIKELY(notifyParent)) {
            mPendingNotifications.push_back(entities[parent]);
        }

        uint32_t parentStart = parent;
        uint32_t parentCount = 1;
        uint32_t levelStart = firstChildren[parent];

        while (levelStart != 0) {
            uint32_t nextLevelStart = 0;
            uint32_t c = levelStart;
            if (UTILS_LIKELY(!accurate)) {
                while (uint32_t(parents[c]) - parentStart < parentCount) {
                    uint32_t const p = parents[c];
                    mat4f const pt = worlds[p];
                    do {
                        dirtyFlags[c] = 0;
                        computeWorldTransform(worlds[c], pt, locals[c]);
                        if (UTILS_UNLIKELY(!nextLevelStart)) {
                            nextLevelStart = firstChildren[c];
                        }
                        ++c;
                    } while (uint32_t(parents[c]) == p);
                }
            } else {
                while (uint32_t(parents[c]) - parentStart < parentCount) {
                    uint32_t const p = parents[c];
                    mat4f const pt = worlds[p];
                    float3 const ptLo = worldTranslationLos[p];
                    do {
                        dirtyFlags[c] = 0;
                        computeWorldTransformAccurate(
                                worlds[c], worldTranslationLos[c],
                                pt, locals[c],
                                ptLo, localTranslationLos[c]);
                        if (UTILS_UNLIKELY(!nextLevelStart)) {
                            nextLevelStart = firstChildren[c];
                        }
                        ++c;
                    } while (uint32_t(parents[c]) == p);
                }
            }

            mPendingNotifications.insert(mPendingNotifications.end(),
                    entities + levelStart, entities + c);

            parentStart = levelStart;
            parentCount = c - levelStart;
            levelStart = nextLevelStart;
        }
        return;
    }

    constexpr size_t STACK_CAPACITY = 128;
    constexpr size_t BATCH_CAPACITY = 128;
    Instance::Type stack[STACK_CAPACITY];
    Entity dirtyBatch[BATCH_CAPACITY];
    size_t top = 0;

    Instance const* const UTILS_RESTRICT nexts = manager.data<NEXT>();

    size_t batchCount = 0;
    if (UTILS_LIKELY(notifyParent)) {
        dirtyBatch[batchCount++] = entities[parent];
    }

    do {
        Instance const firstChild = firstChildren[parent];
        mat4f const pt = worlds[parent];
        float3 const ptLo = UTILS_UNLIKELY(accurate) ? worldTranslationLos[parent] : float3{};
        Instance overflowStart = 0;
        for (Instance c = firstChild; c; c = nexts[c]) {
            dirtyFlags[c] = 0;
            if (UTILS_LIKELY(!accurate)) {
                computeWorldTransform(worlds[c], pt, locals[c]);
            } else {
                computeWorldTransformAccurate(
                        worlds[c], worldTranslationLos[c],
                        pt, locals[c],
                        ptLo, localTranslationLos[c]);
            }
            dirtyBatch[batchCount++] = entities[c];
            if (UTILS_UNLIKELY(batchCount == BATCH_CAPACITY)) {
                mPendingNotifications.insert(mPendingNotifications.end(),
                        dirtyBatch, dirtyBatch + BATCH_CAPACITY);
                batchCount = 0;
            }
            if (UTILS_UNLIKELY(firstChildren[c])) {
                if (UTILS_LIKELY(top < STACK_CAPACITY)) {
                    stack[top++] = c;
                } else if (!overflowStart) {
                    overflowStart = c;
                }
            }
        }
        if (UTILS_UNLIKELY(overflowStart)) {
            if (batchCount > 0) {
                mPendingNotifications.insert(mPendingNotifications.end(),
                        dirtyBatch, dirtyBatch + batchCount);
                batchCount = 0;
            }
            for (Instance c = overflowStart; c; c = nexts[c]) {
                if (firstChildren[c]) {
                    transformChildren(manager, c, false);
                }
            }
        }
        if (top == 0) {
            break;
        }
        parent = stack[--top];
    } while (true);

    if (batchCount > 0) {
        mPendingNotifications.insert(mPendingNotifications.end(),
                dirtyBatch, dirtyBatch + batchCount);
    }
}

UTILS_ALWAYS_INLINE
void FTransformManager::computeWorldTransform(
        mat4f& UTILS_RESTRICT outWorld,
        mat4f const& UTILS_RESTRICT pt,
        mat4f const& UTILS_RESTRICT local) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    float32x4_t const p0 = vld1q_f32(&pt[0][0]);
    float32x4_t const p1 = vld1q_f32(&pt[1][0]);
    float32x4_t const p2 = vld1q_f32(&pt[2][0]);
    float32x4_t const p3 = vld1q_f32(&pt[3][0]);

    float32x4_t const l0 = vld1q_f32(&local[0][0]);
    float32x4_t const l1 = vld1q_f32(&local[1][0]);
    float32x4_t const l2 = vld1q_f32(&local[2][0]);
    float32x4_t const l3 = vld1q_f32(&local[3][0]);

    float32x4_t r0 = vmulq_laneq_f32(p0, l0, 0);
    float32x4_t r1 = vmulq_laneq_f32(p0, l1, 0);
    float32x4_t r2 = vmulq_laneq_f32(p0, l2, 0);
    float32x4_t r3 = vmulq_laneq_f32(p0, l3, 0);

    r0 = vfmaq_laneq_f32(r0, p1, l0, 1);
    r1 = vfmaq_laneq_f32(r1, p1, l1, 1);
    r2 = vfmaq_laneq_f32(r2, p1, l2, 1);
    r3 = vfmaq_laneq_f32(r3, p1, l3, 1);

    r0 = vfmaq_laneq_f32(r0, p2, l0, 2);
    r1 = vfmaq_laneq_f32(r1, p2, l1, 2);
    r2 = vfmaq_laneq_f32(r2, p2, l2, 2);
    r3 = vfmaq_laneq_f32(r3, p2, l3, 2);

    r0 = vfmaq_laneq_f32(r0, p3, l0, 3);
    r1 = vfmaq_laneq_f32(r1, p3, l1, 3);
    r2 = vfmaq_laneq_f32(r2, p3, l2, 3);
    r3 = vfmaq_laneq_f32(r3, p3, l3, 3);

    vst1q_f32(&outWorld[0][0], r0);
    vst1q_f32(&outWorld[1][0], r1);
    vst1q_f32(&outWorld[2][0], r2);
    vst1q_f32(&outWorld[3][0], r3);
#else
    outWorld[0] = pt * local[0];
    outWorld[1] = pt * local[1];
    outWorld[2] = pt * local[2];
    outWorld[3] = pt * local[3];
#endif
}

UTILS_ALWAYS_INLINE
void FTransformManager::computeWorldTransformAccurate(
        mat4f& UTILS_RESTRICT outWorld,
        float3& UTILS_RESTRICT inoutWorldTranslationLo,
        mat4f const& UTILS_RESTRICT pt,
        mat4f const& UTILS_RESTRICT local,
        float3 const& UTILS_RESTRICT ptTranslationLo,
        float3 const& UTILS_RESTRICT localTranslationLo) noexcept {
#if defined(__ARM_NEON) && defined(__aarch64__)
    float32x4_t const p0 = vld1q_f32(&pt[0].x);
    float32x4_t const p1 = vld1q_f32(&pt[1].x);
    float32x4_t const p2 = vld1q_f32(&pt[2].x);
    float32x4_t const p3 = vld1q_f32(&pt[3].x);

    float32x4_t const l0 = vld1q_f32(&local[0].x);
    float32x4_t const l1 = vld1q_f32(&local[1].x);
    float32x4_t const l2 = vld1q_f32(&local[2].x);
    float32x4_t const l3 = vld1q_f32(&local[3].x);

    float32x4_t r0 = vmulq_laneq_f32(p0, l0, 0);
    float32x4_t r1 = vmulq_laneq_f32(p0, l1, 0);
    float32x4_t r2 = vmulq_laneq_f32(p0, l2, 0);

    r0 = vfmaq_laneq_f32(r0, p1, l0, 1);
    r1 = vfmaq_laneq_f32(r1, p1, l1, 1);
    r2 = vfmaq_laneq_f32(r2, p1, l2, 1);

    r0 = vfmaq_laneq_f32(r0, p2, l0, 2);
    r1 = vfmaq_laneq_f32(r1, p2, l1, 2);
    r2 = vfmaq_laneq_f32(r2, p2, l2, 2);

    r0 = vfmaq_laneq_f32(r0, p3, l0, 3);
    r1 = vfmaq_laneq_f32(r1, p3, l1, 3);
    r2 = vfmaq_laneq_f32(r2, p3, l2, 3);

    float32x2_t const l3_lo_xy = vld1_f32(&localTranslationLo.x);
    float32x2_t const l3_lo_zw = vset_lane_f32(localTranslationLo.z, vdup_n_f32(0.0f), 0);
    float64x2_t const l3d_xy = vaddq_f64(vcvt_f64_f32(vget_low_f32(l3)), vcvt_f64_f32(l3_lo_xy));
    float64x2_t const l3d_zw = vaddq_f64(vcvt_high_f64_f32(l3), vcvt_f64_f32(l3_lo_zw));

    float32x2_t const p3_lo_xy = vld1_f32(&ptTranslationLo.x);
    float32x2_t const p3_lo_zw = vset_lane_f32(ptTranslationLo.z, vdup_n_f32(0.0f), 0);
    float64x2_t const p3d_xy = vaddq_f64(vcvt_f64_f32(vget_low_f32(p3)), vcvt_f64_f32(p3_lo_xy));
    float64x2_t const p3d_zw = vaddq_f64(vcvt_high_f64_f32(p3), vcvt_f64_f32(p3_lo_zw));

    float64x2_t wt_xy = vmulq_laneq_f64(vcvt_f64_f32(vget_low_f32(p0)), l3d_xy, 0);
    float64x2_t wt_zw = vmulq_laneq_f64(vcvt_high_f64_f32(p0),          l3d_xy, 0);

    wt_xy = vfmaq_laneq_f64(wt_xy, vcvt_f64_f32(vget_low_f32(p1)), l3d_xy, 1);
    wt_zw = vfmaq_laneq_f64(wt_zw, vcvt_high_f64_f32(p1),          l3d_xy, 1);

    wt_xy = vfmaq_laneq_f64(wt_xy, vcvt_f64_f32(vget_low_f32(p2)), l3d_zw, 0);
    wt_zw = vfmaq_laneq_f64(wt_zw, vcvt_high_f64_f32(p2),          l3d_zw, 0);

    wt_xy = vfmaq_laneq_f64(wt_xy, p3d_xy, l3d_zw, 1);
    wt_zw = vfmaq_laneq_f64(wt_zw, p3d_zw, l3d_zw, 1);

    float32x2_t const r3_low = vcvt_f32_f64(wt_xy);
    float32x4_t const r3 = vcvt_high_f32_f64(r3_low, wt_zw);

    vst1q_f32(&outWorld[0].x, r0);
    vst1q_f32(&outWorld[1].x, r1);
    vst1q_f32(&outWorld[2].x, r2);
    vst1q_f32(&outWorld[3].x, r3);

    float32x2_t const lo_xy = vcvt_f32_f64(vsubq_f64(wt_xy, vcvt_f64_f32(r3_low)));
    float const lo_z = float(vgetq_lane_f64(wt_zw, 0) - double(vgetq_lane_f32(r3, 2)));
    vst1_f32(&inoutWorldTranslationLo.x, lo_xy);
    inoutWorldTranslationLo.z = lo_z;
#else
    computeWorldTransform(outWorld, pt, local);

    const mat4 ptd{
            pt[0], pt[1], pt[2],
            double4{ double3(pt[3].xyz) + double3(ptTranslationLo), pt[3].w }};

    const double4 worldTranslation =
            ptd * double4{ double3(local[3].xyz) + double3(localTranslationLo), local[3].w };

    inoutWorldTranslationLo = worldTranslation.xyz - float3{ worldTranslation.xyz };
    outWorld[3] = worldTranslation;
#endif
}

void FTransformManager::validateNode(UTILS_UNUSED_IN_RELEASE Instance const i) noexcept {
#ifndef NDEBUG
    auto& manager = mManager;
    if (i) {
        Instance const parent = manager[i].parent;
        Instance const firstChild = manager[i].firstChild;
        Instance const prev = manager[i].prev;
        Instance const next = manager[i].next;
        assert_invariant(parent != i);
        assert_invariant(prev != i);
        assert_invariant(next != i);
        assert_invariant(firstChild != i);
        if (prev) {
            if (parent) {
                assert_invariant(manager[parent].firstChild != i);
            }
            assert_invariant(manager[prev].next == i);
        } else {
            if (parent) {
                assert_invariant(manager[parent].firstChild == i);
            }
        }
        if (next) {
            assert_invariant(manager[next].prev == i);
        }
        if (parent) {
            // make sure we are in the child list of our parent
            Instance child = manager[parent].firstChild;
            assert_invariant(child);
            while (child && child != i) {
                child = manager[child].next;
            }
            assert_invariant(child);
        }
        if (firstChild) {
            assert_invariant(manager[firstChild].parent == i);
            assert_invariant(manager[firstChild].prev == 0);
        }
    }
#endif
}

void FTransformManager::invalidateTopologyFromParent(Instance const minParent) noexcept {
    assert_invariant(minParent);
    mTopologyDirty = true;
    if (!mDefragParentCursor) {
        // Still in Phase 1 (packing roots); Phase 2 hasn't started yet.
        return;
    }
    if (minParent > mDefragParentCursor) {
        // Phase 2 hasn't reached minParent yet; [1, mDefragWriteCursor) only contains roots and
        // children of parents <= mDefragParentCursor < minParent.
        return;
    }
    // In the placed child region [mDefragRootCount + 1, mDefragWriteCursor), parents[] is sorted
    // in monotonically non-decreasing order. Binary-search for the first slot K with
    // parents[K] >= minParent; all slots [1, K) remain in valid BFS order.
    Instance const* const parents = mManager.data<PARENT>();
    Instance::Type const childBegin = mDefragRootCount + 1;
    Instance::Type const childEnd = std::min<Instance::Type>(mDefragWriteCursor, mManager.end());
    if (childBegin < childEnd) {
        auto const it = std::lower_bound(parents + childBegin, parents + childEnd, minParent);
        mDefragWriteCursor = Instance(uint32_t(it - parents));
    } else {
        mDefragWriteCursor = Instance(childBegin);
    }
    mDefragParentCursor = minParent;
    mDefragChildCursor = mManager[minParent].firstChild;
}

void FTransformManager::gc() noexcept {
    mManager.gc(this, &FTransformManager::destroyComponents);
    defragment();
}

void FTransformManager::defragment(size_t const maxSwaps) noexcept {
    if (UTILS_LIKELY(!mTopologyDirty || maxSwaps == 0)) {
        return;
    }

    auto& manager = mManager;

    // swapNode() uses manager.end() as a temporary node to fix up sibling/parent links, and
    // transformChildren()'s fast path (enabled once mTopologyDirty is cleared) writes a sentinel
    // there. Reserve it before any path below can clear mTopologyDirty.
    auto& soa = manager.getSoA();
    soa.ensureCapacity(soa.size() + 1);

    if (UTILS_UNLIKELY(manager.getComponentCount() <= 1)) {
        // Nothing to reorder; go back to the initial state.
        resetDefragCursors();
        mTopologyDirty = false;
        return;
    }

    // Resolve any pending dirty instances before swapping Instance indices so mDirtyInstances
    // does not hold stale Instance handles.
    ensureWorldTransformsUpToDate();

    Instance const end = manager.end();
    size_t swaps = 0;
    size_t steps = 0;
    size_t const maxSteps = (maxSwaps <= (SIZE_MAX / 8)) ? (maxSwaps * 8) : SIZE_MAX;

    // Phase 1: Pack all root nodes (parent == 0) into [1, R].
    if (!mDefragParentCursor) {
        while (mDefragChildCursor < end && swaps < maxSwaps && steps < maxSteps) {
            ++steps;
            Instance const scan = mDefragChildCursor++;
            if (!Instance(manager[scan].parent)) {
                Instance const target = mDefragWriteCursor++;
                if (scan != target) {
                    swapNode(scan, target);
                    ++swaps;
                }
            }
        }
        if (mDefragChildCursor < end) {
            return;
        }
        mDefragRootCount = uint32_t(mDefragWriteCursor) - 1;
        if (mDefragWriteCursor >= end) {
            // All nodes in the manager are root nodes; defragmentation is complete.
            mDefragParentCursor = end;
            mTopologyDirty = false;
            return;
        }
        mDefragParentCursor = 1;
        mDefragChildCursor = manager[1].firstChild;
    }

    // Phase 2: Breadth-First Search (BFS) placement of children into [R + 1, end - 1].
    // The prefix [1, mDefragWriteCursor) acts as an implicit in-place BFS queue:
    // mDefragParentCursor scans parents in [1, mDefragWriteCursor) while mDefragWriteCursor
    // places each parent's children contiguously at the tail of the placed prefix.
    while (mDefragWriteCursor < end && swaps < maxSwaps && steps < maxSteps) {
        ++steps;
        if (!mDefragChildCursor) {
            ++mDefragParentCursor;
            if (UTILS_UNLIKELY(mDefragParentCursor >= mDefragWriteCursor)) {
                mDefragParentCursor = end;
                mTopologyDirty = false;
                return;
            }
            mDefragChildCursor = manager[mDefragParentCursor].firstChild;
            continue;
        }

        Instance const c = mDefragChildCursor;
        Instance const target = mDefragWriteCursor++;
        assert_invariant(c >= target);
        if (c != target) {
            swapNode(c, target);
            ++swaps;
        }
        // Logical node `c` now resides at `target`; advance to its next sibling in the child list.
        mDefragChildCursor = manager[target].next;
    }

    if (mDefragWriteCursor >= end) {
        mDefragParentCursor = end;
        mTopologyDirty = false;
    }
}
TransformManager::children_iterator& TransformManager::children_iterator::operator++() noexcept {
    FTransformManager const& that = downcast(*mManager);
    mInstance = that.mManager[mInstance].next;
    return *this;
}

} // namespace filament
