/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_SCENE_H
#define TNT_FILAMENT_SCENE_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <stddef.h>

namespace utils {
    class Entity;
} // namespace utils

namespace filament {

class IndirectLight;
class Skybox;

/**
 * A Scene is a flat container of Renderable and Light instances.
 *
 * A Scene doesn't provide a hierarchy of Renderable objects, i.e.: it's not a scene-graph.
 * However, it manages the list of objects to render and the list of lights. Renderable
 * and Light objects can be added or removed from a Scene at any time.
 *
 * A Renderable *must* be added to a Scene in order to be rendered, and the Scene must be
 * provided to a View.
 *
 *
 * Creation and Destruction
 * ========================
 *
 * A Scene is created using Engine.createScene() and destroyed using
 * Engine.destroy(const Scene*).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Scene.h>
 * #include <filament/Engine.h>
 * using namespace filament;
 *
 * Engine* engine = Engine::create();
 *
 * Scene* scene = engine->createScene();
 * engine->destroy(&scene);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see View, Renderable, Light
 */
class UTILS_PUBLIC Scene : public FilamentAPI {
public:

    /**
     * Sets the Skybox.
     *
     * The Skybox is drawn last and covers all pixels not touched by geometry.
     *
     * @param skybox The Skybox to use to fill untouched pixels, or nullptr to unset the Skybox.
     */
    void setSkybox(Skybox* UTILS_NULLABLE skybox) noexcept;

    /**
     * Returns the Skybox associated with the Scene.
     *
     * @return The associated Skybox, or nullptr if there is none.
     */
    Skybox* UTILS_NULLABLE getSkybox() const noexcept;

    /**
     * Set the IndirectLight to use when rendering the Scene.
     *
     * Currently, a Scene may only have a single IndirectLight. This call replaces the current
     * IndirectLight.
     *
     * @param ibl The IndirectLight to use when rendering the Scene or nullptr to unset.
     * @see getIndirectLight
     */
    void setIndirectLight(IndirectLight* UTILS_NULLABLE ibl) noexcept;

    /**
     * Get the IndirectLight or nullptr if none is set.
     *
     * @return the the IndirectLight or nullptr if none is set
     * @see setIndirectLight
     */
    IndirectLight* UTILS_NULLABLE getIndirectLight() const noexcept;

    /**
     * Adds an Entity to the Scene.
     *
     * @param entity The entity is ignored if it doesn't have a Renderable or Light component.
     *
     * \attention
     *  A given Entity object can only be added once to a Scene.
     *
     */
    void addEntity(utils::Entity entity);

    /**
     * Adds a list of entities to the Scene.
     *
     * @param entities Array containing entities to add to the scene.
     * @param count Size of the entity array.
     */
    void addEntities(const utils::Entity* UTILS_NONNULL entities, size_t count);

    /**
     * Removes the Renderable from the Scene.
     *
     * @param entity The Entity to remove from the Scene. If the specified
     *                   \p entity doesn't exist, this call is ignored.
     */
    void remove(utils::Entity entity);

    /**
     * Removes a list of entities to the Scene.
     *
     * This is equivalent to calling remove in a loop.
     * If any of the specified entities do not exist in the scene, they are skipped.
     *
     * @param entities Array containing entities to remove from the scene.
     * @param count Size of the entity array.
     */
    void removeEntities(const utils::Entity* UTILS_NONNULL entities, size_t count);

    /**
     * Returns the total number of Entities in the Scene, whether alive or not.
     * @return Total number of Entities in the Scene.
     */
    size_t getEntityCount() const noexcept;

    /**
     * Returns the number of active (alive) Renderable objects in the Scene.
     *
     * @return The number of active (alive) Renderable objects in the Scene.
     */
    size_t getRenderableCount() const noexcept;

    /**
     * Returns the number of active (alive) Light objects in the Scene.
     *
     * @return The number of active (alive) Light objects in the Scene.
     */
    size_t getLightCount() const noexcept;

    /**
     * Returns true if the given entity is in the Scene.
     *
     * @return Whether the given entity is in the Scene.
     */
    bool hasEntity(utils::Entity entity) const noexcept;

    /**
     * Invokes user functor on each entity in the scene.
     *
     * It is not allowed to add or remove an entity from the scene within the functor.
     *
     * @param functor User provided functor called for each entity in the scene
     */
    void forEach(utils::Invocable<void(utils::Entity entity)>&& functor) const noexcept;

protected:
    // prevent heap allocation
    ~Scene() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_SCENE_H
