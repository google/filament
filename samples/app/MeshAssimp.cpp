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

#include "MeshAssimp.h"

#include <string.h>
#include <array>

#include <filament/Color.h>
#include <filament/VertexBuffer.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <math/norm.h>
#include <math/vec3.h>
#include <math/TVecHelpers.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>

#include <stb_image.h>

#include <stdlib.h>


using namespace filament;
using namespace filamat;
using namespace math;
using namespace utils;

static constexpr uint8_t DEFAULT_MATERIAL_PACKAGE[] = {
    #include "generated/material/aiDefaultMat.inc"
};

static constexpr uint8_t DEFAULT_TRANSPARENT_PACKAGE[] = {
    #include "generated/material/aiDefaultTrans.inc"
};

static constexpr uint8_t GLTF2_PACKAGE[] = {
    #include "generated/material/gltf2.inc"
};

static constexpr uint8_t GLTF2_DS_PACKAGE[] = {
    #include "generated/material/gltf2DoubleSided.inc"
};

static constexpr uint8_t GLTF2_TRANS_PACKAGE[] = {
    #include "generated/material/gltf2Trans.inc"
};

static constexpr uint8_t GLTF2_DS_TRANS_PACKAGE[] = {
    #include "generated/material/gltf2DoubleSidedTrans.inc"
};

static constexpr uint8_t GLTF2_MASKED_PACKAGE[] = {
    #include "generated/material/gltf2Masked.inc"
};

static constexpr uint8_t GLTF2_DS_MASKED_PACKAGE[] = {
    #include "generated/material/gltf2DoubleSidedMasked.inc"
};

static constexpr uint8_t GLTF2_UNLIT_PACKAGE[] = {
    #include "generated/material/gltf2Unlit.inc"
};

static constexpr uint8_t GLTF2_DS_UNLIT_PACKAGE[] = {
    #include "generated/material/gltf2DoubleSidedUnlit.inc"
};

Texture* MeshAssimp::createOneByOneTexture(uint32_t pixel) {
    uint32_t *textureData = (uint32_t *) malloc(sizeof(uint32_t));
    *textureData = pixel;

    Texture *texturePtr = Texture::Builder()
            .width(uint32_t(1))
            .height(uint32_t(1))
            .levels(0xff)
            .format(driver::TextureFormat::RGBA8)
            .build(mEngine);

    Texture::PixelBufferDescriptor defaultNormalBuffer(textureData,
            size_t(1 * 1 * 4),
            Texture::Format::RGBA,
            Texture::Type::UBYTE,
            (driver::BufferDescriptor::Callback) &free);

    texturePtr->setImage(mEngine, 0, std::move(defaultNormalBuffer));
    texturePtr->generateMipmaps(mEngine);

    return texturePtr;
}

MeshAssimp::MeshAssimp(Engine& engine) : mEngine(engine) {
    //Initialize some things here
    mDefaultMap = createOneByOneTexture(0xffffffff);
    mDefaultNormalMap = createOneByOneTexture(0xffff8080);

    mDefaultColorMaterial = Material::Builder()
            .package((void*) DEFAULT_MATERIAL_PACKAGE, sizeof(DEFAULT_MATERIAL_PACKAGE))
            .build(mEngine);

    mDefaultColorMaterial->setDefaultParameter("baseColor",   RgbType::LINEAR, float3{0.8});
    mDefaultColorMaterial->setDefaultParameter("metallic",    0.0f);
    mDefaultColorMaterial->setDefaultParameter("roughness",   0.4f);
    mDefaultColorMaterial->setDefaultParameter("reflectance", 0.5f);

    mDefaultTransparentColorMaterial = Material::Builder()
            .package((void*) DEFAULT_TRANSPARENT_PACKAGE, sizeof(DEFAULT_TRANSPARENT_PACKAGE))
            .build(mEngine);

    mDefaultTransparentColorMaterial->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    mDefaultTransparentColorMaterial->setDefaultParameter("metallic",  0.0f);
    mDefaultTransparentColorMaterial->setDefaultParameter("roughness", 0.4f);

    mGltfMaterial = Material::Builder()
            .package((void*) GLTF2_PACKAGE, sizeof(GLTF2_PACKAGE))
            .build(mEngine);
    mGltfMaterialDS = Material::Builder()
            .package((void*) GLTF2_DS_PACKAGE, sizeof(GLTF2_DS_PACKAGE))
            .build(mEngine);
    mGltfMaterialTrans = Material::Builder()
            .package((void*) GLTF2_TRANS_PACKAGE, sizeof(GLTF2_TRANS_PACKAGE))
            .build(mEngine);
    mGltfMaterialDSTrans = Material::Builder()
            .package((void*) GLTF2_DS_TRANS_PACKAGE, sizeof(GLTF2_DS_TRANS_PACKAGE))
            .build(mEngine);
    mGltfMaterialMasked = Material::Builder()
            .package((void*) GLTF2_MASKED_PACKAGE, sizeof(GLTF2_MASKED_PACKAGE))
            .build(mEngine);
    mGltfMaterialDSMasked = Material::Builder()
            .package((void*) GLTF2_DS_MASKED_PACKAGE, sizeof(GLTF2_DS_MASKED_PACKAGE))
            .build(mEngine);
    mGltfMaterialUnlit = Material::Builder()
            .package((void*) GLTF2_UNLIT_PACKAGE, sizeof(GLTF2_UNLIT_PACKAGE))
            .build(mEngine);
    mGltfMaterialDSUnlit = Material::Builder()
            .package((void*) GLTF2_DS_UNLIT_PACKAGE, sizeof(GLTF2_DS_UNLIT_PACKAGE))
            .build(mEngine);
}

