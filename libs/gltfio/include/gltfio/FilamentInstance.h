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

#ifndef GLTFIO_FILAMENTINSTANCE_H
#define GLTFIO_FILAMENTINSTANCE_H

#include <utils/Entity.h>

namespace gltfio {

class Animator;

/**
 * \class FilamentInstance FilamentInstance.h gltfio/FilamentInstance.h
 * \brief Provides access to a hierarchy of entities that have been instanced from a glTF asset.
 *
 * Every entity has a filament::TransformManager component, and some entities also have \c Name or
 * \c Renderable components.
 *
 * \see AssetLoader::createInstancedAsset()
 */
class FilamentInstance {
public:
    /**
     * Gets the list of entities in this instance, one for each glTF node. All of these have a
     * Transform component. Some of the returned entities may also have a Renderable component or
     * Name component.
     */
    const utils::Entity* getEntities() const noexcept;

    /**
     * Gets the number of entities returned by getEntities().
     */
    size_t getEntityCount() const noexcept;

    /** Gets the transform root for the instance, which has no matching glTF node. */
    utils::Entity getRoot() const noexcept;

    /**
     * Lazily creates the animation engine for the instance, or returns it from the cache.
     *
     * The animator is owned by the asset and should not be manually deleted.
     * The first time this is called, it must be called before FilamentAsset::releaseSourceData().
     */
    Animator* getAnimator() noexcept;
};

} // namespace gltfio

#endif // GLTFIO_FILAMENTINSTANCE_H
