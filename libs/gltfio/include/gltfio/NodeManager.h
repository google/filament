/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef GLTFIO_NODEMANAGER_H
#define GLTFIO_NODEMANAGER_H

#include <filament/FilamentAPI.h>

#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/EntityInstance.h>
#include <utils/FixedCapacityVector.h>

namespace utils {
class Entity;
} // namespace utils

namespace filament::gltfio {

class FNodeManager;

/**
 * NodeManager is used to add annotate entities with glTF-specific information.
 *
 * Node components are created by gltfio and exposed to users to allow inspection.
 *
 * Nodes do not store the glTF hierarchy or names; see TransformManager and NameComponentManager.
 */
class UTILS_PUBLIC NodeManager {
public:
    using Instance = utils::EntityInstance<NodeManager>;
    using Entity = utils::Entity;
    using CString = utils::CString;
    using SceneMask = utils::bitset32;

    static constexpr size_t MAX_SCENE_COUNT = 32;

    /**
     * Returns whether a particular Entity is associated with a component of this NodeManager
     * @param e An Entity.
     * @return true if this Entity has a component associated with this manager.
     */
    bool hasComponent(Entity e) const noexcept;

    /**
     * Gets an Instance representing the node component associated with the given Entity.
     * @param e An Entity.
     * @return An Instance object, which represents the node component associated with the Entity e.
     * @note Use Instance::isValid() to make sure the component exists.
     * @see hasComponent()
     */
    Instance getInstance(Entity e) const noexcept;

    /**
     * Creates a node component and associates it with the given entity.
     * @param entity            An Entity to associate a node component with.
     *
     * If this component already exists on the given entity, it is first destroyed as if
     * destroy(Entity e) was called.
     *
     * @see destroy()
     */
    void create(Entity entity);

    /**
     * Destroys this component from the given entity.
     * @param e An entity.
     *
     * @see create()
     */
    void destroy(Entity e) noexcept;

    void setMorphTargetNames(Instance ci, utils::FixedCapacityVector<CString> names) noexcept;
    const utils::FixedCapacityVector<CString>& getMorphTargetNames(Instance ci) const noexcept;

    void setExtras(Instance ci, CString extras) noexcept;
    const CString& getExtras(Instance ci) const noexcept;

    void setSceneMembership(Instance ci, SceneMask scenes) noexcept;
    SceneMask getSceneMembership(Instance ci) const noexcept;

protected:
    NodeManager() noexcept = default;
    ~NodeManager() = default;

public:
    NodeManager(NodeManager const&) = delete;
    NodeManager(NodeManager&&) = delete;
    NodeManager& operator=(NodeManager const&) = delete;
    NodeManager& operator=(NodeManager&&) = delete;
};

} // namespace filament::gltfio


#endif // GLTFIO_NODEMANAGER_H
