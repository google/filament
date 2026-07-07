/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "materials/uberarchive.h"

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/FilamentInstance.h>
#include <gltfio/math.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filament/Engine.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <math/mathfwd.h>

#include <gtest/gtest.h>
#include <meshoptimizer.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <unistd.h>

using namespace filament;
using namespace backend;
using namespace gltfio;
using namespace utils;

char const* ANIMATED_MORPH_CUBE_GLB = "AnimatedMorphCube.glb";
char const* DAMAGED_HELMET_WEBP_GLB = "DamagedHelmetWebp.glb";

static constexpr uint8_t VALID_1X1_PNG[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0x15, 0xc4, 0x89, 0x00, 0x00, 0x00,
    0x0d, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x63, 0xf8, 0xcf, 0xc0, 0xf0,
    0x1f, 0x00, 0x05, 0x00, 0x01, 0xff, 0x89, 0x99, 0x3d, 0x1d, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

namespace {

static void appendU32LE(std::vector<uint8_t>& dst, uint32_t value) {
    dst.push_back(uint8_t(value & 0xffu));
    dst.push_back(uint8_t((value >> 8u) & 0xffu));
    dst.push_back(uint8_t((value >> 16u) & 0xffu));
    dst.push_back(uint8_t((value >> 24u) & 0xffu));
}

static std::vector<uint8_t> makeMeshoptPayload(size_t vertexCount, size_t stride) {
    std::vector<uint8_t> vertices(vertexCount * stride, 0);
    std::vector<uint8_t> encoded(meshopt_encodeVertexBufferBound(vertexCount, stride));
    const size_t encodedSize = meshopt_encodeVertexBuffer(encoded.data(), encoded.size(),
            vertices.data(), vertexCount, stride);
    EXPECT_GT(encodedSize, 0u);
    encoded.resize(encodedSize);
    return encoded;
}

static std::string makeMeshoptGlbJson(size_t meshoptCount, size_t meshoptEncodedSize,
        size_t bufferByteLength, size_t stride) {
    const size_t decodedSize = meshoptCount * stride;
    return std::string(R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["EXT_meshopt_compression"],
  "buffers": [
    { "byteLength": )") + std::to_string(bufferByteLength) + R"( }
  ],
  "bufferViews": [
    {
      "buffer": 0,
      "byteOffset": 0,
      "byteLength": )" + std::to_string(decodedSize) + R"(,
      "extensions": {
        "EXT_meshopt_compression": {
          "buffer": 0,
          "byteOffset": 0,
          "byteLength": )" + std::to_string(meshoptEncodedSize) + R"(,
          "byteStride": )" + std::to_string(stride) + R"(,
          "count": )" + std::to_string(meshoptCount) + R"(,
          "mode": "ATTRIBUTES",
          "filter": "NONE"
        }
      }
    }
  ],
  "nodes": [],
  "scenes": [
    { "nodes": [] }
  ],
  "scene": 0
})";
}

static std::vector<uint8_t> makeMeshoptGlb(size_t meshoptCount, size_t stride) {
    static constexpr size_t kEncodedVertexCount = 256;
    const std::vector<uint8_t> meshopt = makeMeshoptPayload(kEncodedVertexCount, stride);

    std::string json = makeMeshoptGlbJson(meshoptCount, meshopt.size(), meshopt.size(), stride);
    while ((json.size() % 4u) != 0u) {
        json.push_back(' ');
    }

    const uint32_t jsonSize = uint32_t(json.size());
    const uint32_t binSize = uint32_t(meshopt.size());
    const uint32_t totalSize = 12u + 8u + jsonSize + 8u + binSize;

    std::vector<uint8_t> glb;
    glb.reserve(totalSize);
    appendU32LE(glb, 0x46546c67u);
    appendU32LE(glb, 2u);
    appendU32LE(glb, totalSize);
    appendU32LE(glb, jsonSize);
    appendU32LE(glb, 0x4e4f534au);
    glb.insert(glb.end(), json.begin(), json.end());
    appendU32LE(glb, binSize);
    appendU32LE(glb, 0x004e4942u);
    glb.insert(glb.end(), meshopt.begin(), meshopt.end());
    return glb;
}

} // namespace