MeshAssimp::~MeshAssimp() {
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);
    mEngine.destroy(mDefaultColorMaterial);
    mEngine.destroy(mDefaultTransparentColorMaterial);
    mEngine.destroy(mGltfMaterial);
    mEngine.destroy(mGltfMaterialDS);
    mEngine.destroy(mGltfMaterialTrans);
    mEngine.destroy(mGltfMaterialDSTrans);
    mEngine.destroy(mGltfMaterialMasked);
    mEngine.destroy(mGltfMaterialDSMasked);
    mEngine.destroy(mGltfMaterialUnlit);
    mEngine.destroy(mGltfMaterialDSUnlit);
    mEngine.destroy(mDefaultNormalMap);
    mEngine.destroy(mDefaultMap);

    for (Entity renderable : mRenderables) {
        mEngine.destroy(renderable);
    }

    for (Texture* texture : mTextures) {
        mEngine.destroy(texture);
    }

    // destroy the Entities itself
    EntityManager::get().destroy(mRenderables.size(), mRenderables.data());
}

template<typename T>
struct State {
    std::vector<T> state;
    explicit State(std::vector<T>&& state) : state(state) { }
    static void free(void* buffer, size_t size, void* user) {
        auto* const that = static_cast<State<T>*>(user);
        delete that;
    }
    size_t size() const { return state.size() * sizeof(T); }
    T const * data() const { return state.data(); }
};

//TODO: Remove redundant method from sample_full_pbr
static void loadTexture(Engine *engine, const std::string &filePath, Texture **map,
        bool sRGB, bool hasAlpha) {

    if (!filePath.empty()) {
        Path path(filePath);
        if (path.exists()) {
            int w, h, n;
            int numChannels = hasAlpha ? 4 : 3;

            driver::TextureFormat inputFormat;
            if (sRGB) {
                inputFormat = hasAlpha ? driver::TextureFormat::SRGB8_A8 : driver::TextureFormat::SRGB8;
            } else {
                inputFormat = hasAlpha ? driver::TextureFormat::RGBA8 : driver::TextureFormat::RGB8;
            }

            Texture::Format outputFormat = hasAlpha ? Texture::Format::RGBA : Texture::Format::RGB;

            uint8_t *data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, numChannels);
            if (data != nullptr) {
                *map = Texture::Builder()
                        .width(uint32_t(w))
                        .height(uint32_t(h))
                        .levels(0xff)
                        .format(inputFormat)
                        .build(*engine);

                Texture::PixelBufferDescriptor buffer(data,
                        size_t(w * h * numChannels),
                        outputFormat,
                        Texture::Type::UBYTE,
                        (driver::BufferDescriptor::Callback) &stbi_image_free);

                (*map)->setImage(*engine, 0, std::move(buffer));
                (*map)->generateMipmaps(*engine);
            } else {
                std::cout << "The texture " << path << " could not be loaded" << std::endl;
            }
        } else {
            std::cout << "The texture " << path << " does not exist" << std::endl;
        }
    }
}

