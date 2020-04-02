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

#include <unordered_map>
#include <map>
#include <vector>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <filamat/MaterialBuilder.h>
#include <filament/Color.h>
#include <filament/Box.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <assimp/scene.h>

class MeshAssimp {
public:
    using mat4f = filament::math::mat4f;
    using half4 = filament::math::half4;
    using short4 = filament::math::short4;
    using half2 = filament::math::half2;
    using ushort2 = filament::math::ushort2;
    explicit MeshAssimp(filament::Engine& engine);
    ~MeshAssimp();

    void addFromFile(const utils::Path& path,
            std::map<std::string, filament::MaterialInstance*>& materials,
            bool overrideMaterial = false);

    const std::vector<utils::Entity> getRenderables() const noexcept {
        return mRenderables;
    }

    //For use with normalizing coordinates
    filament::math::float3 minBound = filament::math::float3(1.0f);
    filament::math::float3 maxBound = filament::math::float3(-1.0f);
    utils::Entity rootEntity;

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
        mat4f accTransform;
    };

    struct Asset {
        utils::Path file;
        std::vector<uint32_t> indices;
        std::vector<half4> positions;
        std::vector<short4> tangents;
        std::vector<ushort2> texCoords0;
        std::vector<ushort2> texCoords1;
        bool snormUV0;
        bool snormUV1;
        std::vector<Mesh> meshes;
        std::vector<int> parents;
    };

    bool setFromFile(Asset& asset, std::map<std::string, filament::MaterialInstance*>& outMaterials);

    void processGLTFMaterial(const aiScene* scene, const aiMaterial* material,
            const std::string& materialName, const std::string& dirName,
            std::map<std::string, filament::MaterialInstance*>& outMaterials) const;

    template<bool SNORMUV0S, bool SNORMUV1S>
    void processNode(Asset& asset,
                     std::map<std::string, filament::MaterialInstance*>& outMaterials,
                     const aiScene *scene,
                     bool isGLTF,
                     size_t deep,
                     size_t matCount,
                     const aiNode *node,
                     int parentIndex,
                     size_t &depth) const;

    filament::Texture* createOneByOneTexture(uint32_t textureData);
    filament::Engine& mEngine;
    filament::VertexBuffer* mVertexBuffer = nullptr;
    filament::IndexBuffer* mIndexBuffer = nullptr;

    filament::Material* mDefaultColorMaterial = nullptr;
    filament::Material* mDefaultTransparentColorMaterial = nullptr;

    mutable std::unordered_map<uint64_t, filament::Material*> mGltfMaterialCache;
    filament::Texture* mDefaultMap = nullptr;
    filament::Texture* mDefaultNormalMap = nullptr;
    float mDefaultMetallic = 0.0;
    float mDefaultRoughness = 0.4;
    filament::sRGBColor mDefaultEmissive = filament::sRGBColor({0.0f, 0.0f, 0.0f});

    std::vector<utils::Entity> mRenderables;

    std::vector<filament::Texture*> mTextures;


};

#endif // TNT_FILAMENT_SAMPLE_MESH_ASSIMP_H
