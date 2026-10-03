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

#include "Utility.h"

#include "extended/TangentsJobExtended.h"

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
#include <math/vec3.h>
#include <math/vec4.h>

#include <cgltf.h>
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
        size_t bufferByteLength, size_t stride, const char* mode = "ATTRIBUTES",
        const char* filter = "NONE") {
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
          "mode": ")" + mode + R"(",
          "filter": ")" + filter + R"("
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

static std::vector<uint8_t> makeMeshoptGlb(size_t meshoptCount, size_t stride,
        const char* mode = "ATTRIBUTES", const char* filter = "NONE") {
    static constexpr size_t kEncodedVertexCount = 256;
    std::vector<uint8_t> meshopt;
    if (stride > 0 && stride % 4 == 0) {
        meshopt = makeMeshoptPayload(kEncodedVertexCount, stride);
    } else {
        meshopt.assign(32, 0);
    }

    std::string json = makeMeshoptGlbJson(meshoptCount, meshopt.size(), meshopt.size(), stride,
            mode, filter);
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

static bool runDecodeMeshopt(cgltf_meshopt_compression_mode mode, size_t count, size_t stride,
        cgltf_meshopt_compression_filter filter = cgltf_meshopt_compression_filter_none,
        cgltf_buffer* bufferOverride = nullptr, size_t offset = 0, size_t size = 32,
        bool provideBuffer = true) {
    uint8_t dummyBuffer[32] = {0};
    cgltf_buffer buffer{};
    buffer.data = dummyBuffer;
    buffer.size = sizeof(dummyBuffer);

    cgltf_buffer_view bufferView{};
    bufferView.has_meshopt_compression = true;
    bufferView.meshopt_compression.buffer = provideBuffer
            ? (bufferOverride ? bufferOverride : &buffer)
            : nullptr;
    bufferView.meshopt_compression.offset = offset;
    bufferView.meshopt_compression.size = size;
    bufferView.meshopt_compression.mode = mode;
    bufferView.meshopt_compression.count = count;
    bufferView.meshopt_compression.stride = stride;
    bufferView.meshopt_compression.filter = filter;

    cgltf_data data{};
    data.buffer_views_count = 1;
    data.buffer_views = &bufferView;

    return utility::decodeMeshoptCompression(&data);
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

        mAssetLoader->gc();
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

        mNameManager->gc();
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

TEST_F(glTFIOTest, MeshoptRejectsInvalidTrianglesCount) {
    const std::vector<uint8_t> glb = makeMeshoptGlb(1, 4, "TRIANGLES", "NONE");

    AssetLoader* assetLoader = AssetLoader::create({mEngine, mMaterialProvider, mNameManager});
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({mEngine, ".", false});
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

TEST_F(glTFIOTest, MeshoptRejectsInvalidTrianglesStride) {
    const std::vector<uint8_t> glb = makeMeshoptGlb(3, 3, "TRIANGLES", "NONE");

    AssetLoader* assetLoader = AssetLoader::create({mEngine, mMaterialProvider, mNameManager});
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({mEngine, ".", false});
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

TEST_F(glTFIOTest, MeshoptRejectsInvalidIndicesStride) {
    const std::vector<uint8_t> glb = makeMeshoptGlb(1, 1, "INDICES", "NONE");

    AssetLoader* assetLoader = AssetLoader::create({mEngine, mMaterialProvider, mNameManager});
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({mEngine, ".", false});
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

TEST_F(glTFIOTest, MeshoptPreconditions) {
    // Mode ATTRIBUTES: stride must be a multiple of 4 and <= 256.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 3));
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 260));

    // Mode TRIANGLES: count must be a multiple of 3.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_triangles, 4, 2));

    // Mode TRIANGLES / INDICES: stride must be 2 or 4.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_triangles, 6, 1));
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_triangles, 6, 3));
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_indices, 4, 1));
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_indices, 4, 3));

    // Mode TRIANGLES / INDICES: filters are disallowed.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_triangles, 6, 2,
            cgltf_meshopt_compression_filter_octahedral));
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_indices, 4, 2,
            cgltf_meshopt_compression_filter_exponential));

    // Unsupported mode.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_invalid, 4, 4));

    // Filter OCTAHEDRAL: stride must be 4 or 8.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 12,
            cgltf_meshopt_compression_filter_octahedral));

    // Filter QUATERNION: stride must be 8.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 4,
            cgltf_meshopt_compression_filter_quaternion));

    // Filter EXPONENTIAL: stride must be a multiple of 4.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 6,
            cgltf_meshopt_compression_filter_exponential));

    // Unsupported filter.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 4,
            cgltf_meshopt_compression_filter_max_enum));

    // Mode ATTRIBUTES: count exceeds theoretical maximum.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes,
            std::numeric_limits<size_t>::max(), 4));

    // Missing buffer.
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 4,
            cgltf_meshopt_compression_filter_none, nullptr, 0, 32, /*provideBuffer=*/false));

    // Null buffer data.
    cgltf_buffer nullDataBuffer{};
    nullDataBuffer.data = nullptr;
    nullDataBuffer.size = 32;
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 4,
            cgltf_meshopt_compression_filter_none, &nullDataBuffer));

    // Buffer bounds exceeded.
    uint8_t dummyPayload[16] = {0};
    cgltf_buffer boundedBuffer{};
    boundedBuffer.data = dummyPayload;
    boundedBuffer.size = sizeof(dummyPayload);
    EXPECT_FALSE(runDecodeMeshopt(cgltf_meshopt_compression_mode_attributes, 4, 4,
            cgltf_meshopt_compression_filter_none, &boundedBuffer, /*offset=*/8, /*size=*/16));
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