void loadEmbeddedTexture(Engine *engine, aiTexture *embeddedTexture, Texture **map,
        bool sRGB, bool hasAlpha) {

    int w, h, n;
    int numChannels = hasAlpha ? 4 : 3;

    driver::TextureFormat inputFormat;
    if (sRGB) {
        inputFormat = hasAlpha ? driver::TextureFormat::SRGB8_A8 : driver::TextureFormat::SRGB8;
    } else {
        inputFormat = hasAlpha ? driver::TextureFormat::RGBA8 : driver::TextureFormat::RGB8;
    }

    Texture::Format outputFormat = hasAlpha ? Texture::Format::RGBA : Texture::Format::RGB;

    uint8_t *data = stbi_load_from_memory((unsigned char *) embeddedTexture->pcData,
            embeddedTexture->mWidth, &w, &h, &n, numChannels);

    *map = Texture::Builder()
            .width(uint32_t(w))
            .height(uint32_t(h))
            .levels(0xff)
            .format(inputFormat)
            .build(*engine);

    Texture::PixelBufferDescriptor defaultBuffer(data,
            size_t(w * h * numChannels),
            outputFormat,
            Texture::Type::UBYTE,
            (driver::BufferDescriptor::Callback) &free);

    (*map)->setImage(*engine, 0, std::move(defaultBuffer));
    (*map)->generateMipmaps(*engine);
}

// Takes a texture filename and returns the index of the embedded texture,
// -1 if the texture is not embedded
int32_t getEmbeddedTextureId(const aiString& path) {
    const char *pathStr = path.C_Str();
    if (path.length >= 2 && pathStr[0] == '*') {
        for (int i = 1; i < path.length; i++) {
            if (!isdigit(pathStr[i])) {
                return -1;
            }
        }
        return std::atoi(pathStr + 1);
    }
    return -1;
}

TextureSampler::WrapMode aiToFilamentMapMode(aiTextureMapMode mapMode) {
    switch(mapMode) {
        case aiTextureMapMode_Clamp :
            return TextureSampler::WrapMode::CLAMP_TO_EDGE;
        case aiTextureMapMode_Mirror :
            return TextureSampler::WrapMode::MIRRORED_REPEAT;
        default:
            return TextureSampler::WrapMode::REPEAT;
    }
}

// TODO: Change this to a member function (requires some alteration of cmakelsts.txt)
void setTextureFromPath(const aiScene *scene, Engine *engine,
        std::vector<filament::Texture*> textures, const aiString &textureFile,
        const std::string &materialName, const std::string &textureDirectory,
        aiTextureMapMode *mapMode, const char *parameterName,
        std::map<std::string, MaterialInstance *> &outMaterials) {

    TextureSampler sampler;
    if (mapMode) {
        sampler = TextureSampler(
                TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                TextureSampler::MagFilter::LINEAR,
                aiToFilamentMapMode(mapMode[0]),
                aiToFilamentMapMode(mapMode[1]),
                aiToFilamentMapMode(mapMode[2]));
    } else {
        sampler = TextureSampler(
                TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                TextureSampler::MagFilter::LINEAR,
                TextureSampler::WrapMode::REPEAT);
    }

    Texture* textureMap = nullptr;
    int32_t embeddedId = getEmbeddedTextureId(textureFile);

    // TODO: change this in refactor
    bool isSRGB = strcmp(parameterName, "baseColorMap") == 0 || strcmp(parameterName, "emissiveMap") == 0;
    bool hasAlpha = strcmp(parameterName, "baseColorMap") == 0;

    if (embeddedId != -1) {
        loadEmbeddedTexture(engine, scene->mTextures[embeddedId], &textureMap, isSRGB, hasAlpha);
    } else {
        loadTexture(engine, textureDirectory + textureFile.C_Str(), &textureMap, isSRGB, hasAlpha);
    }

    textures.push_back(textureMap);

    if (textureMap != nullptr) {
        outMaterials[materialName]->setParameter(parameterName, textureMap, sampler);
    }
}

