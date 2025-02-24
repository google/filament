/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "state_tracker/state_object.h"

vvl::StateObject::~StateObject() { Destroy(); }

void vvl::StateObject::Destroy() {
    Invalidate();
    destroyed_ = true;
}

const VulkanTypedHandle* vvl::StateObject::InUse() const {
    // NOTE: for performance reasons, this method calls up the tree
    // with the read lock held.
    auto guard = ReadLockTree();
    for (auto& item : parent_nodes_) {
        auto node = item.second.lock();
        if (!node) {
            continue;
        }
        if (node->InUse()) {
            return &node->Handle();
        }
    }
    return nullptr;
}

bool vvl::StateObject::AddParent(StateObject* parent_node) {
    auto guard = WriteLockTree();
    auto result = parent_nodes_.emplace(parent_node->Handle(), std::weak_ptr<StateObject>(parent_node->shared_from_this()));
    return result.second;
}

void vvl::StateObject::RemoveParent(StateObject* parent_node) {
    assert(parent_node);
    auto guard = WriteLockTree();
    parent_nodes_.erase(parent_node->Handle());
}

// copy the current set of parents so that we don't need to hold the lock
// while calling NotifyInvalidate on them, as that would lead to recursive locking.
vvl::StateObject::NodeMap vvl::StateObject::GetParentsForInvalidate(bool unlink) {
    NodeMap result;
    if (unlink) {
        auto guard = WriteLockTree();
        result = std::move(parent_nodes_);
        parent_nodes_.clear();
    } else {
        auto guard = ReadLockTree();
        result = parent_nodes_;
    }
    return result;
}

vvl::StateObject::NodeMap vvl::StateObject::ObjectBindings() const {
    auto guard = ReadLockTree();
    return parent_nodes_;
}

void vvl::StateObject::Invalidate(bool unlink) {
    NodeList empty;
    // We do not want to call the virtual method here because any special handling
    // in an overriden NotifyInvalidate() is for when a child node has become invalid.
    // But calling Invalidate() indicates the current node is invalid.
    // Calling the default implementation directly here avoids duplicating it inline.
    StateObject::NotifyInvalidate(empty, unlink);
}

void vvl::StateObject::NotifyInvalidate(const NodeList& invalid_nodes, bool unlink) {
    auto current_parents = GetParentsForInvalidate(unlink);
    if (current_parents.empty()) {
        return;
    }

    NodeList up_nodes = invalid_nodes;
    up_nodes.emplace_back(shared_from_this());
    for (auto& item : current_parents) {
        auto node = item.second.lock();
        if (node && !node->Destroyed()) {
            node->NotifyInvalidate(up_nodes, unlink);
        }
    }
}
