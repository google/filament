/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef GLTFIO_DEPENDENCY_GRAPH_H
#define GLTFIO_DEPENDENCY_GRAPH_H

#include <utils/Entity.h>

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <queue>
#include <string>

namespace filament {
    class MaterialInstance;
    class Texture;
}

namespace gltfio {

/**
 * Internal graph that enables FilamentAsset to discover "ready-to-render" entities by tracking
 * the Texture objects that each entity depends on.
 *
 * Renderables connect to a set of material instances, which in turn connect to a set of parameter
 * names, which in turn connect to a set of texture objects. These relationships are not easily
 * inspectable using the Filament API or ECS.
 *
 * One graph corresponds to a single glTF asset. The graph only contains weak references, it does
 * not have ownership over any Filament objects. Here's an example:
 *
 *    Entity           Entity     Entity   Entity
 *      |            /        \     |     /
 *      |           /          \    |    /
 *   Material   Material         Material
 *             /   |    \           |
 *            /    |     \          |
 *        Param  Param   Param    Param
 *           \     /       |        |
 *            \   /        |        |
 *          Texture     Texture  Texture
 *
 * Note that the left-most entity in the above graph has no textures, so it becomes ready as soon as
 * finalize is called.
 */
class DependencyGraph {
public:
    using Material = filament::MaterialInstance;
    using Entity = utils::Entity;

    // Pops up to "count" ready-to-render entities off the queue.
    // If "result" is non-null, returns the number of written items.
    // If "result" is null, returns the number of available entities.
    size_t popRenderables(Entity* result, size_t count) noexcept;

    // These are called during the initial asset loader phase.
    void addEdge(Entity entity, Material* material);
    void addEdge(Material* material, const char* parameter);

    // This is called at the end of the initial asset loading phase.
    void finalize();

    // These are called after textures have created and decoded.
    void addEdge(filament::Texture* texture, Material* material, const char* parameter);
    void markAsReady(filament::Texture* texture);

private:
    struct TextureNode {
        filament::Texture* texture;
        bool ready;
    };

    struct MaterialNode {
        tsl::robin_map<std::string, TextureNode*> params;
    };

    struct EntityNode {
        tsl::robin_set<Material*> materials;
        size_t numReadyMaterials = 0;
    };

    void markAsReady(Material* material);
    TextureNode* getStatus(filament::Texture* texture);

    // The following maps contain the directed edges in the graph.
    tsl::robin_map<Entity, EntityNode> mEntityToMaterial;
    tsl::robin_map<Material*, tsl::robin_set<Entity>> mMaterialToEntity;
    tsl::robin_map<Material*, MaterialNode> mMaterialToTexture;
    tsl::robin_map<filament::Texture*, tsl::robin_set<Material*>> mTextureToMaterial;

    // Each texture (and its readiness flag) can be referenced from multiple nodes, so we own
    // a collection of wrapper objects in the following map. This uses std::unique_ptr to allow
    // nodes to refer to a texture wrapper using a stable weak pointer.
    tsl::robin_map<filament::Texture*, std::unique_ptr<TextureNode>> mTextureNodes;

    std::queue<Entity> mReadyRenderables;
    bool mFinalized = false;
};

} // namespace gltfio

#endif // GLTFIO_DEPENDENCY_GRAPH_H