// Builds a minimal single-primitive GLB whose index accessor can be given an arbitrary type,
// component type, count and bufferView byteStride. This is what lets a test express the case where
// the IndexBuffer capacity (count * componentSize) and the size computed from the accessor's
// stride and type disagree.
static std::vector<uint8_t> makeIndexAccessorGlb(char const* indexType, int indexComponentType,
        uint32_t indexCount, uint32_t indexByteStride) {
    constexpr uint32_t kBinSize = 256u;
    // Three vertices, so that the all-zero index data in the bin chunk stays in bounds and the
    // only thing under test is the index accessor's own layout.
    constexpr uint32_t kPositionCount = 3u;
    constexpr uint32_t kPositionBytes = kPositionCount * 12u;

    std::string strideJson;
    if (indexByteStride > 0) {
        strideJson = ", \"byteStride\": " + std::to_string(indexByteStride);
    }

    std::string json = std::string(R"({
  "asset": { "version": "2.0" },
  "buffers": [
    { "byteLength": )") + std::to_string(kBinSize) + R"( }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": )" + std::to_string(kPositionBytes) + R"( },
    { "buffer": 0, "byteOffset": )" + std::to_string(kPositionBytes) + R"(, "byteLength": )" +
            std::to_string(kBinSize - kPositionBytes) + strideJson + R"( }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": )" + std::to_string(kPositionCount) + R"(,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [0, 0, 0]
    },
    {
      "bufferView": 1,
      "componentType": )" + std::to_string(indexComponentType) + R"(,
      "count": )" + std::to_string(indexCount) + R"(,
      "type": ")" + indexType + R"("
    }
  ],
  "meshes": [
    {
      "primitives": [
        { "attributes": { "POSITION": 0 }, "indices": 1 }
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

    std::vector<uint8_t> bin(kBinSize, 0);

    uint32_t const jsonSize = uint32_t(json.size());
    uint32_t const binSize = uint32_t(bin.size());
    uint32_t const totalSize = 12u + 8u + jsonSize + 8u + binSize;

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


static std::vector<uint8_t> makeMorphTargetGlb(int morphTargetCount) {
    std::string targets;
    std::string weights = "[";
    for (int i = 0; i < morphTargetCount; ++i) {
        targets += (i == 0) ? "{\"TANGENT\":2}" : ",{\"TANGENT\":2}";
        weights += (i == 0) ? "0.0" : ",0.0";
    }
    weights += "]";

    std::string json =
            "{\"asset\":{\"version\":\"2.0\"},\"scene\":0,\"scenes\":[{\"nodes\":[0]}],"
            "\"nodes\":[{\"mesh\":0}],"
            "\"meshes\":[{\"weights\":" + weights + ",\"primitives\":[{\"attributes\":{\"POSITION\":0,\"TANGENT\":1},\"mode\":4,"
            "\"targets\":[" + targets + "]}]}],"
            "\"accessors\":["
            "{\"bufferView\":0,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\","
            "\"min\":[0,0,0],\"max\":[1,1,1]},"
            "{\"bufferView\":1,\"componentType\":5126,\"count\":3,\"type\":\"VEC4\"},"
            "{\"bufferView\":2,\"componentType\":5126,\"count\":3,\"type\":\"VEC3\"}],"
            "\"bufferViews\":[{\"buffer\":0,\"byteOffset\":0,\"byteLength\":36},"
            "{\"buffer\":0,\"byteOffset\":36,\"byteLength\":48},"
            "{\"buffer\":0,\"byteOffset\":84,\"byteLength\":36}],"
            "\"buffers\":[{\"byteLength\":120}]}";

    while ((json.size() % 4u) != 0u) {
        json.push_back(' ');
    }

    std::vector<uint8_t> bin(120, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // 3 vertices positions
    fbin[0] = 0.0f; fbin[1] = 0.0f; fbin[2] = 0.0f;
    fbin[3] = 1.0f; fbin[4] = 0.0f; fbin[5] = 0.0f;
    fbin[6] = 0.0f; fbin[7] = 1.0f; fbin[8] = 0.0f;
    // 3 vertices base tangents (vec4) at byteOffset 36 = float offset 9
    fbin[9]  = 1.0f; fbin[10] = 0.0f; fbin[11] = 0.0f; fbin[12] = 1.0f;
    fbin[13] = 1.0f; fbin[14] = 0.0f; fbin[15] = 0.0f; fbin[16] = 1.0f;
    fbin[17] = 1.0f; fbin[18] = 0.0f; fbin[19] = 0.0f; fbin[20] = 1.0f;
    // 3 vertices target tangent deltas (vec3) at byteOffset 84 = float offset 21
    fbin[21] = 0.0f; fbin[22] = 0.0f; fbin[23] = 0.0f;
    fbin[24] = 0.0f; fbin[25] = 0.0f; fbin[26] = 0.0f;
    fbin[27] = 0.0f; fbin[28] = 0.0f; fbin[29] = 0.0f;

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

enum class MorphMaterial { NONE, UNLIT, LIT };

// The morph position delta stored for vertex `v` by makeSparseTangentMorphGlb().
static math::float3 sparseMorphPositionDelta(int v) {
    return { 0.25f * float(v + 1), 0.5f, -0.125f };
}

// The base vertex color stored by makeSparseTangentMorphGlb() when `withColors` is set.
static constexpr math::float4 kSparseMorphBaseColor = { 1.0f, 0.0f, 1.0f, 1.0f };

// Pads `json` and wraps it with `bin` into a GLB container.
static std::vector<uint8_t> assembleGlb(std::string json, std::vector<uint8_t> const& bin) {
    while ((json.size() % 4u) != 0u) {
        json.push_back(' ');
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

// Fills in the JSON fragments that declare `material` and reference it from the first primitive.
static void morphMaterialJson(MorphMaterial material, std::string* extensionsUsed,
        std::string* materials, std::string* materialRef) {
    switch (material) {
        case MorphMaterial::NONE:
            break;
        case MorphMaterial::UNLIT:
            *extensionsUsed = R"("extensionsUsed":["KHR_materials_unlit"],)";
            *materials = R"("materials":[{"extensions":{"KHR_materials_unlit":{}}}],)";
            *materialRef = R"(,"material":0)";
            break;
        case MorphMaterial::LIT:
            *materials = R"("materials":[{}],)";
            *materialRef = R"(,"material":0)";
            break;
    }
}

// Builds a single-triangle GLB with `morphTargetCount` morph targets. Every target carries the
// same POSITION delta (see sparseMorphPositionDelta), and only the target at `tangentTargetIndex`
// additionally carries a TANGENT delta. This reproduces the layouts of issues #10180 (no material)
// and #10500 (unlit material), where the extended loader used to size slotIndices by a filtered
// target count but index it by the raw target index. When `withColors` is set, the base primitive
// also has a COLOR_0 attribute. When `withUvs` is set, it also has a TEXCOORD_0 attribute, which
// makes lit primitives use mikktspace instead of the provided tangents.
static std::vector<uint8_t> makeSparseTangentMorphGlb(int morphTargetCount, int tangentTargetIndex,
        MorphMaterial material, bool withColors = false, bool withUvs = false) {
    std::string targets;
    std::string weights = "[";
    for (int i = 0; i < morphTargetCount; ++i) {
        if (i > 0) {
            targets += ",";
            weights += ",";
        }
        targets += (i == tangentTargetIndex) ? R"({"POSITION":3,"TANGENT":4})"
                                             : R"({"POSITION":3})";
        weights += "0.0";
    }
    weights += "]";

    std::string materialRef;
    std::string materials;
    std::string extensionsUsed;
    morphMaterialJson(material, &extensionsUsed, &materials, &materialRef);

    std::string const colorAttribute = withColors ? R"(,"COLOR_0":5)" : "";
    std::string const uvAttribute = withUvs ? R"(,"TEXCOORD_0":6)" : "";

    std::string const json =
            R"({"asset":{"version":"2.0"},)" + extensionsUsed + materials +
            R"("scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
            R"("meshes":[{"weights":)" + weights +
            R"(,"primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TANGENT":2)" +
            colorAttribute + uvAttribute + R"(},"mode":4)" + materialRef + R"(,"targets":[)" +
            targets + R"(]}]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]},)"
            R"({"bufferView":1,"componentType":5126,"count":3,"type":"VEC3"},)"
            R"({"bufferView":2,"componentType":5126,"count":3,"type":"VEC4"},)"
            R"({"bufferView":3,"componentType":5126,"count":3,"type":"VEC3",)"
            R"("min":[0.25,0.5,-0.125],"max":[0.75,0.5,-0.125]},)"
            R"({"bufferView":4,"componentType":5126,"count":3,"type":"VEC3"},)"
            R"({"bufferView":5,"componentType":5126,"count":3,"type":"VEC4"},)"
            R"({"bufferView":6,"componentType":5126,"count":3,"type":"VEC2"}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":36,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":72,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":120,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":156,"byteLength":36},)"
            R"({"buffer":0,"byteOffset":192,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":240,"byteLength":24}],)"
            R"("buffers":[{"byteLength":264}]})";

    std::vector<uint8_t> bin(264, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    // Base positions (3 x vec3) at float offset 0.
    fbin[3] = 1.0f;
    fbin[7] = 1.0f;
    // Base normals (3 x vec3) at float offset 9, all pointing along +Z.
    for (int v = 0; v < 3; ++v) {
        fbin[9 + v * 3 + 2] = 1.0f;
    }
    // Base tangents (3 x vec4) at float offset 18, all (1, 0, 0, 1).
    for (int v = 0; v < 3; ++v) {
        fbin[18 + v * 4 + 0] = 1.0f;
        fbin[18 + v * 4 + 3] = 1.0f;
    }
    // Morph position deltas (3 x vec3) at float offset 30.
    for (int v = 0; v < 3; ++v) {
        math::float3 const delta = sparseMorphPositionDelta(v);
        fbin[30 + v * 3 + 0] = delta.x;
        fbin[30 + v * 3 + 1] = delta.y;
        fbin[30 + v * 3 + 2] = delta.z;
    }
    // Morph tangent deltas (3 x vec3) at float offset 39 are left at zero.
    // Base colors (3 x vec4) at float offset 48.
    for (int v = 0; v < 3; ++v) {
        for (int c = 0; c < 4; ++c) {
            fbin[48 + v * 4 + c] = kSparseMorphBaseColor[c];
        }
    }
    // Base UVs (3 x vec2) at float offset 60, equal to the xy of the base positions.
    fbin[62] = 1.0f;
    fbin[65] = 1.0f;

    return assembleGlb(json, bin);
}

// Builds a non-indexed GLB with two triangles forming a unit quad, and a single morph target. The
// two vertices on the shared edge are duplicated, and the copies differ only in COLOR_0 (red for
// the first triangle, blue for the second). This is how hard color edges are usually authored. The
// morph delta depends only on the position, so the copies also receive identical deltas.
//
// With a lit material and TEXCOORD_0, the base tangent-space job keeps the copies apart (6
// vertices) because their colors differ, while the morph target job welds them (4 vertices)
// because it does not carry the base colors. The morph target data therefore does not line up
// with the base vertex buffer.
static std::vector<uint8_t> makeColorSeamMorphGlb(MorphMaterial material) {
    std::string materialRef;
    std::string materials;
    std::string extensionsUsed;
    morphMaterialJson(material, &extensionsUsed, &materials, &materialRef);

    std::string const json =
            R"({"asset":{"version":"2.0"},)" + extensionsUsed + materials +
            R"("scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
            R"("meshes":[{"weights":[0.0],)"
            R"("primitives":[{"attributes":{"POSITION":0,"NORMAL":1,"TEXCOORD_0":2,"COLOR_0":3},)"
            R"("mode":4)" + materialRef + R"(,"targets":[{"POSITION":4}]}]}],)"
            R"("accessors":[)"
            R"({"bufferView":0,"componentType":5126,"count":6,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[1,1,0]},)"
            R"({"bufferView":1,"componentType":5126,"count":6,"type":"VEC3"},)"
            R"({"bufferView":2,"componentType":5126,"count":6,"type":"VEC2"},)"
            R"({"bufferView":3,"componentType":5126,"count":6,"type":"VEC4"},)"
            R"({"bufferView":4,"componentType":5126,"count":6,"type":"VEC3",)"
            R"("min":[0,0,0],"max":[0,0,1]}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":72},)"
            R"({"buffer":0,"byteOffset":72,"byteLength":72},)"
            R"({"buffer":0,"byteOffset":144,"byteLength":48},)"
            R"({"buffer":0,"byteOffset":192,"byteLength":96},)"
            R"({"buffer":0,"byteOffset":288,"byteLength":72}],)"
            R"("buffers":[{"byteLength":360}]})";

    static constexpr math::float2 kCorners[6] = {
            { 0, 0 }, { 1, 0 }, { 0, 1 },   // first triangle
            { 1, 0 }, { 1, 1 }, { 0, 1 },   // second triangle, sharing the edge (1,0)-(0,1)
    };

    std::vector<uint8_t> bin(360, 0);
    float* fbin = reinterpret_cast<float*>(bin.data());
    for (int v = 0; v < 6; ++v) {
        math::float2 const p = kCorners[v];
        // Positions (6 x vec3) at float offset 0.
        fbin[v * 3 + 0] = p.x;
        fbin[v * 3 + 1] = p.y;
        // Normals (6 x vec3) at float offset 18, all pointing along +Z.
        fbin[18 + v * 3 + 2] = 1.0f;
        // UVs (6 x vec2) at float offset 36, equal to the xy of the positions.
        fbin[36 + v * 2 + 0] = p.x;
        fbin[36 + v * 2 + 1] = p.y;
        // Colors (6 x vec4) at float offset 48, red for the first triangle, blue for the second.
        fbin[48 + v * 4 + (v < 3 ? 0 : 2)] = 1.0f;
        fbin[48 + v * 4 + 3] = 1.0f;
        // Morph position deltas (6 x vec3) at float offset 72, a function of the position only.
        fbin[72 + v * 3 + 2] = 0.5f * (p.x + p.y);
    }

    return assembleGlb(json, bin);
}

} // namespace

TEST_F(glTFIOTest, MorphTargetsExceedingMaxComputeTangents) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    // 20,000 morph targets exceeds both MAX_MORPH_TARGETS (256) and JobSystem::MAX_JOB_COUNT (16,384).
    std::vector<uint8_t> glb = makeMorphTargetGlb(20000);
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_TRUE(resourceLoader.loadResources(asset));

    ASSERT_EQ(asset->getRenderableEntityCount(), 1u);
    Entity const renderable = asset->getRenderableEntities()[0];
    EXPECT_EQ(asset->getMorphTargetCountAt(renderable), MAX_MORPH_TARGETS);

    auto const& renderableManager = mEngine->getRenderableManager();
    auto const inst = renderableManager.getInstance(renderable);
    EXPECT_EQ(renderableManager.getMorphTargetCount(inst), MAX_MORPH_TARGETS);
    ASSERT_NE(renderableManager.getMorphTargetBuffer(inst), nullptr);
    EXPECT_EQ(renderableManager.getMorphTargetBuffer(inst)->getCount(), MAX_MORPH_TARGETS);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// Loads `glb` through the extended loader, uploads its resources, and checks that the single
// renderable exposes `expectedMorphTargets` morph targets. Out-of-bounds accesses are caught under
// ASan, and the slot bookkeeping invariants are checked by assert_invariant in debug builds.
static void loadExtendedMorphAsset(Engine* engine, MaterialProvider* materials,
        NameComponentManager* names, std::vector<uint8_t> const& glb,
        size_t expectedMorphTargets) {
    AssetConfigurationExtended ext{ .gltfPath = "." };
    AssetLoader* assetLoader = AssetLoader::create({
            .engine = engine, .materials = materials, .names = names, .ext = &ext });
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ engine, ".", false });
    EXPECT_TRUE(resourceLoader.loadResources(asset));

    ASSERT_EQ(asset->getRenderableEntityCount(), 1u);
    EXPECT_EQ(asset->getMorphTargetCountAt(asset->getRenderableEntities()[0]),
            expectedMorphTargets);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// Issue #10180: with no material, a POSITION-only target preceding a target with TANGENT used to be
// dropped from the extended loader's morph target list, which made slotIndices too small for the
// raw index of the later target.
TEST_F(glTFIOTest, ExtendedMorphTargetsWithoutMaterialDoNotOverflow) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended asset loader is not supported on this platform.";
    }
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(2, 1, MorphMaterial::NONE), 2u);
}