template<typename VECTOR, typename INDEX>
Box computeTransformedAABB(VECTOR const* vertices, INDEX const* indices, size_t count,
        const mat4f& transform) noexcept {
    size_t stride = sizeof(VECTOR);
    math::float3 bmin(std::numeric_limits<float>::max());
    math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const*>(
                (char const*) vertices + indices[i] * stride);
        const math::float3 v(p->x, p->y, p->z);
        float3 tv = (transform * float4(v, 1.0f)).xyz;
        bmin = min(bmin, tv);
        bmax = max(bmax, tv);
    }
    return Box().set(bmin, bmax);
}

void MeshAssimp::addFromFile(const Path& path,
        std::map<std::string, MaterialInstance*>& materials, bool overrideMaterial) {

    std::vector<Mesh> meshes;
    std::vector<int> parents;

    { // This scope to make sure we're not using std::move()'d objects later
        std::vector<uint32_t> indices;
        std::vector<half4> positions;
        std::vector<short4> tangents;
        std::vector<half2> texCoords;

        // TODO: if we had a way to allocate temporary buffers from the engine with a
        // "command buffer" lifetime, we wouldn't need to have to deal with freeing the
        // std::vectors here.

        //TODO: a lot of these method arguments should probably be class or global variables
        if (!setFromFile(path, indices, positions, tangents, texCoords, meshes, parents, materials)) {
            return;
        }

        mVertexBuffer = VertexBuffer::Builder()
                .vertexCount((uint32_t)positions.size())
                .bufferCount(3)
                .attribute(VertexAttribute::POSITION,     0, VertexBuffer::AttributeType::HALF4)
                .attribute(VertexAttribute::TANGENTS,     1, VertexBuffer::AttributeType::SHORT4)
                .attribute(VertexAttribute::UV0,          2, VertexBuffer::AttributeType::HALF2)
                .normalized(VertexAttribute::TANGENTS)
                .build(mEngine);

        auto ps = new State<half4>(std::move(positions));
        auto ns = new State<short4>(std::move(tangents));
        auto ts = new State<half2>(std::move(texCoords));
        auto is = new State<uint32_t>(std::move(indices));

        mVertexBuffer->setBufferAt(mEngine, 0,
                VertexBuffer::BufferDescriptor(ps->data(), ps->size(), State<half4>::free, ps));

        mVertexBuffer->setBufferAt(mEngine, 1,
                VertexBuffer::BufferDescriptor(ns->data(), ns->size(), State<short4>::free, ns));

        mVertexBuffer->setBufferAt(mEngine, 2,
                VertexBuffer::BufferDescriptor(ts->data(), ts->size(), State<half2>::free, ts));

        mIndexBuffer = IndexBuffer::Builder().indexCount(uint32_t(is->size())).build(mEngine);
        mIndexBuffer->setBuffer(mEngine,
                IndexBuffer::BufferDescriptor(is->data(), is->size(), State<uint32_t>::free, is));
    }

    // always add the DefaultMaterial (with its default parameters), so we don't pick-up
    // whatever defaults is used in mesh
    if (materials.find(AI_DEFAULT_MATERIAL_NAME) == materials.end()) {
        materials[AI_DEFAULT_MATERIAL_NAME] = mDefaultColorMaterial->createInstance();
    }

    size_t startIndex = mRenderables.size();
    mRenderables.resize(startIndex + meshes.size());
    EntityManager::get().create(meshes.size(), mRenderables.data() + startIndex);
    EntityManager::get().create(1, &rootEntity);

    TransformManager& tcm = mEngine.getTransformManager();
    //Add root instance
    tcm.create(rootEntity, TransformManager::Instance{}, mat4f());

    for (auto& mesh : meshes) {
        RenderableManager::Builder builder(mesh.parts.size());
        builder.boundingBox(mesh.aabb);

        size_t partIndex = 0;
        for (auto& part : mesh.parts) {
            builder.geometry(partIndex, RenderableManager::PrimitiveType::TRIANGLES,
                    mVertexBuffer, mIndexBuffer, part.offset, part.count);

            if (overrideMaterial) {
                builder.material(partIndex, materials[AI_DEFAULT_MATERIAL_NAME]);
            } else {
                auto pos = materials.find(part.material);

                if (pos != materials.end()) {
                    builder.material(partIndex, pos->second);
                } else {
                    MaterialInstance* colorMaterial;
                    if (part.opacity < 1.0f) {
                        colorMaterial = mDefaultTransparentColorMaterial->createInstance();
                        colorMaterial->setParameter("baseColor", RgbaType::sRGB,
                                sRGBColorA { part.baseColor, part.opacity });
                    } else {
                        colorMaterial = mDefaultColorMaterial->createInstance();
                        colorMaterial->setParameter("baseColor", RgbType::sRGB, part.baseColor);
                        colorMaterial->setParameter("reflectance", part.reflectance);
                    }
                    colorMaterial->setParameter("metallic", part.metallic);
                    colorMaterial->setParameter("roughness", part.roughness);
                    builder.material(partIndex, colorMaterial);
                    materials[part.material] = colorMaterial;
                }
            }
            partIndex++;
        }

        const size_t meshIndex = &mesh - meshes.data();
        Entity entity = mRenderables[startIndex + meshIndex];
        if (!mesh.parts.empty()) {
            builder.build(mEngine, entity);
        }
        auto pindex = parents[meshIndex];
        TransformManager::Instance parent((pindex < 0) ?
                tcm.getInstance(rootEntity) : tcm.getInstance(mRenderables[pindex]));
        tcm.create(entity, parent, mesh.transform);
    }
}

