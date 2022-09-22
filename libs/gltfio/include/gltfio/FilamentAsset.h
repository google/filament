/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef GLTFIO_FILAMENTASSET_H
#define GLTFIO_FILAMENTASSET_H

#include <filament/Box.h>
#include <filament/TextureSampler.h>

#include <gltfio/NodeManager.h>

#include <utils/compiler.h>
#include <utils/Entity.h>

namespace filament {
    class Camera;
    class Engine;
    class MaterialInstance;
    class Scene;
}

namespace filament::gltfio {

class Animator;
class FilamentInstance;

/**
 * \class FilamentAsset FilamentAsset.h gltfio/FilamentAsset.h
 * \brief Owns a bundle of Filament objects that have been created by AssetLoader.
 *
 * For usage instructions, see the documentation for AssetLoader.
 *
 * This class owns a hierarchy of entities that have been loaded from a glTF asset. Every entity has
 * a filament::TransformManager component, and some entities also have \c Name, \c Renderable,
 * \c Light, \c Camera, or \c Node components.
 *
 * In addition to the aforementioned entities, an asset has strong ownership over a list of
 * filament::VertexBuffer, filament::IndexBuffer, filament::Texture,
 * and, optionally, a simple animation engine (gltfio::Animator).
 *
 * Clients must use ResourceLoader to create filament::Texture objects, compute tangent quaternions,
 * and upload data into vertex buffers and index buffers.
 *
 * \todo Only the default glTF scene is loaded, other glTF scenes are ignored.
 */
class UTILS_PUBLIC FilamentAsset {
public:
    using Entity = utils::Entity;
    using SceneMask = NodeManager::SceneMask;

    /**
     * Gets the list of entities, one for each glTF node. All of these have a Transform component.
     * Some of the returned entities may also have a Renderable component and/or a Light component.
     */
    const Entity* getEntities() const noexcept;

    /**
     * Gets the number of entities returned by getEntities().
     */
    size_t getEntityCount() const noexcept;

    /**
     * Gets the list of entities in the scene representing lights. All of these have a Light component.
     */
    const Entity* getLightEntities() const noexcept;

    /**
     * Gets the number of entities returned by getLightEntities().
     */
    size_t getLightEntityCount() const noexcept;

    /**
     * Gets the list of entities in the asset that have renderable components.
     */
    const utils::Entity* getRenderableEntities() const noexcept;

    /**
     * Gets the number of entities returned by getRenderableEntities().
     */
    size_t getRenderableEntityCount() const noexcept;

    /**
     * Gets the list of entities in the scene representing cameras. All of these have a \c Camera
     * component.
     *
     * Note about aspect ratios:
     * gltfio always uses an aspect ratio of 1.0 when setting the projection matrix for perspective
     * cameras. gltfio then sets the camera's scaling matrix with the aspect ratio specified in the
     * glTF file (if present).
     *
     * The camera's scaling matrix allows clients to adjust the aspect ratio independently from the
     * camera's projection.
     *
     * To change the aspect ratio of the glTF camera:
     *
     *     camera->setScaling(double4 {1.0 / newAspectRatio, 1.0, 1.0, 1.0});
     *
     * @see filament::Camera::setScaling
     */
    const Entity* getCameraEntities() const noexcept;

    /**
     * Gets the number of entities returned by getCameraEntities().
     */
    size_t getCameraEntityCount() const noexcept;

    /**
     * Gets the transform root for the asset, which has no matching glTF node.
     *
     * This node exists for convenience, allowing users to transform the entire asset. For instanced
     * assets, this is a "super root" where each of its children is a root in a particular instance.
     * This allows users to transform all instances en masse if they wish to do so.
     */
    Entity getRoot() const noexcept;

    /**
     * Pops a ready renderable off the queue, or returns 0 if no renderables have become ready.
     *
     * NOTE: To determine the progress percentage or completion status, please use
     * ResourceLoader#asyncGetLoadProgress. To get the number of ready renderables,
     * please use popRenderables().
     *
     * This method allows clients to progressively add the asset's renderables to the scene as
     * textures gradually become ready through asynchronous loading. For example, on every frame
     * progressive applications can do something like this:
     *
     *    while (Entity e = popRenderable()) { scene.addEntity(e); }
     *
     * Progressive reveal is not supported for dynamically added instances.
     *
     * \see ResourceLoader#asyncBeginLoad
     * \see popRenderables()
     */
    Entity popRenderable() noexcept;

    /**
     * Pops up to "count" ready renderables off the queue, or returns the available number.
     *
     * The given pointer should either be null or point to memory that can hold up to count
     * entities. If the pointer is null, returns the number of available renderables. Otherwise
     * returns the number of entities that have been written.
     *
     * \see ResourceLoader#asyncBeginLoad
     */
    size_t popRenderables(Entity* entities, size_t count) noexcept;