class glTFData {
public:
    glTFData(Path filename, Engine* engine, MaterialProvider* materialProvider,
            NameComponentManager* nameManager)
        : mAssetLoader(AssetLoader::create({engine, materialProvider, nameManager})),
          mResourceLoader(new ResourceLoader({
                  engine, filename.getAbsolutePath().c_str(), false, /* normalizeSkinningWeights */
          })),
          mStbDecoder(createStbProvider(engine)), mKtxDecoder(createKtx2Provider(engine)),
          mWebpDecoder(createWebpProvider(engine)) {
        mResourceLoader->addTextureProvider("image/png", mStbDecoder);
        mResourceLoader->addTextureProvider("image/ktx2", mKtxDecoder);
        if (mWebpDecoder) {
            mResourceLoader->addTextureProvider("image/webp", mWebpDecoder);
        }

        long contentSize = static_cast<long>(getFileSize(filename.c_str()));
        if (contentSize <= 0) {
            std::cerr << "Unable to open " << filename.c_str() << std::endl;
            exit(1);
        }

        // Consume the glTF file.
        std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
        std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
        if (!in.read((char*) buffer.data(), contentSize)) {
            std::cerr << "Unable to read " << filename.c_str() << std::endl;
            exit(1);
        }

        // Parse the glTF file and create Filament entities.
        mAsset = mAssetLoader->createAsset(buffer.data(), buffer.size());
        buffer.clear();
        buffer.shrink_to_fit();

        if (!mAsset) {
            std::cerr << "Unable to parse " << filename.c_str() << std::endl;
            exit(1);
        }

        // Load resources
        if (!mResourceLoader->asyncBeginLoad(mAsset)) {
            std::cerr << "Unable to start loading resources for " << filename << std::endl;
            exit(1);
        }
        mAsset->releaseSourceData();
    }

    ~glTFData() {
        mAssetLoader->destroyAsset(mAsset);
        delete mResourceLoader;
        delete mStbDecoder;
        delete mKtxDecoder;
        delete mWebpDecoder;

        AssetLoader::destroy(&mAssetLoader);
    }

    FilamentAsset* getAsset() const { return mAsset; }

    AssetLoader* mAssetLoader;
    ResourceLoader* mResourceLoader = nullptr;
    TextureProvider* mStbDecoder = nullptr;
    TextureProvider* mKtxDecoder = nullptr;
    TextureProvider* mWebpDecoder = nullptr;
    FilamentAsset* mAsset = nullptr;
};

class glTFIOTest : public testing::Test {
protected:
    Engine* mEngine = nullptr;
    NameComponentManager* mNameManager = nullptr;
    MaterialProvider* mMaterialProvider = nullptr;

    //    std::unique_ptr<glTFData> mData;
    std::unordered_map<char const*, std::unique_ptr<glTFData>> mData;

    void SetUp() override {
        mEngine = Engine::Builder().backend(Backend::NOOP).build();

        mNameManager = new NameComponentManager(EntityManager::get());
        mMaterialProvider = createUbershaderProvider(mEngine, UBERARCHIVE_DEFAULT_DATA,
                UBERARCHIVE_DEFAULT_SIZE);

        for (auto fname: {ANIMATED_MORPH_CUBE_GLB, DAMAGED_HELMET_WEBP_GLB}) {
            Path gltfFile = Path::getCurrentExecutable().getParent() + Path(fname);
            mData[fname] =
                    std::make_unique<glTFData>(gltfFile, mEngine, mMaterialProvider, mNameManager);
        }
    }

    void TearDown() override {
        mData.clear();
        mMaterialProvider->destroyMaterials();
        Engine::destroy(&mEngine);

        delete mMaterialProvider;
        delete mNameManager;
    }
};

TEST_F(glTFIOTest, AnimatedMorphCubeMaterials) {
    FilamentAsset const& morphCubeAsset = *mData[ANIMATED_MORPH_CUBE_GLB]->getAsset();
    Entity const* renderables = morphCubeAsset.getRenderableEntities();
    auto& renderableManager = mEngine->getRenderableManager();

    auto inst = renderableManager.getInstance(renderables[0]);
    auto materialInst = renderableManager.getMaterialInstanceAt(inst, 0);
    std::string_view name{materialInst->getName()};

    EXPECT_EQ(name, "Material");
}

TEST_F(glTFIOTest, StbProviderCancelFreesDecodedTextureWithStbAllocator) {
    TextureProvider* provider = createStbProvider(mEngine);
    Texture* texture = provider->pushTexture(VALID_1X1_PNG, sizeof(VALID_1X1_PNG), "image/png",
            TextureProvider::TextureFlags::NONE);

    ASSERT_NE(texture, nullptr) << provider->getPushMessage();

    provider->waitForCompletion();
    provider->cancelDecoding();

    mEngine->destroy(texture);
    delete provider;
}