using Assimp::Importer;

bool MeshAssimp::setFromFile(const Path& file, std::vector<uint32_t>& outIndices,
        std::vector<half4>& outPositions, std::vector<short4>& outTangents,
        std::vector<half2>& outTexCoords, std::vector<Mesh>& outMeshes,
        std::vector<int>& outParents, std::map<std::string, MaterialInstance*>& outMaterials) {

    Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

    aiScene const* scene = importer.ReadFile(file,
            // normals and tangents
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            // UV Coordinates
            aiProcess_GenUVCoords |
            // topology optimization
            aiProcess_FindInstances |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices |
            // misc optimization
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            // we only support triangles
            aiProcess_Triangulate);

    scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace);
    size_t index = importer.GetImporterIndex(file.getExtension().c_str());
    const aiImporterDesc* importerDesc = importer.GetImporterInfo(index);
    bool isGLTF = importerDesc &&
            (!strncmp("glTF Importer",  importerDesc->mName, 13) ||
             !strncmp("glTF2 Importer", importerDesc->mName, 14));

    if (!scene) {
        std::cout << "No scene" << std::endl;
    }

    if (scene && !scene->mRootNode) {
        std::cout << "No root node" << std::endl;
    }

    // we could use those, but we want to keep the graph if any, for testing
    //      aiProcess_OptimizeGraph
    //      aiProcess_PreTransformVertices

    const std::function<void(aiNode const* node, size_t& totalVertexCount, size_t& totalIndexCount)>
            countVertices = [scene, &countVertices]
            (aiNode const* node, size_t& totalVertexCount, size_t& totalIndexCount) {
        for (size_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh const *mesh = scene->mMeshes[node->mMeshes[i]];
            totalVertexCount += mesh->mNumVertices;

            const aiFace *faces = mesh->mFaces;
            const size_t numFaces = mesh->mNumFaces;
            totalIndexCount += numFaces * faces[0].mNumIndices;
        }

        for (size_t i = 0; i < node->mNumChildren; i++) {
            countVertices(node->mChildren[i], totalVertexCount, totalIndexCount);
        }
    };

    size_t deep = 0;
    size_t depth = 0;
    size_t matCount = 0;

    const std::function<void(aiNode const* node, int parentIndex)> processNode =
            [&](aiNode const* node, int parentIndex) {

        mat4f const& current = transpose(*reinterpret_cast<mat4f const*>(&node->mTransformation));

        size_t totalIndices = 0;
        outParents.push_back(parentIndex);
        outMeshes.push_back(Mesh{});
        outMeshes.back().offset = outIndices.size();
        outMeshes.back().transform = current;

        mat4f parentTransform = parentIndex >= 0 ? outMeshes[parentIndex].accTransform : mat4f();
        outMeshes.back().accTransform = parentTransform * current;

        for (size_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];

            float3 const* positions  = reinterpret_cast<float3 const*>(mesh->mVertices);
            float3 const* tangents   = reinterpret_cast<float3 const*>(mesh->mTangents);
            float3 const* bitangents = reinterpret_cast<float3 const*>(mesh->mBitangents);
            float3 const* normals    = reinterpret_cast<float3 const*>(mesh->mNormals);
            float3 const* texCoords  = reinterpret_cast<const float3*>(mesh->mTextureCoords[0]);

            const size_t numVertices = mesh->mNumVertices;
            if (numVertices > 0) {
                const aiFace* faces = mesh->mFaces;
                const size_t numFaces = mesh->mNumFaces;

                if (numFaces > 0) {
                    size_t indicesOffset = outPositions.size();

                    for (size_t j = 0; j < numVertices; j++) {
                        float3 normal = normals[j];
                        float3 texCoord = texCoords ? texCoords[j] : float3{0.0};
                        float3 tangent;
                        float3 bitangent;

                        //If the tangent and bitangent don't exist, make arbitrary ones
                        // TODO: The glTF specification recommends using the MikkTSpace algorithm
                        //       for computing tangent vectors in the absence of explicit tangents.
                        if (!tangents) {
                            bitangent = norm(cross(normal, float3{1.0, 0.0, 0.0}));
                            tangent = norm(cross(normal, bitangent));
                        } else {
                            tangent = tangents[j];
                            bitangent = bitangents[j];
                        }

                        quatf q = mat3f::packTangentFrame({tangent, bitangent, normal});
                        outTangents.push_back(packSnorm16(q.xyzw));
                        outTexCoords.emplace_back(texCoord.xy);
                        outPositions.emplace_back(positions[j], 1.0_h);
                    }

                    // all faces should be triangles since we configure assimp to triangulate faces
                    size_t indicesCount = numFaces * faces[0].mNumIndices;
                    size_t indexBufferOffset = outIndices.size();
                    totalIndices += indicesCount;

                    for (size_t j = 0; j < numFaces; ++j) {
                        const aiFace& face = faces[j];
                        for (size_t k = 0; k < face.mNumIndices; ++k) {
                            outIndices.push_back(uint32_t(face.mIndices[k] + indicesOffset));
                        }
                    }

                    uint32_t materialId = mesh->mMaterialIndex;
                    aiMaterial const* material = scene->mMaterials[materialId];

                    aiString name;
                    std::string materialName;

                    if (material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
                        if (isGLTF) {
                            while (outMaterials.find("_mat_" + std::to_string(matCount))
                                   != outMaterials.end()) {
                                matCount++;
                            }
                            materialName = "_mat_" + std::to_string(matCount);
                        } else {
                            materialName = AI_DEFAULT_MATERIAL_NAME;
                        }
                    } else {
                        materialName = name.C_Str();
                    }

                    if (isGLTF && outMaterials.find(materialName) == outMaterials.end()) {
                        std::string dirName = file.getParent();
                        processGLTFMaterial(scene, material, materialName, dirName, outMaterials);
                    }

                    aiColor3D color;
                    sRGBColor baseColor{1.0f};
                    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
                        baseColor = *reinterpret_cast<sRGBColor*>(&color);
                    }

                    float opacity;
                    if (material->Get(AI_MATKEY_OPACITY, opacity) != AI_SUCCESS) {
                        opacity = 1.0f;
                    }
                    if (opacity <= 0.0f) opacity = 1.0f;

                    float shininess;
                    if (material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS) {
                        shininess = 0.0f;
                    }

                    // convert shininess to roughness
                    float roughness = std::sqrt(2.0f / (shininess + 2.0f));

                    float metallic = 0.0f;
                    float reflectance = 0.5f;
                    if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
                        // if there's a non-grey specular color, assume a metallic surface
                        if (color.r != color.g && color.r != color.b) {
                            metallic = 1.0f;
                            baseColor = *reinterpret_cast<sRGBColor*>(&color);
                        } else {
                            if (baseColor.r == 0.0f && baseColor.g == 0.0f && baseColor.b == 0.0f) {
                                metallic = 1.0f;
                                baseColor = *reinterpret_cast<sRGBColor*>(&color);
                            } else {
                                // TODO: the conversion formula is correct
                                // reflectance = sqrtf(color.r / 0.16f);
                            }
                        }
                    }

                    outMeshes.back().parts.push_back({
                            indexBufferOffset, indicesCount, materialName,
                            baseColor, opacity, metallic, roughness, reflectance
                    });
                }
            }
        }
        if (node->mNumMeshes > 0) {
            outMeshes.back().count = totalIndices;
        }

        if (node->mNumChildren) {
            parentIndex = static_cast<int>(outMeshes.size()) - 1;
            deep++;
            depth = std::max(deep, depth);
            for (size_t i = 0, c = node->mNumChildren; i < c; i++) {
                processNode(node->mChildren[i], parentIndex);
            }
            deep--;
        }
    };

    if (scene) {
        aiNode const* node = scene->mRootNode;

        size_t totalVertexCount = 0;
        size_t totalIndexCount = 0;

        countVertices(node, totalVertexCount, totalIndexCount);

        outPositions.reserve(outPositions.size() + totalVertexCount);
        outTangents.reserve(outTangents.size() + totalVertexCount);
        outTexCoords.reserve(outTexCoords.size() + totalVertexCount);
        outIndices.reserve(outIndices.size() + totalIndexCount);

        processNode(node, -1);

        std::cout << "Hierarchy depth = " << depth << std::endl;

        // compute the aabb and find bounding box of entire model
        for (auto& mesh : outMeshes) {
            mesh.aabb = RenderableManager::computeAABB(
                    outPositions.data(),
                    outIndices.data() + mesh.offset,
                    mesh.count);

            Box transformedAabb = computeTransformedAABB(
                    outPositions.data(),
                    outIndices.data() + mesh.offset,
                    mesh.count,
                    mesh.accTransform);

            float3 aabbMin = transformedAabb.getMin();
            float3 aabbMax = transformedAabb.getMax();

            if (!isinf(aabbMin.x) && !isinf(aabbMax.x)) {
                if (minBound.x > maxBound.x) {
                    minBound.x = aabbMin.x;
                    maxBound.x = aabbMax.x;
                } else {
                    minBound.x = fmin(minBound.x, aabbMin.x);
                    maxBound.x = fmax(maxBound.x, aabbMax.x);
                }
            }

            if (!isinf(aabbMin.y) && !isinf(aabbMax.y)) {
                if (minBound.y > maxBound.y) {
                    minBound.y = aabbMin.y;
                    maxBound.y = aabbMax.y;
                } else {
                    minBound.y = fmin(minBound.y, aabbMin.y);
                    maxBound.y = fmax(maxBound.y, aabbMax.y);
                }
            }

            if (!isinf(aabbMin.z) && !isinf(aabbMax.z)) {
                if (minBound.z > maxBound.z) {
                    minBound.z = aabbMin.z;
                    maxBound.z = aabbMax.z;
                } else {
                    minBound.z = fmin(minBound.z, aabbMin.z);
                    maxBound.z = fmax(maxBound.z, aabbMax.z);
                }
            }
        }
        return true;
    }
    return false;
}