    /** Gets resource URIs for all externally-referenced buffers. */
    const char* const* getResourceUris() const noexcept;

    /** Gets the number of resource URIs returned by getResourceUris(). */
    size_t getResourceUriCount() const noexcept;

    /**
     * Gets the bounding box computed from the supplied min / max values in glTF accessors.
     *
     * This does not return a bounding box over all FilamentInstance, it's just a straightforward
     * AAAB that can be determined at load time from the asset data.
     */
    filament::Aabb getBoundingBox() const noexcept;

    /** Gets the NameComponentManager label for the given entity, if it exists. */
    const char* getName(Entity) const noexcept;

    /** Returns the first entity with the given name, or 0 if none exist. */
    Entity getFirstEntityByName(const char* name) noexcept;

    /**
     * Gets a list of entities with the given name.
     *
     * @param name Null-terminated string to match.
     * @param entities Pointer to an array to populate.
     * @param maxCount Maximum number of entities to retrieve.
     *
     * @return If entities is non-null, the number of entities written to the entity pointer.
     * Otherwise this returns the number of entities with the given name.
     */
    size_t getEntitiesByName(const char* name, Entity* entities,
            size_t maxCount) const noexcept;

    /**
     * Gets a list of entities whose names start with the given prefix.
     *
     * @param prefix Null-terminated prefix string to match.
     * @param entities Pointer to an array to populate.
     * @param maxCount Maximum number of entities to retrieve.
     *
     * @return If entities is non-null, the number of entities written to the entity pointer.
     * Otherwise this returns the number of entities with the given prefix.
     */
    size_t getEntitiesByPrefix(const char* prefix, Entity* entities,
            size_t maxCount) const noexcept;

    /** Gets the glTF extras string for a specific node, or for the asset, if it exists. */
    const char* getExtras(Entity entity = {}) const noexcept;

    /**
     * Gets the morph target name at the given index in the given entity.
     */
    const char* getMorphTargetNameAt(Entity entity, size_t targetIndex) const noexcept;

    /**
     * Returns the number of morph targets in the given entity.
     */
    size_t getMorphTargetCountAt(Entity entity) const noexcept;

    /**
     * Lazily creates a single LINES renderable that draws the transformed bounding-box hierarchy
     * for diagnostic purposes. The wireframe is owned by the asset so clients should not delete it.
     */
    Entity getWireframe() noexcept;

    /**
     * Returns the Filament engine associated with the AssetLoader that created this asset.
     */
    filament::Engine* getEngine() const noexcept;

    /**
     * Reclaims CPU-side memory for URI strings, binding lists, and raw animation data.
     *
     * This should only be called after ResourceLoader::loadResources().
     * If this is an instanced asset, this prevents creation of new instances.
     */
    void releaseSourceData() noexcept;

    /**
     * Returns a weak reference to the underlying cgltf hierarchy. This becomes invalid after
     * calling releaseSourceData().
     */
    const void* getSourceAsset() noexcept;

    /**
     * Returns the number of scenes in the asset.
     */
    size_t getSceneCount() const noexcept;

    /**
     * Returns the name of the given scene.
     *
     * Returns null if the given scene does not have a name or is out of bounds.
     */
    const char* getSceneName(size_t sceneIndex) const noexcept;

    /**
     * Adds entities to a Filament scene only if they belong to at least one of the given glTF
     * scenes.
     *
     * This is just a helper that provides an alternative to directly calling scene->addEntities()
     * and provides filtering functionality.
     */
    void addEntitiesToScene(filament::Scene& targetScene, const Entity* entities, size_t count,
            SceneMask sceneFilter) const;

    /**
     * Releases ownership of entities and their Filament components.
     *
     * This makes the client take responsibility for destroying Filament
     * components (e.g. Renderable, TransformManager component) as well as
     * the underlying entities.
     */
    void detachFilamentComponents() noexcept;

    bool areFilamentComponentsDetached() const noexcept;

    /**
     * Convenience function to get the first instance, or null if it doesn't exist.
     */
    FilamentInstance* getInstance() noexcept {
        return getAssetInstanceCount() > 0 ? getAssetInstances()[0] : nullptr;
    }

    /*! \cond PRIVATE */

    FilamentInstance** getAssetInstances() noexcept;
    size_t getAssetInstanceCount() const noexcept;

protected:
    FilamentAsset() noexcept = default;
    ~FilamentAsset() = default;

public:
    FilamentAsset(FilamentAsset const&) = delete;
    FilamentAsset(FilamentAsset&&) = delete;
    FilamentAsset& operator=(FilamentAsset const&) = delete;
    FilamentAsset& operator=(FilamentAsset&&) = delete;
    /*! \endcond */
};

} // namespace filament::gltfio

#endif // GLTFIO_FILAMENTASSET_H
