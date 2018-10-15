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

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>

#include <stb_image.h>


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

MeshAssimp::MeshAssimp(Engine& engine) : mEngine(engine) {
}

MeshAssimp::~MeshAssimp() {
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);
    mEngine.destroy(mDefaultColorMaterial);
    mEngine.destroy(mDefaultTransparentColorMaterial);
    mEngine.destroy(mGltfMaterial);
    mEngine.destroy(mDefaultNormalMap);
    mEngine.destroy(mDefaultMap);
    for (Entity renderable : mRenderables) {
        mEngine.destroy(renderable);
    }

    for (Texture *tex : mTextures) {
        mEngine.destroy(tex);
    }

    // destroy the Entities itself
    EntityManager::get().destroy(mRenderables.size(), mRenderables.data());
}

template<typename T>
struct State {
    std::vector<T> state;
    explicit State(std::vector<T>&& state) : state(state) { }
    static void free(void* buffer, size_t size, void* user) {
        auto* const that = (State<T>*)user;
        delete that;
    }
    size_t size() const { return state.size() * sizeof(T); }
    T const * data() const { return state.data(); }
};

//TODO: Remove redundant method from sample_full_pbr
void loadTex(Engine* engine, const std::string& filePath, Texture** map, bool sRGB = true) {
    if (!filePath.empty()) {
        Path path(filePath);
        if (path.exists()) {
            int w, h, n;
            unsigned char* data = stbi_load(path.getAbsolutePath().c_str(), &w, &h, &n, 3);
            if (data != nullptr) {
                *map = Texture::Builder()
                        .width(uint32_t(w))
                        .height(uint32_t(h))
                        .levels(0xff)
                        .format(sRGB ? driver::TextureFormat::SRGB8 : driver::TextureFormat::RGB8)
                        .build(*engine);
                Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 3),
                                                      Texture::Format::RGB, Texture::Type::UBYTE,
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

        // Make default map for all textures except Normal
        int defaultTexSize = 4;
        unsigned char *defaultTexData = (unsigned char *) malloc(defaultTexSize*sizeof(unsigned char*));
        for (int i=0; i < defaultTexSize; i++){
            defaultTexData[i] = static_cast<unsigned char>(255);
        }
        mDefaultMap = Texture::Builder()
                .width(uint32_t(1))
                .height(uint32_t(1))
                .levels(0xff)
                .format(driver::TextureFormat::SRGB8)
                .build(mEngine);
        Texture::PixelBufferDescriptor defaultBuffer(defaultTexData, size_t(1 * 1 * 3),
                                                     Texture::Format::RGB, Texture::Type::UBYTE
                ,(driver::BufferDescriptor::Callback) &free);
        mDefaultMap->setImage(mEngine, 0, std::move(defaultBuffer));
        mDefaultMap->generateMipmaps(mEngine);

        //Make default Normal Map
        unsigned char *defaultNormalTexData = (unsigned char *) malloc(defaultTexSize*sizeof(unsigned char*));
        defaultNormalTexData[0] = static_cast<unsigned char>(128);
        defaultNormalTexData[1] = static_cast<unsigned char>(128);
        defaultNormalTexData[2] = static_cast<unsigned char>(255);
        defaultNormalTexData[3] = static_cast<unsigned char>(0);

        mDefaultNormalMap = Texture::Builder()
                .width(uint32_t(1))
                .height(uint32_t(1))
                .levels(0xff)
                .format(driver::TextureFormat::SRGB8)
                .build(mEngine);
        Texture::PixelBufferDescriptor defaultNormalBuffer(defaultNormalTexData, size_t(1 * 1 * 3),
                                                     Texture::Format::RGB, Texture::Type::UBYTE
                ,(driver::BufferDescriptor::Callback) &free);
        mDefaultNormalMap->setImage(mEngine, 0, std::move(defaultNormalBuffer));
        mDefaultNormalMap->generateMipmaps(mEngine);

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

    // always add the DefaultMaterial (with its default parameters), so we don't pick-up
    // whatever defaults is used in mesh
    if (materials.find(AI_DEFAULT_MATERIAL_NAME) == materials.end()) {
        materials[AI_DEFAULT_MATERIAL_NAME] = mDefaultColorMaterial->createInstance();
    }



    size_t startIndex = mRenderables.size();
    mRenderables.resize(startIndex + meshes.size());
    EntityManager::get().create(meshes.size(), mRenderables.data() + startIndex);

    TransformManager& tcm = mEngine.getTransformManager();

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

                //TODO: add usage of gltf materials
                //Load all gltf textures
                auto pos = materials.find(part.material);

                if (pos != materials.end()) {
                    builder.material(partIndex, pos->second);
                } else {
                    std::cout << "I don't think this should be running" << std::endl;
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
                TransformManager::Instance{} : tcm.getInstance(mRenderables[pindex]));
        tcm.create(entity, parent, mesh.transform);
    }
}

using Assimp::Importer;

bool MeshAssimp::setFromFile(const Path& file,
        std::vector<uint32_t>& outIndices,
        std::vector<half4>&    outPositions,
        std::vector<short4>&   outTangents,
        std::vector<half2>&    outTexCoords,
        std::vector<Mesh>&     outMeshes,
        std::vector<int>&      outParents,
        std::map<std::string, MaterialInstance*>& outMaterials
        ) {
    Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

    aiScene const* scene = importer.ReadFile(file,
            // normals and tangents
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            // topology optimization
            aiProcess_FindInstances |
            aiProcess_OptimizeMeshes |
            aiProcess_JoinIdenticalVertices |
            // misc optimization
            aiProcess_ImproveCacheLocality |
            aiProcess_SortByPType |
            // we only support triangles
            aiProcess_Triangulate);

    if (!scene){
        std::cout << "no scene" << std::endl;
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

    const std::function<void(aiNode const* node, int parentIndex)> processNode =
            [this, file, scene, &processNode, &outParents, &deep, &depth,
                    &outIndices, &outPositions, &outTangents, &outTexCoords, &outMeshes, &outMaterials]
            (aiNode const* node, int parentIndex) {

        mat4f const& current = transpose(*reinterpret_cast<mat4f const*>(&node->mTransformation));

        size_t totalIndices = 0;
        outParents.push_back(parentIndex);
        outMeshes.push_back(Mesh{});
        outMeshes.back().offset = outIndices.size();
        outMeshes.back().transform = current;

        // Bias and scale factor when storing tangent frames in normalized short4
        const float bias = 1.0f / 32767.0f;
        const float factor = (float) (sqrt(1.0 - (double) bias * (double) bias));

        mGltfMaterial = Material::Builder()
                .package((void*) GLTF2_PACKAGE, sizeof(GLTF2_PACKAGE))
                .build(mEngine);

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
                        quatf q = mat3f::packTangentFrame({tangents[j], bitangents[j], normals[j]});
                        outTangents.push_back(packSnorm16(q.xyzw));
                        outTexCoords.emplace_back(texCoords[j].xy);
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

                    //Get filepaths for PBR textures

                    int texIndex = 0;
                    aiString baseColorPath;
                    aiString AOPath;
                    aiString MRPath;
                    aiString normalPath;
                    aiString emissivePath;
                    aiString name;
                    std::string materialName;

                    if (material->Get(AI_MATKEY_NAME, name) != AI_SUCCESS) {
                        materialName = AI_DEFAULT_MATERIAL_NAME;
                    } else {
                        materialName = name.C_Str();
                    }

                    // TODO: Add support for non-image path values and binary gltf

                    TextureSampler sampler(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR,
                                           TextureSampler::MagFilter::LINEAR, TextureSampler::WrapMode::REPEAT);
                    sampler.setAnisotropy(8.0f);

                    std::string dirName = file.getParent();
                    std::cout << "directory: " << dirName << std::endl;

                    if (outMaterials.find(materialName) == outMaterials.end()) {

                        outMaterials[materialName] = mGltfMaterial->createInstance();

                        // Load property values for gltf files
                        aiColor4D baseColorFactor;
                        sRGBColorA baseColorFactorCast{1.0f};
                        aiColor3D emissiveFactor;
                        sRGBColor emissiveFactorCast{1.0f};
                        float metallicFactor = 1.0;
                        float roughnessFactor = 1.0;

//                        if(material->Get("$mat.gltf.occlusionTexture.strength", 0, 0, occlusionStrength) == AI_SUCCESS){
//                            std::cout << "hey" << std::endl;
//                        }

                        // Load texture images for gltf files
                        if (material->GetTexture(aiTextureType_DIFFUSE, 1, &baseColorPath) == AI_SUCCESS) {
                            Texture* baseColorMap = nullptr;
                            loadTex(&mEngine, dirName + baseColorPath.C_Str(), &baseColorMap);
                            mTextures.push_back(baseColorMap);
                            if (baseColorMap != nullptr) {
                                outMaterials[materialName]->setParameter(
                                        "baseColorMap", baseColorMap, sampler);
                            }
                            std::cout << dirName + baseColorPath.C_Str() << std::endl;
                        } else {
                            if (mDefaultMap == nullptr){
                                std::cout << "default mat is null" << std::endl;
                            }
                            outMaterials[materialName]->setParameter("baseColorMap", mDefaultMap, sampler);
                        }

                        if (material->GetTexture(aiTextureType_UNKNOWN, 0, &MRPath) == AI_SUCCESS) {
                            Texture* metallicRoughnessMap = nullptr;
                            loadTex(&mEngine, dirName + MRPath.C_Str(), &metallicRoughnessMap, false);
                            mTextures.push_back(metallicRoughnessMap);

                            if (metallicRoughnessMap != nullptr) {
                                outMaterials[materialName]->setParameter(
                                        "metallicRoughnessMap", metallicRoughnessMap, sampler);
                            }
                            std::cout << dirName + MRPath.C_Str() << std::endl;
                        } else {
                            outMaterials[materialName]->setParameter("metallicRoughnessMap", mDefaultMap, sampler);
                            outMaterials[materialName]->setParameter("metallicFactor", mDefaultMetallic);
                            outMaterials[materialName]->setParameter("roughnessFactor", mDefaultRoughness);
                        }

                        if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &AOPath) == AI_SUCCESS) {
                            Texture* aoMap = nullptr;
                            loadTex(&mEngine, dirName + AOPath.C_Str(), &aoMap, false);
                            mTextures.push_back(aoMap);

                            if (aoMap != nullptr) {
                                outMaterials[materialName]->setParameter(
                                        "aoMap", aoMap, sampler);
                            }
                            std::cout << dirName + AOPath.C_Str() << std::endl;
                        } else {
                            outMaterials[materialName]->setParameter("aoMap", mDefaultMap, sampler);
                        }

                        if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath) == AI_SUCCESS) {
                            Texture* normalMap = nullptr;
                            loadTex(&mEngine, dirName + normalPath.C_Str(), &normalMap, false);
                            mTextures.push_back(normalMap);

                            if (normalMap != nullptr) {
                                outMaterials[materialName]->setParameter(
                                        "normalMap", normalMap, sampler);
                            }
                            std::cout << dirName + normalPath.C_Str() << std::endl;
                        } else {
                            outMaterials[materialName]->setParameter("normalMap", mDefaultNormalMap, sampler);
                        }

                        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissivePath) == AI_SUCCESS) {
                            Texture* emissiveMap = nullptr;
                            loadTex(&mEngine, dirName + emissivePath.C_Str(), &emissiveMap, false);
                            mTextures.push_back(emissiveMap);

                            if (emissiveMap != nullptr) {
                                outMaterials[materialName]->setParameter(
                                        "emissiveMap", emissiveMap, sampler);
                            }
                            std::cout << dirName + emissivePath.C_Str() << std::endl;
                        }  else {
                            outMaterials[materialName]->setParameter("emissiveMap", mDefaultMap, sampler);
                            outMaterials[materialName]->setParameter("emissiveFactor", mDefaultEmissive);
                        }

                        //If the gltf has texture factors, override the default factor values
                        if(material->Get("$mat.gltf.pbrMetallicRoughness.metallicFactor", 0, 0, metallicFactor) == AI_SUCCESS){
                            outMaterials[materialName]->setParameter("metallicFactor", metallicFactor);
                        }
                        if (material->Get("$mat.gltf.pbrMetallicRoughness.roughnessFactor", 0, 0, roughnessFactor) == AI_SUCCESS){
                            outMaterials[materialName]->setParameter("roughnessFactor", roughnessFactor);
                        }
                        if(material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor) == AI_SUCCESS){
                            emissiveFactorCast = *reinterpret_cast<sRGBColor*>(&emissiveFactor);
                            outMaterials[materialName]->setParameter("emissiveFactor", emissiveFactorCast);
                        }
                        if(material->Get("$mat.gltf.pbrMetallicRoughness.baseColorFactor", 0, 0, baseColorFactor) == AI_SUCCESS){
                            baseColorFactorCast = *reinterpret_cast<sRGBColorA*>(&baseColorFactor);
                            outMaterials[materialName]->setParameter("baseColorFactor", baseColorFactorCast);
                        }

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
            // std::cout << depth << ": num children = " << node->mNumChildren
            //      << ", parent = " << parentIndex << std::endl;
            for (size_t i = 0, c = node->mNumChildren; i < c; i++) {
                processNode(node->mChildren[i], parentIndex);
            }
            deep--;
        }
    };

    if (scene) {
        aiNode const* node = scene->mRootNode;
        if(!node){
            std::cout << scene->mNumMeshes << std::endl;
        }

        size_t totalVertexCount = 0;
        size_t totalIndexCount = 0;

        countVertices(node, totalVertexCount, totalIndexCount);

        outPositions.reserve(outPositions.size() + totalVertexCount);
        outTangents.reserve(outTangents.size() + totalVertexCount);
        outTexCoords.reserve(outTexCoords.size() + totalVertexCount);
        outIndices.reserve(outIndices.size() + totalIndexCount);

        processNode(node, -1);

        std::cout << "Hierarchy depth = " << depth << std::endl;

        // compute the aabb
        for (auto& mesh : outMeshes) {
            mesh.aabb = RenderableManager::computeAABB(
                    outPositions.data(),
                    outIndices.data() + mesh.offset,
                    mesh.count);
        }

        return true;
    }

    return false;
}