void MeshAssimp::processGLTFMaterial(const aiScene* scene, const aiMaterial* material,
        const std::string& materialName, const std::string& dirName,
         std::map<std::string, MaterialInstance*>& outMaterials) const {

    aiString baseColorPath;
    aiString AOPath;
    aiString MRPath;
    aiString normalPath;
    aiString emissivePath;
    aiTextureMapMode mapMode[3];

    bool materialIsDoubleSided = false;
    material->Get("$mat.twosided", 0, 0, materialIsDoubleSided);

    bool materialIsUnlit = false;
    material->Get("$mat.gltf.unlit", 0, 0, materialIsUnlit);

    aiString alphaMode;

    if (materialIsUnlit) {
        if (materialIsDoubleSided){
            outMaterials[materialName] = mGltfMaterialDSUnlit->createInstance();
        } else {
            outMaterials[materialName] = mGltfMaterialUnlit->createInstance();
        }
    } else if (materialIsDoubleSided) {
        material->Get("$mat.gltf.alphaMode", 0, 0, alphaMode);

        if (strcmp(alphaMode.C_Str(), "BLEND") == 0) {
            outMaterials[materialName] = mGltfMaterialDSTrans->createInstance();
        } else if (strcmp(alphaMode.C_Str(), "MASK") == 0) {
            outMaterials[materialName] = mGltfMaterialDSMasked->createInstance();
            float maskThreshold = 0.5;
            material->Get("$mat.gltf.alphaCutoff", 0, 0, maskThreshold);
            outMaterials[materialName]->setParameter("maskThreshold", maskThreshold);
        } else {
            outMaterials[materialName] = mGltfMaterialDS->createInstance();
        }
    } else {
         material->Get("$mat.gltf.alphaMode", 0, 0, alphaMode);

         if (strcmp(alphaMode.C_Str(), "BLEND") == 0) {
             outMaterials[materialName] = mGltfMaterialTrans->createInstance();
         } else if (strcmp(alphaMode.C_Str(), "MASK") == 0) {
             outMaterials[materialName] = mGltfMaterialMasked->createInstance();
             float maskThreshold = 0.5;
             material->Get("$mat.gltf.alphaCutoff", 0, 0, maskThreshold);
             outMaterials[materialName]->setParameter("maskThreshold", maskThreshold);
         } else {
             outMaterials[materialName] = mGltfMaterial->createInstance();
         }
    }

    // Load property values for gltf files
    aiColor4D baseColorFactor;
    aiColor3D emissiveFactor;
    float metallicFactor = 1.0;
    float roughnessFactor = 1.0;

    // TODO: is occlusion strength available on Assimp now?

    // Load texture images for gltf files
    TextureSampler sampler(
            TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
            TextureSampler::MagFilter::LINEAR,
            TextureSampler::WrapMode::REPEAT);

    if (material->GetTexture(aiTextureType_DIFFUSE, 1, &baseColorPath,
            nullptr, nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        setTextureFromPath(scene, &mEngine, mTextures, baseColorPath,
                materialName, dirName, mapMode, "baseColorMap", outMaterials);
    } else {
        outMaterials[materialName]->setParameter("baseColorMap", mDefaultMap, sampler);
    }

    if (material->GetTexture(aiTextureType_UNKNOWN, 0, &MRPath,
            nullptr, nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        setTextureFromPath(scene, &mEngine, mTextures, MRPath, materialName,
                dirName, mapMode, "metallicRoughnessMap", outMaterials);
    } else {
        outMaterials[materialName]->setParameter("metallicRoughnessMap", mDefaultMap, sampler);
        outMaterials[materialName]->setParameter("metallicFactor", mDefaultMetallic);
        outMaterials[materialName]->setParameter("roughnessFactor", mDefaultRoughness);
    }

    if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &AOPath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        setTextureFromPath(scene, &mEngine, mTextures, AOPath, materialName,
                dirName, mapMode, "aoMap", outMaterials);
    } else {
        outMaterials[materialName]->setParameter("aoMap", mDefaultMap, sampler);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        setTextureFromPath(scene, &mEngine, mTextures, normalPath, materialName,
                dirName, mapMode, "normalMap", outMaterials);
    } else {
        outMaterials[materialName]->setParameter("normalMap", mDefaultNormalMap, sampler);
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissivePath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        setTextureFromPath(scene, &mEngine, mTextures, emissivePath,
                materialName, dirName, mapMode, "emissiveMap", outMaterials);
    }  else {
        outMaterials[materialName]->setParameter("emissiveMap", mDefaultMap, sampler);
        outMaterials[materialName]->setParameter("emissiveFactor", mDefaultEmissive);
    }

    //If the gltf has texture factors, override the default factor values
    if (material->Get("$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, metallicFactor)
            == AI_SUCCESS) {
        outMaterials[materialName]->setParameter("metallicFactor", metallicFactor);
    }

    if (material->Get("$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, roughnessFactor)
            == AI_SUCCESS) {
        outMaterials[materialName]->setParameter("roughnessFactor", roughnessFactor);
    }

    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor) == AI_SUCCESS) {
        sRGBColor emissiveFactorCast = *reinterpret_cast<sRGBColor*>(&emissiveFactor);
        outMaterials[materialName]->setParameter("emissiveFactor", emissiveFactorCast);
    }

    if (material->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, baseColorFactor)
            == AI_SUCCESS) {
        sRGBColorA baseColorFactorCast = *reinterpret_cast<sRGBColorA*>(&baseColorFactor);
        outMaterials[materialName]->setParameter("baseColorFactor", baseColorFactorCast);
    }
}
