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

namespace filament::gltfio {

/**
 * Internal graph that enables FilamentAsset to discover "ready-to-render" entities by tracking
 * the loading status of Texture objects that each entity depends on.
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
    //
    // If "result" is non-null, returns the number of written items.
    // If "result" is null, returns the number of available entities.
    size_t popRenderables(Entity* result, size_t count) noexcept;

    // These are called during the initial asset loader phase.
    void addEdge(Entity entity, Material* material);
    void addEdge(Material* material, const char* parameter);
    void addEdge(filament::Texture* texture, Material* material, const char* parameter);

    // Marks the end of synchronous asset loading.
    //
    // At this point, the graph enters a finalized state and all non-textured entities are
    // immediately marked as "ready". However textures are not yet fully decoded.
    //
    // After finalization, the only nodes that can be added to the graph are entities.
    void finalize();

    // Marks the given texture as being fully decoded, with all miplevels initialized.
    //
    // This can only be called on a finalized graph.
    void markAsReady(filament::Texture* texture);

    // Marks the material as ready, but due to an error.
    //
    // This should be called when it known that at least one of the material's texture
    // dependencies will never become available.
    void markAsError(Material* material) { markAsReady(material); }

    // Re-checks the readiness of all entities after finalization.
    //
    // This exists only to support dynamic instancing.  It is slower than finalize() because it
    // checks the readiness of existing materials.
    void refinalize();

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

    void checkReadiness(Material* material);
    void markAsReady(Material* material);
    TextureNode* getStatus(filament::Texture* texture);

    // The following maps contain the directed edges in the graph.
    tsl::robin_map<Entity, EntityNode, Entity::Hasher> mEntityToMaterial;
    tsl::robin_map<Material*, tsl::robin_set<Entity, Entity::Hasher>> mMaterialToEntity;
    tsl::robin_map<Material*, MaterialNode> mMaterialToTexture;
    tsl::robin_map<filament::Texture*, tsl::robin_set<Material*>> mTextureToMaterial;

    // Each texture (and its readiness flag) can be referenced from multiple nodes, so we own
    // a collection of wrapper objects in the following map. This uses std::unique_ptr to allow
    // nodes to refer to a texture wrapper using a stable weak pointer.
    tsl::robin_map<filament::Texture*, std::unique_ptr<TextureNode>> mTextureNodes;

    std::queue<Entity> mReadyRenderables;
    bool mFinalized = false;
};

} // namespace filament::gltfio

#endif // GLTFIO_DEPENDENCY_GRAPH_H