// Issue #10500: with an unlit material, only targets with TANGENT used to be kept, so a single
// high-indexed target with TANGENT wrote far past the end of slotIndices.
TEST_F(glTFIOTest, ExtendedMorphTargetsWithUnlitMaterialDoNotOverflow) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended asset loader is not supported on this platform.";
    }
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(10, 9, MorphMaterial::UNLIT), 10u);
}

// A lit material exercises the path that computes morphed tangent frames for every target. Without
// UVs, the provided tangents are used. With UVs, mikktspace is used, and it remeshes.
TEST_F(glTFIOTest, ExtendedMorphTargetsWithLitMaterial) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended asset loader is not supported on this platform.";
    }
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(10, 9, MorphMaterial::LIT), 10u);
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(10, 9, MorphMaterial::LIT, /* withColors = */ false,
                    /* withUvs = */ true),
            10u);
}

// A morph target job passes its position deltas through the COLORS auxiliary attribute. A base
// COLOR_0 attribute used to overwrite (or leak) that storage depending on hash map iteration order.
// This runs the job directly and requires that the output positions are exactly the morph deltas.
static void expectMorphJobPositionsAreDeltas(MorphMaterial material) {
    std::vector<uint8_t> const glb =
            makeSparseTangentMorphGlb(2, 1, material, /* withColors = */ true);

    cgltf_options options{};
    cgltf_data* data = nullptr;
    ASSERT_EQ(cgltf_parse(&options, glb.data(), glb.size(), &data), cgltf_result_success);
    ASSERT_EQ(cgltf_load_buffers(&options, data, "."), cgltf_result_success);
    ASSERT_EQ(data->meshes_count, 1u);
    cgltf_primitive const* prim = &data->meshes[0].primitives[0];

    for (int target = 0; target < 2; ++target) {
        TangentsJobExtended::Params params{ .in = { .prim = prim, .morphTargetIndex = target } };
        TangentsJobExtended::run(&params);
        auto& out = params.out;

        ASSERT_EQ(out.vertexCount, 3u);
        ASSERT_NE(out.positions, nullptr);
        for (int v = 0; v < 3; ++v) {
            math::float3 const expected = sparseMorphPositionDelta(v);
            EXPECT_FLOAT_EQ(out.positions[v].x, expected.x) << "target " << target << " v " << v;
            EXPECT_FLOAT_EQ(out.positions[v].y, expected.y) << "target " << target << " v " << v;
            EXPECT_FLOAT_EQ(out.positions[v].z, expected.z) << "target " << target << " v " << v;
        }
        // Morph target jobs do not produce colors.
        EXPECT_EQ(out.colors, nullptr);

        free(out.positions);
        free(out.tbn);
        free(out.triangles);
    }
    cgltf_free(data);
}

