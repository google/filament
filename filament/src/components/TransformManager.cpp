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

#include <math/mat4.h>

#include <utils/debug.h>
#include <filament/TransformManager.h>


using namespace utils;
using namespace filament::math;

namespace filament {

FTransformManager::FTransformManager() noexcept = default;

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

void FTransformManager::create(Entity const entity) {
    create(entity, 0, mat4f{});
}

void FTransformManager::create(Entity const entity, Instance const parent, const mat4f& localTransform) {
    // this always adds at the end, so all existing instances stay valid
    auto& manager = mManager;

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
        insertNode(i, parent);
        setTransform(i, localTransform);
    }
}

void FTransformManager::create(Entity const entity, Instance const parent, const mat4& localTransform) {
    // this always adds at the end, so all existing instances stay valid
    auto& manager = mManager;

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
        insertNode(i, parent);
        setTransform(i, localTransform);
    }
}

void FTransformManager::setParent(Instance const i, Instance const parent) noexcept {
    validateNode(i);
    if (i) {
        auto& manager = mManager;
        Instance const oldParent = manager[i].parent;
        if (oldParent != parent) {
            // TODO: on debug builds, ensure that the new parent isn't one of our descendant
            removeNode(i);
            insertNode(i, parent);
            updateNodeTransform(i);
            // Note: setParent() doesn't reorder the child after the parent in the array,
            // but that's not a problem because TransformManager doesn't rely on that.
            // Also note that commitLocalTransformTransaction() does reorder all children after
            // their parent, as an optimization to calculate the world transform.
        }
    }
}

Entity FTransformManager::getParent(Instance i) const noexcept {
    i = mManager[i].parent;
    return i ? mManager.getEntity(i) : Entity();
}

size_t FTransformManager::getChildCount(Instance const i) const noexcept {
    size_t count = 0;
    for (Instance ci = mManager[i].firstChild; ci; ci = mManager[ci].next, ++count);
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

void FTransformManager::destroy(Entity const e) noexcept {
    // update the reference of the element we're removing
    auto& manager = mManager;
    Instance const i = manager.getInstance(e);
    validateNode(i);
    if (i) {
        // 1) remove the entry from the linked lists
        removeNode(i);

        // our children don't have parents anymore
        Instance child = manager[i].firstChild;
        while (child) {
            manager[child].parent = 0;
            child = manager[child].next;
        }

        // 2) remove the component
        Instance const moved = manager.removeComponent(e);

        // 3) update the references to the entry now with Instance i
        if (moved != i) {
            updateNode(i);
        }
    }
}

void FTransformManager::setTransform(Instance const ci, const mat4f& model) noexcept {
    validateNode(ci);
    if (ci) {
        auto& manager = mManager;
        // store our local transform
        manager[ci].local = model;
        manager[ci].localTranslationLo = {};
        updateNodeTransform(ci);
    }
}

void FTransformManager::setTransform(Instance const ci, const mat4& model) noexcept {
    validateNode(ci);
    if (ci) {
        auto& manager = mManager;
        // store our local transform + accurate translation information
        manager[ci].local = mat4f(model);
        manager[ci].localTranslationLo = float3{ model[3].xyz - float3{ model[3].xyz }};
        updateNodeTransform(ci);
    }
}

void FTransformManager::updateNodeTransform(Instance const i) noexcept {
    if (UTILS_UNLIKELY(mLocalTransformTransactionOpen)) {
        return;
    }

    validateNode(i);
    auto& manager = mManager;
    assert_invariant(i);

    // find our parent's world transform, if any
    // note: by using the raw_array() we don't need to check that parent is valid.
    Instance const parent = manager[i].parent;
    computeWorldTransform(
            manager[i].world, manager[i].worldTranslationLo,
            manager[parent].world, manager[i].local,
            manager[parent].worldTranslationLo, manager[i].localTranslationLo,
            mAccurateTranslations);

    // update our children's world transforms
    Instance const child = manager[i].firstChild;
    if (UTILS_UNLIKELY(child)) { // assume we don't have a hierarchy in the common case
        transformChildren(manager, child);
    }
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

    // swapNode() below needs some temporary storage which we provide here
    const bool accurate = mAccurateTranslations;
    auto& soa = manager.getSoA();
    soa.ensureCapacity(soa.size() + 1);

    for (Instance i = manager.begin(), e = manager.end(); i != e; ++i) {
        // Ensure that children are always sorted after their parent.
        while (UTILS_UNLIKELY(Instance(manager[i].parent) > i)) {
            swapNode(i, manager[i].parent);
        }
        Instance const parent = manager[i].parent;
        assert_invariant(parent < i);

        computeWorldTransform(
                manager[i].world, manager[i].worldTranslationLo,
                manager[parent].world, manager[i].local,
                manager[parent].worldTranslationLo, manager[i].localTranslationLo,
                accurate);
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

void FTransformManager::transformChildren(Sim& manager, Instance i) noexcept {
    const bool accurate = mAccurateTranslations;
    while (i) {
        // update child's world transform
        Instance const parent = manager[i].parent;
        computeWorldTransform(
                manager[i].world, manager[i].worldTranslationLo,
                manager[parent].world, manager[i].local,
                manager[parent].worldTranslationLo, manager[i].localTranslationLo,
                accurate);

        // assume we don't have a deep hierarchy
        Instance const child = manager[i].firstChild;
        if (UTILS_UNLIKELY(child)) {
            transformChildren(manager, child);
        }

        // process our next child
        i = manager[i].next;
    }
}

void FTransformManager::computeWorldTransform(
        mat4f& UTILS_RESTRICT outWorld,
        float3& UTILS_RESTRICT inoutWorldTranslationLo,
        mat4f const& UTILS_RESTRICT pt,
        mat4f const& UTILS_RESTRICT local,
        float3 const& UTILS_RESTRICT ptTranslationLo,       // reference to avoid unneeded access
        float3 const& UTILS_RESTRICT localTranslationLo,    // reference to avoid unneeded access
        bool const accurate) {

    outWorld[0] = pt * local[0];
    outWorld[1] = pt * local[1];
    outWorld[2] = pt * local[2];

    // "a branch not taken is free", i.e.: we burn a BT cache entry only in the accurate case
    if (UTILS_LIKELY(!accurate)) {
        outWorld[3] = pt * local[3];
    } else {
        // this version takes the extra precision of the translation into account,
        // we assume that the last row of local is [0 0 0 x].
        // Only the last column of the result needs special treatment -- unfortunately this requires
        // converting 'pt' to a mat4 (double)

        const mat4 ptd{
                pt[0], pt[1], pt[2],
                double4{ double3(pt[3].xyz) + double3(ptTranslationLo), pt[3].w }};

        const double4 worldTranslation =
                ptd * double4{ double3(local[3].xyz) + double3(localTranslationLo), local[3].w };

        inoutWorldTranslationLo = worldTranslation.xyz - float3{ worldTranslation.xyz };
        outWorld[3] = worldTranslation;
    }
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

void FTransformManager::gc(EntityManager& em) noexcept {
    mManager.gc(em, [this](Entity const e) {
                destroy(e);
            });
}

TransformManager::children_iterator& TransformManager::children_iterator::operator++() {
    FTransformManager const& that = downcast(mManager);
    mInstance = that.mManager[mInstance].next;
    return *this;
}

} // namespace filament
