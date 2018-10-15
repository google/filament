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

#ifndef TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H
#define TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H

namespace filament {
    class Engine;
    class VertexBuffer;
    class IndexBuffer;
    class Material;
    class MaterialInstance;
    class Renderable;
}

#include <map>
#include <vector>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <filamat/MaterialBuilder.h>
#include <filament/Color.h>
#include <filament/Box.h>
#include <filament/Texture.h>

class MeshAssimp {
public:
    using mat4f = math::mat4f;
    using half4 = math::half4;
    using short4 = math::short4;
    using half2 = math::half2;
    explicit MeshAssimp(filament::Engine& engine);
    ~MeshAssimp();

    void addFromFile(const utils::Path& path,
            std::map<std::string, filament::MaterialInstance*>& materials,
            bool overrideMaterial = false);

    const std::vector<utils::Entity> getRenderables() const noexcept {
        return mRenderables;
    }

private:
    struct Part {
        size_t offset;
        size_t count;
        std::string material;
        filament::sRGBColor baseColor;
        float opacity;
        float metallic;
        float roughness;
        float reflectance;
    };

    struct Mesh {
        size_t offset;
        size_t count;
        std::vector<Part> parts;
        filament::Box aabb;
        mat4f transform;
    };

    bool setFromFile(const utils::Path& file,
            std::vector<uint32_t>& outIndices,
            std::vector<half4>&    outPositions,
            std::vector<short4>&   outTangents,
            std::vector<half2>&    outTexCoords,
            std::vector<Mesh>&     outMeshes,
            std::vector<int>&      outParents,
            std::map<std::string, filament::MaterialInstance*>& outMaterials
            );


    filament::Engine& mEngine;
    filament::VertexBuffer* mVertexBuffer = nullptr;
    filament::IndexBuffer* mIndexBuffer = nullptr;

    filament::Material* mDefaultColorMaterial = nullptr;
    filament::Material* mDefaultTransparentColorMaterial = nullptr;
    filament::Material* mGltfMaterial = nullptr;

    filament::Texture* mDefaultMap = nullptr;
    filament::Texture* mDefaultNormalMap = nullptr;
    float mDefaultMetallic = 0.0;
    float mDefaultRoughness = 0.4;
    filament::sRGBColor mDefaultEmissive = filament::sRGBColor({0.0f, 0.0f, 0.0f});


    std::vector<utils::Entity> mRenderables;

    std::vector<filament::Texture*> mTextures;
};

#endif // TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H