TEST(glTFIOTangentsJobExtended, MorphTargetWithBaseColorsUnlit) {
    expectMorphJobPositionsAreDeltas(MorphMaterial::UNLIT);
}

TEST(glTFIOTangentsJobExtended, MorphTargetWithBaseColorsLit) {
    expectMorphJobPositionsAreDeltas(MorphMaterial::LIT);
}

// End-to-end load of morph targets on a primitive with COLOR_0 through the extended loader.
TEST_F(glTFIOTest, ExtendedMorphTargetsWithBaseColors) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended asset loader is not supported on this platform.";
    }
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(2, 1, MorphMaterial::NONE, /* withColors = */ true), 2u);
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeSparseTangentMorphGlb(2, 1, MorphMaterial::UNLIT, /* withColors = */ true), 2u);
}

// Runs the base and the morph target tangent-space jobs on the color edge GLB, and returns their
// output vertex counts.
static void runColorSeamJobs(MorphMaterial material, size_t* baseVertexCount,
        size_t* morphVertexCount) {
    std::vector<uint8_t> const glb = makeColorSeamMorphGlb(material);

    cgltf_options options{};
    cgltf_data* data = nullptr;
    ASSERT_EQ(cgltf_parse(&options, glb.data(), glb.size(), &data), cgltf_result_success);
    ASSERT_EQ(cgltf_load_buffers(&options, data, "."), cgltf_result_success);
    ASSERT_EQ(data->meshes_count, 1u);
    cgltf_primitive const* prim = &data->meshes[0].primitives[0];

    auto runJob = [prim](int morphTargetIndex) {
        TangentsJobExtended::Params params{
                .in = { .prim = prim, .morphTargetIndex = morphTargetIndex } };
        TangentsJobExtended::run(&params);
        auto& out = params.out;
        size_t const vertexCount = out.vertexCount;
        free(out.triangles);
        free(out.tbn);
        free(out.uv0);
        free(out.uv1);
        free(out.positions);
        free(out.joints);
        free(out.weights);
        free(out.colors);
        return vertexCount;
    };
    *baseVertexCount = runJob(TangentsJobExtended::kMorphTargetUnused);
    *morphVertexCount = runJob(0);
    cgltf_free(data);
}