// A macro to help with mat comparisons within a range.
#define EXPECT_MAT_NEAR(MAT1, MAT2, eps)                        \
do {                                                            \
    const decltype(MAT1) v1 = MAT1;                             \
    const decltype(MAT2) v2 = MAT2;                             \
    EXPECT_EQ(v1.NUM_ROWS, v2.NUM_ROWS);                        \
    EXPECT_EQ(v1.NUM_COLS, v2.NUM_COLS);                        \
    for (int i = 0; i < v1.NUM_ROWS; ++i) {                     \
        for (int j = 0; j < v1.NUM_COLS; ++j)                   \
            EXPECT_NEAR(v1[i][j], v2[i][j], eps) <<             \
                "v[" << i << "][" << j << "]";                  \
    }                                                           \
} while(0)


TEST_F(glTFIOTest, AnimatedMorphCubeTransforms) {
    FilamentAsset const& morphCubeAsset = *mData[ANIMATED_MORPH_CUBE_GLB]->getAsset();
    auto const& transformManager = mEngine->getTransformManager();
    Entity const* renderables = morphCubeAsset.getRenderableEntities();

    EXPECT_EQ(morphCubeAsset.getRenderableEntityCount(), 1u);

    EXPECT_TRUE(transformManager.hasComponent(renderables[0]));

    auto const inst = transformManager.getInstance(renderables[0]);
    math::mat4f const transform = transformManager.getTransform(inst);
    math::mat4f const expectedTransform = composeMatrix(math::float3{0.0, 0.0, 0.0},
            math::quatf{0.0, 0.0, 0.7071067, -0.7071068}, math::float3{100.0, 100.0, 100.0});

    auto const result = inverse(transform) * expectedTransform;

    float const value_eps = float(0.00001) * std::numeric_limits<float>::epsilon();

    // We expect the result to be identity
    EXPECT_MAT_NEAR(result, math::mat4f{}, value_eps);
}

TEST_F(glTFIOTest, AnimatedMorphCubeRenderables) {
    FilamentAsset const& morphCubeAsset = *mData[ANIMATED_MORPH_CUBE_GLB]->getAsset();
    Entity const* renderables = morphCubeAsset.getRenderableEntities();
    auto const& renderableManager = mEngine->getRenderableManager();

    EXPECT_EQ(morphCubeAsset.getRenderableEntityCount(), 1u);

    EXPECT_TRUE(renderableManager.hasComponent(renderables[0]));
    auto const inst = renderableManager.getInstance(renderables[0]);
    EXPECT_EQ(renderableManager.getPrimitiveCount(inst), 1u);
    AttributeBitset const attribs = renderableManager.getEnabledAttributesAt(inst, 0);

    EXPECT_TRUE(attribs[VertexAttribute::POSITION]);
    EXPECT_TRUE(attribs[VertexAttribute::TANGENTS]);
    if (mMaterialProvider->needsDummyData(VertexAttribute::COLOR)) {
        EXPECT_TRUE(attribs[VertexAttribute::COLOR]);
    } else {
        EXPECT_FALSE(attribs[VertexAttribute::COLOR]);
    }
    if (mMaterialProvider->needsDummyData(VertexAttribute::UV0)) {
        EXPECT_TRUE(attribs[VertexAttribute::UV0]);
    } else {
        EXPECT_FALSE(attribs[VertexAttribute::UV0]);
    }
    if (mMaterialProvider->needsDummyData(VertexAttribute::UV1)) {
        EXPECT_TRUE(attribs[VertexAttribute::UV1]);
    } else {
        EXPECT_FALSE(attribs[VertexAttribute::UV1]);
    }

    // The AnimatedMorphCube has two morph targets: "thin" and "angle"
    EXPECT_EQ(renderableManager.getMorphTargetCount(inst), 2u);

    // The 0-th MorphTargetBuffer holds both of the targets
    auto const morphTargetBuffer = renderableManager.getMorphTargetBuffer(inst);
    EXPECT_EQ(morphTargetBuffer->getCount(), 2u);

    // The number of vertices for the morph target should be the face vertices in a cube =>
    // (6 faces * 4 vertices per face) = 24 vertices
    EXPECT_EQ(morphTargetBuffer->getVertexCount(), 24u);
}

