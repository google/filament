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

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

#include <filamentapp/MeshAssimp.h>

#include <stdlib.h>
#include <string.h>

#include <array>
#include <iostream>

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
#include <assimp/pbrmaterial.h>

#include <stb_image.h>

#include <backend/DriverEnums.h>

#include "generated/resources/filamentapp.h"

using namespace filament;
using namespace filamat;
using namespace filament::math;
using namespace utils;

enum class AlphaMode : uint8_t {
    OPAQUE,
    MASKED,
    TRANSPARENT
};

struct MaterialConfig {
    bool doubleSided = false;
    bool unlit = false;
    bool hasVertexColors = false;
    AlphaMode alphaMode = AlphaMode::OPAQUE;
    float maskThreshold = 0.5f;
    uint8_t baseColorUV = 0;
    uint8_t metallicRoughnessUV = 0;
    uint8_t emissiveUV = 0;
    uint8_t aoUV = 0;
    uint8_t normalUV = 0;

    uint8_t maxUVIndex() {
        return std::max({baseColorUV, metallicRoughnessUV, emissiveUV, aoUV, normalUV});
    }
};

void appendBooleanToBitMask(uint64_t &bitmask, bool b) {
    bitmask <<= 1;
    bitmask |= b;
}

uint64_t hashMaterialConfig(MaterialConfig config) {
    uint64_t bitmask = 0;
    memcpy(&bitmask, &config.maskThreshold, sizeof(config.maskThreshold));
    appendBooleanToBitMask(bitmask, config.doubleSided);
    appendBooleanToBitMask(bitmask, config.unlit);
    appendBooleanToBitMask(bitmask, config.hasVertexColors);
    appendBooleanToBitMask(bitmask, config.alphaMode == AlphaMode::OPAQUE);
    appendBooleanToBitMask(bitmask, config.alphaMode == AlphaMode::MASKED);
    appendBooleanToBitMask(bitmask, config.alphaMode == AlphaMode::TRANSPARENT);
    appendBooleanToBitMask(bitmask, config.baseColorUV == 0);
    appendBooleanToBitMask(bitmask, config.metallicRoughnessUV == 0);
    appendBooleanToBitMask(bitmask, config.emissiveUV == 0);
    appendBooleanToBitMask(bitmask, config.aoUV == 0);
    appendBooleanToBitMask(bitmask, config.normalUV == 0);
    return bitmask;
}

std::string shaderFromConfig(MaterialConfig config) {
    std::string shader = R"SHADER(
        void material(inout MaterialInputs material) {
    )SHADER";

    shader += "float2 normalUV = getUV" + std::to_string(config.normalUV) + "();\n";
    shader += "float2 baseColorUV = getUV" + std::to_string(config.baseColorUV) + "();\n";
    shader += "float2 metallicRoughnessUV = getUV" + std::to_string(config.metallicRoughnessUV) + "();\n";
    shader += "float2 aoUV = getUV" + std::to_string(config.aoUV) + "();\n";
    shader += "float2 emissiveUV = getUV" + std::to_string(config.emissiveUV) + "();\n";

    if (!config.unlit) {
        shader += R"SHADER(
            material.normal = texture(materialParams_normalMap, normalUV).xyz * 2.0 - 1.0;
            material.normal.y = -material.normal.y;
        )SHADER";
    }

    shader += R"SHADER(
        prepareMaterial(material);
        material.baseColor = texture(materialParams_baseColorMap, baseColorUV);
        material.baseColor *= materialParams.baseColorFactor;
    )SHADER";

    if (config.alphaMode == AlphaMode::TRANSPARENT) {
        shader += R"SHADER(
            material.baseColor.rgb *= material.baseColor.a;
        )SHADER";
    }

    if (!config.unlit) {
        shader += R"SHADER(
            vec4 metallicRoughness = texture(materialParams_metallicRoughnessMap, metallicRoughnessUV);
            material.roughness = materialParams.roughnessFactor * metallicRoughness.g;
            material.metallic = materialParams.metallicFactor * metallicRoughness.b;
            material.ambientOcclusion = texture(materialParams_aoMap, aoUV).r;
            material.emissive.rgb = texture(materialParams_emissiveMap, emissiveUV).rgb;
            material.emissive.rgb *= materialParams.emissiveFactor.rgb;
            material.emissive.a = 0.0;
        )SHADER";
    }

    shader += "}\n";
    return shader;
}

