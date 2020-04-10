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
     * Sets the SkyBox.
     *
     * The Skybox is drawn last and covers all pixels not touched by geometry.
     *
     * @param skybox The Skybox to use to fill untouched pixels, or nullptr to unset the Skybox.
     */
    void setSkybox(Skybox* skybox) noexcept;

    /**
     * Set the IndirectLight to use when rendering the Scene.
     *
     * Currently, a Scene may only have a single IndirectLight. This call replaces the current
     * IndirectLight.
     *
     * @param ibl The IndirectLight to use when rendering the Scene or nullptr to unset.
     */
    void setIndirectLight(IndirectLight const* ibl) noexcept;

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
     * Adds a contiguous list of entities to the Scene.
     *
     * @param entities Array containing entities to add to the scene.
     * @param count Size of the entity array.
     */
    void addEntities(const utils::Entity* entities, size_t count);

    /**
     * Removes the Renderable from the Scene.
     *
     * @param entity The Entity to remove from the Scene. If the specified
     *                   \p entity doesn't exist, this call is ignored.
     */
    void remove(utils::Entity entity);

    /**
     * Returns the number of Renderable objects in the Scene.
     *
     * @return number of Renderable objects in the Scene.
     */
    size_t getRenderableCount() const noexcept;

    /**
     * Returns the total number of Light objects in the Scene.
     *
     * @return The total number of Light objects in the Scene.
     */
    size_t getLightCount() const noexcept;

    /**
     * Returns true if the given entity is in the Scene.
     *
     * @return Whether the given entity is in the Scene.
     */
    bool hasEntity(utils::Entity entity) const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_SCENE_H
