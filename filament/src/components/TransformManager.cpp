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

using namespace utils;
using namespace filament::math;

namespace filament {
namespace details {

FTransformManager::FTransformManager() noexcept = default;

FTransformManager::~FTransformManager() noexcept = default;

void FTransformManager::terminate() noexcept {
}

void FTransformManager::create(Entity entity) {
    create(entity, 0, {});
}

void FTransformManager::create(Entity entity, Instance parent, const mat4f& localTransform) {
    // this always adds at the end, so all existing instances stay valid
    auto& manager = mManager;

    // TODO: try to keep entries sorted with their siblings/parents to improve cache access
    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance i = manager.addComponent(entity);
    assert(i);
    assert(i != parent);

    if (i && i != parent) {
        manager[i].parent = 0;
        manager[i].next = 0;
        manager[i].prev = 0;
        manager[i].firstChild = 0;
        insertNode(i, parent);
        setTransform(i, localTransform);
    }
}

void FTransformManager::setParent(Instance i, Instance parent) noexcept {
    validateNode(i);
    if (i) {
        auto& manager = mManager;
        Instance oldParent = manager[i].parent;
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

size_t FTransformManager::getChildCount(Instance i) const noexcept {
    size_t count = 0;
    for (Instance ci = mManager[i].firstChild; ci; ci = mManager[ci].next, ++count);
    return count;
}

size_t FTransformManager::getChildren(Instance i, utils::Entity* children,
        size_t count) const noexcept {
    Instance ci = mManager[i].firstChild;
    size_t numWritten = 0;
    while (ci && numWritten < count) {
        children[numWritten++] = mManager.getEntity(ci);
        ci = mManager[ci].next;
    }
    return numWritten;
}

TransformManager::children_iterator FTransformManager::getChildrenBegin(
        Instance parent) const noexcept {
    return { *this, mManager[parent].firstChild };
}

TransformManager::children_iterator FTransformManager::getChildrenEnd(Instance) const noexcept {
    return { *this, 0 };
}

void FTransformManager::destroy(Entity e) noexcept {
    // update the reference of the element we're removing
    auto& manager = mManager;
    Instance i = manager.getInstance(e);
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
        Instance moved = manager.removeComponent(e);

        // 3) update the references to the entry now with Instance i
        if (moved != i) {
            updateNode(i);
        }
    }
}

void FTransformManager::setTransform(Instance ci, const mat4f& model) noexcept {
    validateNode(ci);
    if (ci) {
        auto& manager = mManager;
        // store our local transform
        manager[ci].local = model;
        updateNodeTransform(ci);
    }
}

void FTransformManager::updateNodeTransform(Instance i) noexcept {
    if (UTILS_UNLIKELY(mLocalTransformTransactionOpen)) {
        return;
    }

    validateNode(i);
    auto& manager = mManager;
    assert(i);

    // find our parent's world transform, if any
    // note: by using the raw_array() we don't need to check that parent is valid.
    Instance parent = manager[i].parent;
    mat4f const& pt = manager.raw_array<WORLD>()[parent];

    // compute our world transform
    manager[i].world = pt * static_cast<mat4f const&>(manager[i].local);

    // update our children's world transforms
    Instance child = manager[i].firstChild;
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
        auto& manager = mManager;

        // swapNode() below needs some temporary storage which we provide here
        auto& soa = manager.getSoA();
        soa.ensureCapacity(soa.size() + 1);

        mat4f const* const UTILS_RESTRICT world = manager.raw_array<WORLD>();
        for (Instance i = manager.begin(), e = manager.end(); i != e; ++i) {
            // Ensure that children are always sorted after their parent.
            while (UTILS_UNLIKELY(Instance(manager[i].parent) > i)) {
                swapNode(i, manager[i].parent);
            }
            Instance parent = manager[i].parent;
            assert(parent < i);
            manager[i].world = world[parent] * static_cast<mat4f const&>(manager[i].local);
        }
    }
}

// Inserts a parentless node in the hierarchy
void FTransformManager::insertNode(Instance i, Instance parent) noexcept {
    auto& manager = mManager;

    assert(manager[i].parent == Instance{});

    manager[i].parent = parent;
    manager[i].prev = 0;
    if (parent) {
        // we insert ourselves first in the parent's list
        Instance next = manager[parent].firstChild;
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

void FTransformManager::swapNode(Instance i, Instance j) noexcept {
    validateNode(i);
    validateNode(j);

    auto& manager = mManager;

    // swap the content of the nodes directly
    std::swap(manager.elementAt<LOCAL>(i), manager.elementAt<LOCAL>(j));
    std::swap(manager.elementAt<WORLD>(i), manager.elementAt<WORLD>(j));
    manager.swap(i, j); // this swaps the data relative to SingleInstanceComponentManager

    // now swap the linked-list references, to do that correctly we must use a temporary
    // node to fix-up the linked-list pointers
    // Here we are guaranteed to have enough capacity for our temporary storage, so we
    // can safely use the item just past the end of the array.
    assert(manager.getSoA().capacity() >= manager.getSoA().size() + 1);

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

// removes an node from the graph, but doesn't removes it or its children from the array
// (making everybody orphaned).
void FTransformManager::removeNode(Instance i) noexcept {
    auto& manager = mManager;
    Instance parent = manager[i].parent;
    Instance prev = manager[i].prev;
    Instance next = manager[i].next;
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
    // we no longer have a parent or siblings. we don't really have to clear thos fields
    // so we only do it in DEBUG mode
    manager[i].parent = 0;
    manager[i].prev = 0;
    manager[i].next = 0;
#endif
}

// update references to this node after it has been moved in the array
void FTransformManager::updateNode(Instance i) noexcept {
    auto& manager = mManager;
    // update our preview sibling's next reference (to ourselves)
    Instance parent = manager[i].parent;
    Instance prev = manager[i].prev;
    Instance next = manager[i].next;
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
        assert(child != i);
        manager[child].parent = i;
        child = manager[child].next;
    }
    validateNode(i);
    validateNode(parent);
    validateNode(prev);
    validateNode(next);
}

void FTransformManager::transformChildren(Sim& manager, Instance ci) noexcept {
    while (ci) {
        // update child's world transform
        Instance parent = manager[ci].parent;
        mat4f const& pt = manager[parent].world;
        mat4f const& local = manager[ci].local;
        manager[ci].world = pt * local;

        // assume we don't have a deep hierarchy
        Instance child = manager[ci].firstChild;
        if (UTILS_UNLIKELY(child)) {
            transformChildren(manager, child);
        }

        // process our next child
        ci = manager[ci].next;
    }
}

void FTransformManager::validateNode(Instance i) noexcept {
#ifndef NDEBUG
    auto& manager = mManager;
    if (i) {
        Instance parent = manager[i].parent;
        Instance firstChild = manager[i].firstChild;
        Instance prev = manager[i].prev;
        Instance next = manager[i].next;
        assert(parent != i);
        assert(prev != i);
        assert(next != i);
        assert(firstChild != i);
        if (prev) {
            if (parent) {
                assert(manager[parent].firstChild != i);
            }
            assert(manager[prev].next == i);
        } else {
            if (parent) {
                assert(manager[parent].firstChild == i);
            }
        }
        if (next) {
            assert(manager[next].prev == i);
        }
        if (parent) {
            // make sure we are in the child list of our parent
            Instance child = manager[parent].firstChild;
            assert(child);
            while (child && child != i) {
                child = manager[child].next;
            }
            assert(child);
        }
        if (firstChild) {
            assert(manager[firstChild].parent == i);
            assert(manager[firstChild].prev == 0);
        }
    }
#endif
}

void FTransformManager::gc(utils::EntityManager& em) noexcept {
    auto& manager = mManager;
    manager.gc(em, 4, [this](Entity e) {
                destroy(e);
            });
}

} // namespace details

using namespace details;

TransformManager::children_iterator& TransformManager::children_iterator::operator++() {
    FTransformManager const& that = upcast(mManager);
    mInstance = that.mManager[mInstance].next;
    return *this;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void TransformManager::create(Entity entity, Instance parent, const mat4f& worldTransform) {
    upcast(this)->create(entity, parent, worldTransform);
}

void TransformManager::destroy(Entity e) noexcept {
    upcast(this)->destroy(e);
}

bool TransformManager::hasComponent(Entity e) const noexcept {
    return upcast(this)->hasComponent(e);
}

TransformManager::Instance TransformManager::getInstance(Entity e) const noexcept {
    return upcast(this)->getInstance(e);
}

void TransformManager::setTransform(Instance ci, const mat4f& model) noexcept {
    upcast(this)->setTransform(ci, model);
}

const mat4f& TransformManager::getTransform(Instance ci) const noexcept {
    return upcast(this)->getTransform(ci);
}

const mat4f& TransformManager::getWorldTransform(Instance ci) const noexcept {
    return upcast(this)->getWorldTransform(ci);
}

void TransformManager::setParent(Instance i, Instance newParent) noexcept {
    upcast(this)->setParent(i, newParent);
}

utils::Entity TransformManager::getParent(Instance i) const noexcept {
    return upcast(this)->getParent(i);
}

size_t TransformManager::getChildCount(Instance i) const noexcept {
    return upcast(this)->getChildCount(i);
}

size_t TransformManager::getChildren(Instance i, utils::Entity* children,
        size_t count) const noexcept {
    return upcast(this)->getChildren(i, children, count);
}

void TransformManager::openLocalTransformTransaction() noexcept {
    upcast(this)->openLocalTransformTransaction();
}

void TransformManager::commitLocalTransformTransaction() noexcept {
    upcast(this)->commitLocalTransformTransaction();
}

TransformManager::children_iterator TransformManager::getChildrenBegin(
        TransformManager::Instance parent) const noexcept {
    return upcast(this)->getChildrenBegin(parent);
}

TransformManager::children_iterator TransformManager::getChildrenEnd(
        TransformManager::Instance parent) const noexcept {
    return upcast(this)->getChildrenEnd(parent);
}

} // namespace filament