Material* createMaterialFromConfig(Engine& engine, MaterialConfig config ) {
    std::string shader = shaderFromConfig(config);
    MaterialBuilder::init();
    MaterialBuilder builder;
    builder
            .name("material")
            .material(shader.c_str())
            .doubleSided(config.doubleSided)
            .require(VertexAttribute::UV0)
            .parameter("baseColorMap", MaterialBuilder::SamplerType::SAMPLER_2D)
            .parameter("baseColorFactor", MaterialBuilder::UniformType::FLOAT4)
            .parameter("metallicRoughnessMap", MaterialBuilder::SamplerType::SAMPLER_2D)
            .parameter("aoMap", MaterialBuilder::SamplerType::SAMPLER_2D)
            .parameter("emissiveMap", MaterialBuilder::SamplerType::SAMPLER_2D)
            .parameter("normalMap", MaterialBuilder::SamplerType::SAMPLER_2D)
            .parameter("metallicFactor", MaterialBuilder::UniformType::FLOAT)
            .parameter("roughnessFactor", MaterialBuilder::UniformType::FLOAT)
            .parameter("normalScale", MaterialBuilder::UniformType::FLOAT)
            .parameter("aoStrength", MaterialBuilder::UniformType::FLOAT)
            .parameter("emissiveFactor", MaterialBuilder::UniformType::FLOAT3);

    if (config.maxUVIndex() > 0) {
        builder.require(VertexAttribute::UV1);
    }

    switch(config.alphaMode) {
        case AlphaMode::MASKED : builder.blending(MaterialBuilder::BlendingMode::MASKED);
            builder.maskThreshold(config.maskThreshold);
            break;
        case AlphaMode::TRANSPARENT : builder.blending(MaterialBuilder::BlendingMode::TRANSPARENT);
            break;
        default : builder.blending(MaterialBuilder::BlendingMode::OPAQUE);
    }

    builder.shading(config.unlit ? Shading::UNLIT : Shading::LIT);

    Package pkg = builder.build(engine.getJobSystem());
    return Material::Builder().package(pkg.getData(), pkg.getSize()).build(engine);
}

Texture* MeshAssimp::createOneByOneTexture(uint32_t pixel) {
    uint32_t *textureData = (uint32_t *) malloc(sizeof(uint32_t));
    *textureData = pixel;

    Texture *texturePtr = Texture::Builder()
            .width(uint32_t(1))
            .height(uint32_t(1))
            .levels(0xff)
            .format(Texture::InternalFormat::RGBA8)
            .build(mEngine);

    Texture::PixelBufferDescriptor defaultNormalBuffer(textureData,
            size_t(1 * 1 * 4),
            Texture::Format::RGBA,
            Texture::Type::UBYTE,
            (Texture::PixelBufferDescriptor::Callback) &free);

    texturePtr->setImage(mEngine, 0, std::move(defaultNormalBuffer));
    texturePtr->generateMipmaps(mEngine);

    return texturePtr;
}

void getMinMaxUV(const aiScene *scene, const aiNode* node, float2 &minUV,
        float2 &maxUV, uint32_t uvIndex) {
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh->HasTextureCoords(uvIndex)) {
            continue;
        }
        const float3* uv = reinterpret_cast<const float3*>(mesh->mTextureCoords[uvIndex]);
        const size_t numVertices = mesh->mNumVertices;
        const size_t numFaces = mesh->mNumFaces;
        if (numVertices == 0 || numFaces == 0) {
            continue;
        }
        if (uv) {
            for (size_t j = 0; j < numVertices; j++) {
                minUV = min(uv[j].xy, minUV);
                maxUV = max(uv[j].xy, maxUV);
            }
        }
    }
    for (size_t i = 0 ; i < node->mNumChildren ; ++i) {
        getMinMaxUV(scene, node->mChildren[i], minUV, maxUV, uvIndex);
    }
}

