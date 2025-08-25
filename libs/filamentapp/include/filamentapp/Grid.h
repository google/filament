/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SAMPLE_GRID_H
#define TNT_FILAMENT_SAMPLE_GRID_H

#include <filament/Engine.h>
#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <utils/Entity.h>

#include <functional>

class Grid {
public:

    Grid(filament::Engine& engine, filament::Material const* material,
        filament::math::float3 linearColor);

    Grid(Grid const&) = delete;
    Grid& operator=(Grid const&) = delete;

    Grid(Grid&& rhs) noexcept;

    utils::Entity getWireFrameRenderable() const {
        return mWireFrameRenderable;
    }

    ~Grid();

    using Generator = std::function<float(int index)>;

    void update(uint32_t width, uint32_t height, uint32_t depth);

    void update(uint32_t width, uint32_t height, uint32_t depth,
            Generator const& genWidth, Generator const& genHeight, Generator const& genDepth);

    void mapFrustum(filament::Engine& engine, filament::Camera const* camera);
    void mapFrustum(filament::Engine& engine, filament::math::mat4 const& transform);
    void mapAabb(filament::Engine& engine, filament::Box const& box);

private:
    filament::Engine& mEngine;
    filament::VertexBuffer* mVertexBuffer = nullptr;
    filament::IndexBuffer* mIndexBuffer = nullptr;
    filament::Material const* mMaterial = nullptr;
    filament::MaterialInstance* mMaterialInstanceWireFrame = nullptr;
    utils::Entity mWireFrameRenderable{};
};


#endif // TNT_FILAMENT_SAMPLE_GRID_H