// Guards the premise of ExtendedMorphTargetsWithColorSeam: with a lit material, mikktspace welds
// the morph target vertices differently from the base vertices. If this ever stops being true
// (e.g. once morph data is mapped through the triangle corners), the loader test below no longer
// exercises the mismatch path, and should be revisited.
TEST(glTFIOTangentsJobExtended, ColorSeamMorphTargetRemeshesDifferently) {
    size_t baseVertexCount = 0;
    size_t morphVertexCount = 0;
    runColorSeamJobs(MorphMaterial::LIT, &baseVertexCount, &morphVertexCount);
    EXPECT_EQ(baseVertexCount, 6u);
    EXPECT_EQ(morphVertexCount, 4u);

    // The unlit path does not remesh, so the counts always match.
    runColorSeamJobs(MorphMaterial::UNLIT, &baseVertexCount, &morphVertexCount);
    EXPECT_EQ(baseVertexCount, 6u);
    EXPECT_EQ(morphVertexCount, 6u);
}

// A morph target whose remeshed vertex count differs from the base used to be uploaded with the
// base count, which read past the end of the morph data. It is now neutralized with a warning, and
// the asset must still load with all of its morph targets.
TEST_F(glTFIOTest, ExtendedMorphTargetsWithColorSeam) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended asset loader is not supported on this platform.";
    }
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeColorSeamMorphGlb(MorphMaterial::LIT), 1u);
    loadExtendedMorphAsset(mEngine, mMaterialProvider, mNameManager,
            makeColorSeamMorphGlb(MorphMaterial::UNLIT), 1u);
}

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