template<bool SNORMUVS>
static ushort2 convertUV(float2 uv) {
    if (SNORMUVS) {
        short2 uvshort(packSnorm16(uv));
        return bit_cast<ushort2>(uvshort);
    } else {
        half2 uvhalf(uv);
        return bit_cast<ushort2>(uvhalf);
    }
}

MeshAssimp::MeshAssimp(Engine& engine) : mEngine(engine) {
    mDefaultMap = createOneByOneTexture(0xffffffff);
    mDefaultNormalMap = createOneByOneTexture(0xffff8080);

    mDefaultColorMaterial = Material::Builder()
            .package(FILAMENTAPP_AIDEFAULTMAT_DATA, FILAMENTAPP_AIDEFAULTMAT_SIZE)
            .build(mEngine);

    mDefaultColorMaterial->setDefaultParameter("baseColor",   RgbType::LINEAR, float3{0.8});
    mDefaultColorMaterial->setDefaultParameter("metallic",    0.0f);
    mDefaultColorMaterial->setDefaultParameter("roughness",   0.4f);
    mDefaultColorMaterial->setDefaultParameter("reflectance", 0.5f);

    mDefaultTransparentColorMaterial = Material::Builder()
            .package(FILAMENTAPP_AIDEFAULTTRANS_DATA, FILAMENTAPP_AIDEFAULTTRANS_SIZE)
            .build(mEngine);

    mDefaultTransparentColorMaterial->setDefaultParameter("baseColor", RgbType::LINEAR, float3{0.8});
    mDefaultTransparentColorMaterial->setDefaultParameter("metallic",  0.0f);
    mDefaultTransparentColorMaterial->setDefaultParameter("roughness", 0.4f);
}