TEST_F(glTFIOTest, DamagedHelmetWebpMaterials) {
    FilamentAsset const& damagedHelmetAsset = *mData[DAMAGED_HELMET_WEBP_GLB]->getAsset();
    Entity const* renderables = damagedHelmetAsset.getRenderableEntities();
    auto& renderableManager = mEngine->getRenderableManager();

    auto inst = renderableManager.getInstance(renderables[0]);
    auto materialInst = renderableManager.getMaterialInstanceAt(inst, 0);
    std::string_view name{materialInst->getName()};
    EXPECT_EQ(name, "Material_MR");
#if defined(FILAMENT_SUPPORTS_WEBP_TEXTURES)
    EXPECT_TRUE(isWebpSupported());
    EXPECT_FALSE(mData[DAMAGED_HELMET_WEBP_GLB]->mWebpDecoder == nullptr);
    EXPECT_EQ(mEngine->getTextureCount(), 8);
#else
    EXPECT_FALSE(isWebpSupported());
    EXPECT_TRUE(mData[DAMAGED_HELMET_WEBP_GLB]->mWebpDecoder == nullptr);
    EXPECT_EQ(mEngine->getTextureCount(), 3);    
#endif
}

TEST_F(glTFIOTest, MeshoptAllocationFailureRejectsGracefully) {
    static constexpr size_t kMeshoptStride = 4;
    const size_t maxCount = std::numeric_limits<size_t>::max() / kMeshoptStride;

    const std::vector<uint8_t> glb = makeMeshoptGlb(maxCount, kMeshoptStride);

    AssetLoader* assetLoader = AssetLoader::create({mEngine, mMaterialProvider, mNameManager});
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({mEngine, ".", false});
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// A mesh may carry morph-target names (mesh.extras.targetNames) whose count is parsed independently
// of its morph-target count. When the mesh has no primitives the morph-target count is zero, so the
// two counts can disagree. createRenderable() must size its name copy by the morph-target count and
// not by the (independent) name count; otherwise it writes past the names storage. This loads such
// a mesh and requires that it parses without an out-of-bounds access (validated under ASan) and
// retains no more morph-target names than morph targets.
TEST_F(glTFIOTest, MalformedMeshTargetNamesWithoutPrimitives) {
    static char const* const kGltf =
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)"
            R"("nodes":[{"mesh":0}],)"
            R"("meshes":[{"extras":{"targetNames":["t0","t1","t2","t3","t4","t5","t6","t7"]}}]})";

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    FilamentAsset* const asset = assetLoader->createAsset(
            reinterpret_cast<uint8_t const*>(kGltf), uint32_t(std::strlen(kGltf)));

    EXPECT_NE(asset, nullptr);
    if (asset != nullptr) {
        Entity const* renderables = asset->getRenderableEntities();
        for (size_t i = 0, n = asset->getRenderableEntityCount(); i < n; ++i) {
            EXPECT_EQ(asset->getMorphTargetCountAt(renderables[i]), 0u);
        }
        assetLoader->destroyAsset(asset);
    }
    AssetLoader::destroy(&assetLoader);
}

// createPrimitives() caps a primitive's morph-target count at MAX_MORPH_TARGETS and sizes its
// slotIndices vector to that cap; createRenderable() must iterate the morph-slot loop over the same
// bound, not the raw (uncapped) morph-target count, or it indexes slotIndices out of range. This
// loads a mesh with more morph targets than the cap and requires it parses without an out-of-bounds
// access (validated under ASan).
TEST_F(glTFIOTest, MorphTargetsExceedingMaxDoNotOverflow) {
    std::string targets;
    for (int i = 0; i < 300; ++i) {
        targets += (i == 0) ? "{\"POSITION\":1}" : ",{\"POSITION\":1}";
    }
    std::string const gltf =
            "{\"asset\":{\"version\":\"2.0\"},\"scene\":0,\"scenes\":[{\"nodes\":[0]}],"
            "\"nodes\":[{\"mesh\":0}],"
            "\"meshes\":[{\"primitives\":[{\"attributes\":{\"POSITION\":0},\"mode\":4,"
            "\"targets\":[" + targets + "]}]}],"
            "\"accessors\":["
            "{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
            "\"min\":[0,0,0],\"max\":[1,1,1]},"
            "{\"bufferView\":1,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\"}],"
            "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36},"
            "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":36}],"
            "\"buffers\":[{\"byteLength\":72}]}";

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    FilamentAsset* const asset = assetLoader->createAsset(
            reinterpret_cast<uint8_t const*>(gltf.data()), uint32_t(gltf.size()));

    EXPECT_NE(asset, nullptr);
    if (asset != nullptr) {
        assetLoader->destroyAsset(asset);
    }
    AssetLoader::destroy(&assetLoader);
}