// The IndexBuffer is allocated as (count * componentSize), so an accessor whose byte span is
// computed from a larger stride or a wider type would overrun it. AssetLoader rejects such an
// accessor before the IndexBuffer exists, which is strictly earlier than the cgltf_validate() call
// in ResourceLoader, so a null return here proves that gltfio's own check is what fired.
TEST_F(glTFIOTest, RejectsNonScalarIndexAccessor) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    // VEC3 of unsigned shorts: capacity would be count * 2, upload would be count * 6.
    std::vector<uint8_t> glb = makeIndexAccessorGlb("VEC3", 5123, 3u, 0u);
    EXPECT_EQ(loader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);

    AssetLoader::destroy(&loader);
}

TEST_F(glTFIOTest, RejectsStridedIndexAccessor) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    // A byteStride of 8 on a scalar ushort accessor: capacity would be count * 2, upload would be
    // 8 * (count - 1) + 2.
    std::vector<uint8_t> glb = makeIndexAccessorGlb("SCALAR", 5123, 3u, 8u);
    EXPECT_EQ(loader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);

    AssetLoader::destroy(&loader);
}

TEST_F(glTFIOTest, AcceptsConformingIndexAccessor) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    std::vector<uint8_t> glb = makeIndexAccessorGlb("SCALAR", 5123, 3u, 0u);
    FilamentAsset* asset = loader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_TRUE(resourceLoader.loadResources(asset));

    loader->destroyAsset(asset);
    AssetLoader::destroy(&loader);
}