MeshAssimp::~MeshAssimp() {
    mEngine.destroy(mVertexBuffer);
    mEngine.destroy(mIndexBuffer);
    mEngine.destroy(mDefaultColorMaterial);
    mEngine.destroy(mDefaultTransparentColorMaterial);
    mEngine.destroy(mDefaultNormalMap);
    mEngine.destroy(mDefaultMap);

    for (auto& item : mGltfMaterialCache) {
        auto material = item.second;
        mEngine.destroy(material);
    }

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
    explicit State(std::vector<T>&& state) : state(std::move(state)) { }
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

            Texture::InternalFormat inputFormat;
            if (sRGB) {
                inputFormat = hasAlpha ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::SRGB8;
            } else {
                inputFormat = hasAlpha ? Texture::InternalFormat::RGBA8 : Texture::InternalFormat::RGB8;
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
                        (Texture::PixelBufferDescriptor::Callback) &stbi_image_free);

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

    Texture::InternalFormat inputFormat;
    if (sRGB) {
        inputFormat = hasAlpha ? Texture::InternalFormat::SRGB8_A8 : Texture::InternalFormat::SRGB8;
    } else {
        inputFormat = hasAlpha ? Texture::InternalFormat::RGBA8 : Texture::InternalFormat::RGB8;
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
            (Texture::PixelBufferDescriptor::Callback) &free);

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
        return std::atoi(pathStr + 1); // NOLINT
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

TextureSampler::MinFilter aiMinFilterToFilament(unsigned int aiMinFilter) {
    switch(aiMinFilter) {
        case GL_NEAREST: return TextureSampler::MinFilter::NEAREST;
        case GL_LINEAR: return TextureSampler::MinFilter::LINEAR;
        case GL_NEAREST_MIPMAP_NEAREST: return TextureSampler::MinFilter::NEAREST_MIPMAP_NEAREST;
        case GL_LINEAR_MIPMAP_NEAREST: return TextureSampler::MinFilter::LINEAR_MIPMAP_NEAREST;
        case GL_NEAREST_MIPMAP_LINEAR: return TextureSampler::MinFilter::NEAREST_MIPMAP_LINEAR;
        case GL_LINEAR_MIPMAP_LINEAR: return TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
        default: return TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR;
    }
}

TextureSampler::MagFilter aiMagFilterToFilament(unsigned int aiMagFilter) {
    switch(aiMagFilter) {
        case GL_NEAREST: return TextureSampler::MagFilter::NEAREST;
        default: return TextureSampler::MagFilter::LINEAR;
    }
}

// TODO: Change this to a member function (requires some alteration of cmakelsts.txt)
void setTextureFromPath(const aiScene *scene, Engine *engine,
        std::vector<filament::Texture*> textures, const aiString &textureFile,
        const std::string &materialName, const std::string &textureDirectory,
        aiTextureMapMode *mapMode, const char *parameterName,
        std::map<std::string, MaterialInstance *> &outMaterials,
        unsigned int aiMinFilterType=0, unsigned int aiMagFilterType=0) {

    TextureSampler::MinFilter minFilterType = aiMinFilterToFilament(aiMinFilterType);
    TextureSampler::MagFilter magFilterType = aiMagFilterToFilament(aiMagFilterType);

    TextureSampler sampler;
    if (mapMode) {
        sampler = TextureSampler(
                minFilterType,
                magFilterType,
                aiToFilamentMapMode(mapMode[0]),
                aiToFilamentMapMode(mapMode[1]),
                aiToFilamentMapMode(mapMode[2]));
    } else {
        sampler = TextureSampler(
                minFilterType,
                magFilterType,
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
    filament::math::float3 bmin(std::numeric_limits<float>::max());
    filament::math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const*>(
                (char const*) vertices + indices[i] * stride);
        const filament::math::float3 v(p->x, p->y, p->z);
        float3 tv = (transform * float4(v, 1.0f)).xyz;
        bmin = min(bmin, tv);
        bmax = max(bmax, tv);
    }
    return Box().set(bmin, bmax);
}

void MeshAssimp::addFromFile(const Path& path,
        std::map<std::string, MaterialInstance*>& materials, bool overrideMaterial) {

    Asset asset;
    asset.file = path;

    { // This scope to make sure we're not using std::move()'d objects later

        // TODO: if we had a way to allocate temporary buffers from the engine with a
        // "command buffer" lifetime, we wouldn't need to have to deal with freeing the
        // std::vectors here.

        //TODO: a lot of these method arguments should probably be class or global variables
        if (!setFromFile(asset, materials)) {
            return;
        }

        VertexBuffer::Builder vertexBufferBuilder = VertexBuffer::Builder()
                .vertexCount((uint32_t)asset.positions.size())
                .bufferCount(4)
                .attribute(VertexAttribute::POSITION,     0, VertexBuffer::AttributeType::HALF4)
                .attribute(VertexAttribute::TANGENTS,     1, VertexBuffer::AttributeType::SHORT4)
                .normalized(VertexAttribute::TANGENTS);

        if (asset.snormUV0) {
            vertexBufferBuilder.attribute(VertexAttribute::UV0, 2, VertexBuffer::AttributeType::SHORT2)
                .normalized(VertexAttribute::UV0);
        } else {
            vertexBufferBuilder.attribute(VertexAttribute::UV0, 2, VertexBuffer::AttributeType::HALF2);
        }

        if (asset.snormUV1) {
            vertexBufferBuilder.attribute(VertexAttribute::UV1, 3, VertexBuffer::AttributeType::SHORT2)
                    .normalized(VertexAttribute::UV1);
        } else {
            vertexBufferBuilder.attribute(VertexAttribute::UV1, 3, VertexBuffer::AttributeType::HALF2);
        }

        mVertexBuffer = vertexBufferBuilder.build(mEngine);

        auto ps = new State<half4>(std::move(asset.positions));
        auto ns = new State<short4>(std::move(asset.tangents));
        auto t0s = new State<ushort2>(std::move(asset.texCoords0));
        auto t1s = new State<ushort2>(std::move(asset.texCoords1));
        auto is = new State<uint32_t>(std::move(asset.indices));

        mVertexBuffer->setBufferAt(mEngine, 0,
                VertexBuffer::BufferDescriptor(ps->data(), ps->size(), State<half4>::free, ps));

        mVertexBuffer->setBufferAt(mEngine, 1,
                VertexBuffer::BufferDescriptor(ns->data(), ns->size(), State<short4>::free, ns));

        mVertexBuffer->setBufferAt(mEngine, 2,
                VertexBuffer::BufferDescriptor(t0s->data(), t0s->size(), State<ushort2>::free, t0s));

        mVertexBuffer->setBufferAt(mEngine, 3,
                VertexBuffer::BufferDescriptor(t1s->data(), t1s->size(), State<ushort2>::free, t1s));

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
    mRenderables.resize(startIndex + asset.meshes.size());
    EntityManager::get().create(asset.meshes.size(), mRenderables.data() + startIndex);
    EntityManager::get().create(1, &rootEntity);

    TransformManager& tcm = mEngine.getTransformManager();
    //Add root instance
    tcm.create(rootEntity, TransformManager::Instance{}, mat4f());

    for (auto& mesh : asset.meshes) {
        RenderableManager::Builder builder(mesh.parts.size());
        builder.boundingBox(mesh.aabb);
        builder.screenSpaceContactShadows(true);

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

        const size_t meshIndex = &mesh - asset.meshes.data();
        Entity entity = mRenderables[startIndex + meshIndex];
        if (!mesh.parts.empty()) {
            builder.build(mEngine, entity);
        }
        auto pindex = asset.parents[meshIndex];
        TransformManager::Instance parent((pindex < 0) ?
                tcm.getInstance(rootEntity) : tcm.getInstance(mRenderables[pindex]));
        tcm.create(entity, parent, mesh.transform);
    }
}

using Assimp::Importer;

bool MeshAssimp::setFromFile(Asset& asset, std::map<std::string, MaterialInstance*>& outMaterials) {
    Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_LINE | aiPrimitiveType_POINT);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, true);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_KEEP_HIERARCHY, true);

    aiScene const* scene = importer.ReadFile(asset.file,
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

    size_t index = importer.GetImporterIndex(asset.file.getExtension().c_str());
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

    if (scene) {
        size_t deep = 0;
        size_t depth = 0;
        size_t matCount = 0;

        aiNode const* node = scene->mRootNode;

        size_t totalVertexCount = 0;
        size_t totalIndexCount = 0;

        countVertices(node, totalVertexCount, totalIndexCount);

        asset.positions.reserve(asset.positions.size() + totalVertexCount);
        asset.tangents.reserve(asset.tangents.size() + totalVertexCount);
        asset.texCoords0.reserve(asset.texCoords0.size() + totalVertexCount);
        asset.texCoords1.reserve(asset.texCoords1.size() + totalVertexCount);
        asset.indices.reserve(asset.indices.size() + totalIndexCount);

        float2 minUV0 = float2(std::numeric_limits<float>::max());
        float2 maxUV0 = float2(std::numeric_limits<float>::lowest());
        getMinMaxUV(scene, node, minUV0, maxUV0, 0);
        float2 minUV1 = float2(std::numeric_limits<float>::max());
        float2 maxUV1 = float2(std::numeric_limits<float>::lowest());
        getMinMaxUV(scene, node, minUV1, maxUV1, 1);

        asset.snormUV0 = minUV0.x >= -1.0f && minUV0.x <= 1.0f && maxUV0.x >= -1.0f && maxUV0.x <= 1.0f &&
                         minUV0.y >= -1.0f && minUV0.y <= 1.0f && maxUV0.y >= -1.0f && maxUV0.y <= 1.0f;

        asset.snormUV1 = minUV1.x >= -1.0f && minUV1.x <= 1.0f && maxUV1.x >= -1.0f && maxUV1.x <= 1.0f &&
                         minUV1.y >= -1.0f && minUV1.y <= 1.0f && maxUV1.y >= -1.0f && maxUV1.y <= 1.0f;

        if (asset.snormUV0) {
            if (asset.snormUV1) {
                processNode<true, true>(asset, outMaterials,
                                        scene, isGLTF, deep, matCount, node, -1, depth);
            } else {
                processNode<true, false>(asset, outMaterials,
                                        scene, isGLTF, deep, matCount, node, -1, depth);
            }
        } else {
            if (asset.snormUV1) {
                processNode<false, true>(asset, outMaterials,
                                        scene, isGLTF, deep, matCount, node, -1, depth);
            } else {
                processNode<false, false>(asset, outMaterials,
                                        scene, isGLTF, deep, matCount, node, -1, depth);
            }
        }

        // compute the aabb and find bounding box of entire model
        for (auto& mesh : asset.meshes) {
            mesh.aabb = RenderableManager::computeAABB(
                    asset.positions.data(),
                    asset.indices.data() + mesh.offset,
                    mesh.count);

            Box transformedAabb = computeTransformedAABB(
                    asset.positions.data(),
                    asset.indices.data() + mesh.offset,
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

template<bool SNORMUV0, bool SNORMUV1>
void MeshAssimp::processNode(Asset& asset,
        std::map<std::string,
        MaterialInstance *> &outMaterials,
        const aiScene *scene,
        bool isGLTF,
        size_t deep,
        size_t matCount,
        const aiNode *node,
        int parentIndex,
        size_t &depth) const {
    mat4f const& current = transpose(*reinterpret_cast<mat4f const*>(&node->mTransformation));

    size_t totalIndices = 0;
    asset.parents.push_back(parentIndex);
    asset.meshes.push_back(Mesh{});
    asset.meshes.back().offset = asset.indices.size();
    asset.meshes.back().transform = current;

    mat4f parentTransform = parentIndex >= 0 ? asset.meshes[parentIndex].accTransform : mat4f();
    asset.meshes.back().accTransform = parentTransform * current;

    for (size_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];

            float3 const* positions  = reinterpret_cast<float3 const*>(mesh->mVertices);
            float3 const* tangents   = reinterpret_cast<float3 const*>(mesh->mTangents);
            float3 const* bitangents = reinterpret_cast<float3 const*>(mesh->mBitangents);
            float3 const* normals    = reinterpret_cast<float3 const*>(mesh->mNormals);
            float3 const* texCoords0 = reinterpret_cast<float3 const*>(mesh->mTextureCoords[0]);
            float3 const* texCoords1 = reinterpret_cast<float3 const*>(mesh->mTextureCoords[1]);

            const size_t numVertices = mesh->mNumVertices;

            if (numVertices > 0) {
                const aiFace* faces = mesh->mFaces;
                const size_t numFaces = mesh->mNumFaces;

                if (numFaces > 0) {
                    size_t indicesOffset = asset.positions.size();

                    for (size_t j = 0; j < numVertices; j++) {
                        float3 normal = normals[j];
                        float3 tangent;
                        float3 bitangent;

                        // Assimp always returns 3D tex coords but we only support 2D tex coords.
                        float2 texCoord0 = texCoords0 ? texCoords0[j].xy : float2{0.0};
                        float2 texCoord1 = texCoords1 ? texCoords1[j].xy : float2{0.0};
                        // If the tangent and bitangent don't exist, make arbitrary ones. This only
                        // occurs when the mesh is missing texture coordinates, because assimp
                        // computes tangents for us. (search up for aiProcess_CalcTangentSpace)
                        if (!tangents) {
                            bitangent = normalize(cross(normal, float3{1.0, 0.0, 0.0}));
                            tangent = normalize(cross(normal, bitangent));
                        } else {
                            tangent = tangents[j];
                            bitangent = bitangents[j];
                        }

                        quatf q = filament::math::details::TMat33<float>::packTangentFrame({tangent, bitangent, normal});
                        asset.tangents.push_back(packSnorm16(q.xyzw));
                        asset.texCoords0.emplace_back(convertUV<SNORMUV0>(texCoord0));
                        asset.texCoords1.emplace_back(convertUV<SNORMUV1>(texCoord1));

                        asset.positions.emplace_back(positions[j], 1.0_h);
                    }

                    // Populate the index buffer. All faces are triangles at this point because we
                    // asked assimp to perform triangulation.
                    size_t indicesCount = numFaces * faces[0].mNumIndices;
                    size_t indexBufferOffset = asset.indices.size();
                    totalIndices += indicesCount;

                    for (size_t j = 0; j < numFaces; ++j) {
                        const aiFace& face = faces[j];
                        for (size_t k = 0; k < face.mNumIndices; ++k) {
                            asset.indices.push_back(uint32_t(face.mIndices[k] + indicesOffset));
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
                        std::string dirName = asset.file.getParent();
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
                    float roughness = sqrt(2.0f / (shininess + 2.0f));

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

                    asset.meshes.back().parts.push_back({
                            indexBufferOffset, indicesCount, materialName,
                            baseColor, opacity, metallic, roughness, reflectance
                    });
                }
            }
        }

    if (node->mNumMeshes > 0) {
        asset.meshes.back().count = totalIndices;
    }

    if (node->mNumChildren) {
        parentIndex = static_cast<int>(asset.meshes.size()) - 1;
        deep++;
        depth = std::max(deep, depth);
        for (size_t i = 0, c = node->mNumChildren; i < c; i++) {
            processNode<SNORMUV0, SNORMUV1>(asset, outMaterials, scene,
                                            isGLTF, deep, matCount, node->mChildren[i], parentIndex, depth);
        }
        deep--;
    }
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
    MaterialConfig matConfig;

    material->Get(AI_MATKEY_TWOSIDED, matConfig.doubleSided);
    material->Get(AI_MATKEY_GLTF_UNLIT, matConfig.unlit);

    aiString alphaMode;
    material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    if (strcmp(alphaMode.C_Str(), "BLEND") == 0) {
        matConfig.alphaMode = AlphaMode::TRANSPARENT;
    } else if (strcmp(alphaMode.C_Str(), "MASK") == 0) {
        matConfig.alphaMode = AlphaMode::MASKED;
        float maskThreshold = 0.5;
        material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, maskThreshold);
        matConfig.maskThreshold = maskThreshold;
    }

    material->Get(_AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE,
                  matConfig.baseColorUV);
    material->Get(_AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE,
                  matConfig.metallicRoughnessUV);
    material->Get(_AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, aiTextureType_LIGHTMAP, 0, matConfig.aoUV);
    material->Get(_AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, aiTextureType_NORMALS, 0, matConfig.normalUV);
    material->Get(_AI_MATKEY_GLTF_TEXTURE_TEXCOORD_BASE, aiTextureType_EMISSIVE, 0, matConfig.emissiveUV);

    uint64_t configHash = hashMaterialConfig(matConfig);

    if (mGltfMaterialCache.find(configHash) == mGltfMaterialCache.end()) {
        mGltfMaterialCache[configHash] = createMaterialFromConfig(mEngine, matConfig);
    }

    outMaterials[materialName] = mGltfMaterialCache[configHash]->createInstance();

    // TODO: is there a way to use the same material for multiple mask threshold values?
//    if (matConfig.alphaMode == masked) {
//        float maskThreshold = 0.5;
//        material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, maskThreshold);
//        outMaterials[materialName]->setParameter("maskThreshold", maskThreshold);
//    }

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

    if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &baseColorPath,
            nullptr, nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        unsigned int minType = 0;
        unsigned int magType = 0;
        material->Get("$tex.mappingfiltermin", AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, minType);
        material->Get("$tex.mappingfiltermag", AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, magType);

        setTextureFromPath(scene, &mEngine, mTextures, baseColorPath,
                materialName, dirName, mapMode, "baseColorMap", outMaterials, minType, magType);
    } else {
        outMaterials[materialName]->setParameter("baseColorMap", mDefaultMap, sampler);
    }

    if (material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &MRPath,
            nullptr, nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        unsigned int minType = 0;
        unsigned int magType = 0;
        material->Get("$tex.mappingfiltermin", AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, minType);
        material->Get("$tex.mappingfiltermag", AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, magType);

        setTextureFromPath(scene, &mEngine, mTextures, MRPath, materialName,
                dirName, mapMode, "metallicRoughnessMap", outMaterials, minType, magType);
    } else {
        outMaterials[materialName]->setParameter("metallicRoughnessMap", mDefaultMap, sampler);
        outMaterials[materialName]->setParameter("metallicFactor", mDefaultMetallic);
        outMaterials[materialName]->setParameter("roughnessFactor", mDefaultRoughness);
    }

    if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &AOPath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        unsigned int minType = 0;
        unsigned int magType = 0;
        material->Get("$tex.mappingfiltermin", aiTextureType_LIGHTMAP, 0, minType);
        material->Get("$tex.mappingfiltermag", aiTextureType_LIGHTMAP, 0, magType);
        setTextureFromPath(scene, &mEngine, mTextures, AOPath, materialName,
                dirName, mapMode, "aoMap", outMaterials, minType, magType);
    } else {
        outMaterials[materialName]->setParameter("aoMap", mDefaultMap, sampler);
    }

    if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        unsigned int minType = 0;
        unsigned int magType = 0;
        material->Get("$tex.mappingfiltermin", aiTextureType_NORMALS, 0, minType);
        material->Get("$tex.mappingfiltermag", aiTextureType_NORMALS, 0, magType);
        setTextureFromPath(scene, &mEngine, mTextures, normalPath, materialName,
                dirName, mapMode, "normalMap", outMaterials, minType, magType);
    } else {
        outMaterials[materialName]->setParameter("normalMap", mDefaultNormalMap, sampler);
    }

    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissivePath, nullptr,
            nullptr, nullptr, nullptr, mapMode) == AI_SUCCESS) {
        unsigned int minType = 0;
        unsigned int magType = 0;
        material->Get("$tex.mappingfiltermin", aiTextureType_EMISSIVE, 0, minType);
        material->Get("$tex.mappingfiltermag", aiTextureType_EMISSIVE, 0, magType);
        setTextureFromPath(scene, &mEngine, mTextures, emissivePath, materialName,
                dirName, mapMode, "emissiveMap", outMaterials, minType, magType);
    }  else {
        outMaterials[materialName]->setParameter("emissiveMap", mDefaultMap, sampler);
        outMaterials[materialName]->setParameter("emissiveFactor", mDefaultEmissive);
    }

    //If the gltf has texture factors, override the default factor values
    if (material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS) {
        outMaterials[materialName]->setParameter("metallicFactor", metallicFactor);
    }

    if (material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS) {
        outMaterials[materialName]->setParameter("roughnessFactor", roughnessFactor);
    }

    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor) == AI_SUCCESS) {
        sRGBColor emissiveFactorCast = *reinterpret_cast<sRGBColor*>(&emissiveFactor);
        outMaterials[materialName]->setParameter("emissiveFactor", emissiveFactorCast);
    }

    if (material->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, baseColorFactor) == AI_SUCCESS) {
        sRGBColorA baseColorFactorCast = *reinterpret_cast<sRGBColorA*>(&baseColorFactor);
        outMaterials[materialName]->setParameter("baseColorFactor", baseColorFactorCast);
    }

    aiBool isSpecularGlossiness = false;
    if (material->Get(AI_MATKEY_GLTF_PBRSPECULARGLOSSINESS, isSpecularGlossiness) == AI_SUCCESS) {
        if (isSpecularGlossiness) {
            std::cout << "Warning: pbrSpecularGlossiness textures are not currently supported" << std::endl;
        }
    }
}