namespace {

static std::vector<uint8_t> makeMalformedEightBitIndexGlb(uint32_t indexCount) {
    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "extensionsUsed": ["KHR_materials_unlit"],
  "buffers": [
    { "byteLength": 13 }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": 12 },
    { "buffer": 0, "byteOffset": 12, "byteLength": 1 }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": 1,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [0, 0, 0]
    },
    {
      "bufferView": 1,
      "componentType": 5121,
      "count": )") +
            std::to_string(indexCount) +
            R"(,
      "type": "SCALAR"
    }
  ],
  "materials": [
    {
      "extensions": {
        "KHR_materials_unlit": {}
      }
    }
  ],
  "meshes": [
    {
      "primitives": [
        {
          "attributes": { "POSITION": 0 },
          "indices": 1,
          "material": 0
        }
      ]
    }
  ],
  "nodes": [
    { "mesh": 0 }
  ],
  "scenes": [
    { "nodes": [0] }
  ],
  "scene": 0
})";

    while ((json.size() % 4u) != 0u) {
        json.push_back(' ');
    }

    std::vector<uint8_t> bin(13, 0);
    while ((bin.size() % 4u) != 0u) {
        bin.push_back(0);
    }

    const uint32_t jsonSize = uint32_t(json.size());
    const uint32_t binSize = uint32_t(bin.size());
    const uint32_t totalSize = 12u + 8u + jsonSize + 8u + binSize;

    std::vector<uint8_t> glb;
    glb.reserve(totalSize);

    appendU32LE(glb, 0x46546c67u);
    appendU32LE(glb, 2u);
    appendU32LE(glb, totalSize);
    appendU32LE(glb, jsonSize);
    appendU32LE(glb, 0x4e4f534au);
    glb.insert(glb.end(), json.begin(), json.end());
    appendU32LE(glb, binSize);
    appendU32LE(glb, 0x004e4942u);
    glb.insert(glb.end(), bin.begin(), bin.end());

    return glb;
}

} // namespace

TEST_F(glTFIOTest, RejectsOversizedEightBitIndexAccessor) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    std::vector<uint8_t> glb = makeMalformedEightBitIndexGlb(100000000u);
    FilamentAsset* asset = loader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    loader->destroyAsset(asset);
    AssetLoader::destroy(&loader);
}

TEST_F(glTFIOTest, SkipsInverseBindMatricesOutsideBufferView) {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / ("gltfio_m7_" + std::to_string(getpid()));
    const fs::path attackerDir = root / "attacker";
    fs::create_directories(attackerDir);

    const fs::path secretPath = root / "secret.bin";
    {
        std::ofstream out(secretPath, std::ios::binary);
        ASSERT_TRUE(out.good());
        const float identity[16] = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f,
        };
        const float secret[16] = {
                1337.0f, 1338.0f, 1339.0f, 1340.0f,
                1341.0f, 1342.0f, 1343.0f, 1344.0f,
                1345.0f, 1346.0f, 1347.0f, 1348.0f,
                1349.0f, 1350.0f, 1351.0f, 1352.0f,
        };
        out.write(reinterpret_cast<const char*>(identity), sizeof(identity));
        out.write(reinterpret_cast<const char*>(secret), sizeof(secret));
    }

    const fs::path gltfPath = attackerDir / "evil.gltf";
    {
        std::ofstream out(gltfPath);
        ASSERT_TRUE(out.good());
        out << R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],)"
               R"("nodes":[{"name":"root","skin":0,"children":[1,2]},{"name":"joint0"},{"name":"joint1"}],)"
               R"("skins":[{"joints":[1,2],"inverseBindMatrices":0}],)"
               R"("buffers":[{"uri":"../secret.bin","byteLength":128}],)"
               R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":64}],)"
               R"("accessors":[{"bufferView":0,"byteOffset":0,"componentType":5126,"count":1,"type":"MAT4"}]})";
    }

    std::ifstream in(gltfPath, std::ifstream::binary | std::ifstream::ate);
    ASSERT_TRUE(in.good());
    const auto contentSize = in.tellg();
    in.seekg(0, std::ifstream::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(contentSize));
    ASSERT_TRUE(in.read(reinterpret_cast<char*>(buffer.data()), contentSize));

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ResourceLoader* resourceLoader = new ResourceLoader({
            mEngine, gltfPath.string().c_str(), false,
    });

    FilamentAsset* asset = assetLoader->createAsset(buffer.data(), buffer.size());
    ASSERT_NE(asset, nullptr);
    EXPECT_TRUE(resourceLoader->loadResources(asset));

    FilamentInstance* instance = asset->getInstance();
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(instance->getSkinCount(), 1u);
    EXPECT_EQ(instance->getJointCountAt(0), 2u);
    EXPECT_THROW((void) instance->getInverseBindMatricesAt(0), utils::PreconditionPanic);

    assetLoader->destroyAsset(asset);
    delete resourceLoader;
    AssetLoader::destroy(&assetLoader);

    fs::remove_all(root);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