TEST_F(glTFIOTest, UploadableIndexAccessorContract) {
    cgltf_accessor accessor{};
    accessor.type = cgltf_type_scalar;
    accessor.component_type = cgltf_component_type_r_16u;
    accessor.stride = 2;
    accessor.count = 3;
    EXPECT_TRUE(gltfio::utility::isUploadableIndexAccessor(&accessor));

    // A wider type inflates cgltf_calc_size() without changing the allocated capacity.
    accessor.type = cgltf_type_vec3;
    EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));
    accessor.type = cgltf_type_scalar;

    // A stride larger than the component size spreads the source over more bytes than the
    // destination holds.
    accessor.stride = 8;
    EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));
    accessor.stride = 2;

    // Sparse contents are not the raw bytes of the bufferView, so a straight copy is wrong.
    accessor.is_sparse = 1;
    EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));
    accessor.is_sparse = 0;

    // A count beyond uint32 is only representable where cgltf_size is wider than uint32_t.
    if constexpr (sizeof(cgltf_size) > sizeof(uint32_t)) {
        accessor.count = cgltf_size(std::numeric_limits<uint32_t>::max()) + 1;
        EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));
    }

    // A count whose byte product wraps size_t is rejected on every target.
    accessor.count = std::numeric_limits<cgltf_size>::max();
    EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));
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
