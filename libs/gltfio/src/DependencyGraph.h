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
 *    Entity          Entity   Entity   Entity
 *      |            /      \     |     /
 *      |           /        \    |    /
 *   Material   Material       Material
 *             /   |    \          |
 *            /    |     \         |
 *        Param  Param   Param   Param
 *           \     /        \     /
 *            \   /          \   /
 *          Texture         Texture
 *
 * Note that the left-most entity in the above graph has no textures, so it becomes ready as soon as
 * finalize is called.
 */
class DependencyGraph {
public:
    using Material = filament::MaterialInstance;
    using Entity = utils::Entity;

    // Returns a ready-to-render entity, or 0 if no new entities have become renderable.
    Entity popReadyRenderable() noexcept;

    // These are called during the initial asset loader phase.
    void addEdge(Entity entity, Material* material);
    void addEdge(Material* material, const char* parameter);

    // This is called at the end of the initial asset loading phase.
    void finalize();

    // These are called after textures have created and decoded.
    void addEdge(filament::Texture* texture, Material* material, const char* parameter);
    void markAsReady(filament::Texture* texture);

private:
    struct TextureStatus {
        filament::Texture* texture;
        bool ready;
    };

    struct MaterialStatus {
        tsl::robin_map<std::string, TextureStatus*> params;
    };

    struct EntityStatus {
        tsl::robin_set<Material*> materials;
        size_t numReadyMaterials = 0;
    };

    void markAsReady(Material* material);
    TextureStatus* getStatus(filament::Texture* texture);

    tsl::robin_map<Entity, EntityStatus> mEntityToMaterial;
    tsl::robin_map<Material*, tsl::robin_set<Entity>> mMaterialToEntity;
    tsl::robin_map<Material*, MaterialStatus> mMaterialToTexture;
    tsl::robin_map<filament::Texture*, tsl::robin_set<Material*>> mTextureToMaterial;
    tsl::robin_map<filament::Texture*, std::unique_ptr<TextureStatus>> mTextures;
    std::queue<Entity> mReadyRenderables;
    bool mFinalized = false;
};

} // namespace gltfio

#endif // GLTFIO_DEPENDENCY_GRAPH_H
