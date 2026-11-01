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

#include "downcast.h"
#include "DracoCache.h"
#include "FFilamentInstance.h"
#include "GlbBuilders.h"
#include "Utility.h"

#include "extended/TangentsJobExtended.h"

#include "materials/uberarchive.h"

#include <gltfio/Animator.h>
#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/FilamentInstance.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/math.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <filament/Box.h>
#include <filament/Engine.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Panic.h>
#include <utils/Path.h>

#include <math/mathfwd.h>
#include <math/vec3.h>

#include <cgltf.h>
#if GLTFIO_DRACO_SUPPORTED
#include <draco/mesh/mesh.h>
#endif
#include <gtest/gtest.h>

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
using namespace test;

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

std::vector<uint8_t> loadFileToBytes(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    // Read the file stream directly into the vector
    return std::vector<uint8_t>(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
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

// Issue #10409: when a mesh declares more morph targets than MAX_MORPH_TARGETS, its morph-weight
// animation channels still contain one weight per declared target per keyframe.
// Animator::applyAnimation must clamp the number of weights passed to
// RenderableManager::setMorphWeights to MAX_MORPH_TARGETS for both LINEAR and CUBICSPLINE
// interpolation, rather than triggering a precondition panic.
TEST_F(glTFIOTest, MorphTargetWeightAnimationExceedingMaxDoesNotPanic) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb = makeAnimatedMorphTargetGlb(int(MAX_MORPH_TARGETS) + 44);
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    ASSERT_TRUE(resourceLoader.loadResources(asset));

    FilamentInstance* const instance = asset->getInstance();
    ASSERT_NE(instance, nullptr);
    Animator* const animator = instance->getAnimator();
    ASSERT_NE(animator, nullptr);
    ASSERT_EQ(animator->getAnimationCount(), 2u);

    // Animation 0 uses LINEAR interpolation; animation 1 uses CUBICSPLINE interpolation.
    EXPECT_NO_THROW(animator->applyAnimation(0, 0.5f));
    EXPECT_NO_THROW(animator->applyAnimation(1, 0.5f));

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

    std::vector<uint8_t> glb = makeMalformedEightBitIndexGlb(uint32_t(backend::MAX_INDEX_COUNT));
    FilamentAsset* asset = loader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    loader->destroyAsset(asset);
    AssetLoader::destroy(&loader);
}

TEST_F(glTFIOTest, RejectsIndexAccessorAboveMaxIndexCount) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    std::vector<uint8_t> glb = makeIndexAccessorGlb("SCALAR", 5123,
            uint32_t(backend::MAX_INDEX_COUNT + 1), 0u);
    EXPECT_EQ(loader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);

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

    // IndexBuffer::Builder::build() caps indexCount at backend::MAX_INDEX_COUNT.
    accessor.count = backend::MAX_INDEX_COUNT;
    EXPECT_TRUE(gltfio::utility::isUploadableIndexAccessor(&accessor));

    accessor.count = backend::MAX_INDEX_COUNT + 1;
    EXPECT_FALSE(gltfio::utility::isUploadableIndexAccessor(&accessor));

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

// A morph-target NORMAL/TANGENT accessor may declare an arbitrary element type. Such accessors
// skip the getElementType validation in createPrimitive and are consumed by the tangent-space
// job in ResourceLoader, which unpacks them into a fixed 3-float-per-vertex scratch buffer. A
// matrix-typed (e.g. MAT4, 16-component) accessor therefore causes more floats to be written per
// element than the buffer holds -- a heap out-of-bounds write on the default, all-platform
// loader, from an untrusted, cgltf_validate-clean .glb.
//
// This test loads such a file. Before the fix it produced a heap-buffer-overflow (visible under
// AddressSanitizer) while computing tangents; after the fix the unsupported accessor type is
// rejected at load, so createAsset returns null and no out-of-bounds access occurs.
TEST_F(glTFIOTest, MorphTargetTangentInvalidTypeIsRejected) {
    AssetLoader* loader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(loader, nullptr);

    Path gltfPath = Path::getCurrentExecutable()
        .getParent()
        .concat("invalid_accessor_type.glb");
    const std::vector<uint8_t> buffer = loadFileToBytes(gltfPath.getAbsolutePath());
    FilamentAsset* asset = loader->createAsset(buffer.data(), buffer.size());

    // The MAT4-typed morph tangent accessor must be rejected at load.
    EXPECT_EQ(asset, nullptr);

    if (asset) {
        // Defense in depth: if a future change lets the asset load, resource loading (which runs
        // the tangent-space computation) must still not perform an out-of-bounds access.
        ResourceLoader resourceLoader({ mEngine, ".", false });
        resourceLoader.loadResources(asset);
        loader->destroyAsset(asset);
    }
    AssetLoader::destroy(&loader);
}

// Issue #10638: a glTF primitive without a POSITION attribute is valid per cgltf_validate(), and
// the glTF 2.0 specification (Section 3.7.2.1 implementation note) states that client
// implementations should skip rendering such a primitive. Previously, AssetLoader created a
// VertexBuffer and scheduled normal/tangent generation for it, causing SurfaceOrientation::Builder
// to panic (Debug) or return nullptr and crash on dereference in TangentsJob::run (Release).
TEST_F(glTFIOTest, PrimitiveWithoutPositionsIsSkipped) {
    std::vector<uint8_t> const glb = makePositionlessPrimitiveGlb(false);

    auto checkLoad = [&](AssetConfigurationExtended* ext) {
        AssetLoader* assetLoader = AssetLoader::create({
                .engine = mEngine,
                .materials = mMaterialProvider,
                .names = mNameManager,
                .ext = ext,
        });
        ASSERT_NE(assetLoader, nullptr);

        FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        EXPECT_TRUE(resourceLoader.loadResources(asset));

        ASSERT_EQ(asset->getRenderableEntityCount(), 1u);
        Entity const renderable = asset->getRenderableEntities()[0];
        auto const& rm = mEngine->getRenderableManager();
        auto const inst = rm.getInstance(renderable);
        EXPECT_EQ(rm.getPrimitiveCount(inst), 1u);
        EXPECT_TRUE(rm.getEnabledAttributesAt(inst, 0).none());

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    };

    checkLoad(nullptr);
    if (AssetConfigurationExtended::isSupported()) {
        AssetConfigurationExtended ext{ .gltfPath = "." };
        checkLoad(&ext);
    }
}

TEST_F(glTFIOTest, MeshWithPositionlessAndValidPrimitives) {
    std::vector<uint8_t> const glb = makePositionlessPrimitiveGlb(true);

    auto checkLoad = [&](AssetConfigurationExtended* ext) {
        AssetLoader* assetLoader = AssetLoader::create({
                .engine = mEngine,
                .materials = mMaterialProvider,
                .names = mNameManager,
                .ext = ext,
        });
        ASSERT_NE(assetLoader, nullptr);

        FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        EXPECT_TRUE(resourceLoader.loadResources(asset));

        ASSERT_EQ(asset->getRenderableEntityCount(), 1u);
        Entity const renderable = asset->getRenderableEntities()[0];
        auto const& rm = mEngine->getRenderableManager();
        auto const inst = rm.getInstance(renderable);
        ASSERT_EQ(rm.getPrimitiveCount(inst), 2u);

        // Primitive 0 has no POSITION and must be skipped.
        EXPECT_TRUE(rm.getEnabledAttributesAt(inst, 0).none());

        // Primitive 1 has POSITION and must be loaded with generated tangents.
        AttributeBitset const attribs1 = rm.getEnabledAttributesAt(inst, 1);
        EXPECT_TRUE(attribs1[VertexAttribute::POSITION]);
        EXPECT_TRUE(attribs1[VertexAttribute::TANGENTS]);

        // The asset bounding box must reflect only the valid primitive.
        Aabb const bounds = asset->getBoundingBox();
        EXPECT_FLOAT_EQ(bounds.min.x, 0.0f);
        EXPECT_FLOAT_EQ(bounds.min.y, 0.0f);
        EXPECT_FLOAT_EQ(bounds.min.z, 0.0f);
        EXPECT_FLOAT_EQ(bounds.max.x, 1.0f);
        EXPECT_FLOAT_EQ(bounds.max.y, 1.0f);
        EXPECT_FLOAT_EQ(bounds.max.z, 0.0f);

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    };

    checkLoad(nullptr);
    if (AssetConfigurationExtended::isSupported()) {
        AssetConfigurationExtended ext{ .gltfPath = "." };
        checkLoad(&ext);
    }
}

TEST_F(glTFIOTest, EmptyImageDoesNotCrash) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);
    TextureProvider* stbProvider = createStbProvider(mEngine);
    ASSERT_NE(stbProvider, nullptr);

    auto loadGltfJson = [&](char const* gltfJson) {
        FilamentAsset* asset = assetLoader->createAsset(
                reinterpret_cast<uint8_t const*>(gltfJson), uint32_t(std::strlen(gltfJson)));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        resourceLoader.addTextureProvider("image/png", stbProvider);
        EXPECT_TRUE(resourceLoader.loadResources(asset));

        assetLoader->destroyAsset(asset);
    };

    // Case 1: Image with neither URI, bufferView, nor mimeType.
    loadGltfJson(
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[]}],)"
            R"("images":[{}],"textures":[{"source":0}],)"
            R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}}}]})");

    // Case 2: Image with mimeType but neither URI nor bufferView.
    loadGltfJson(
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[]}],)"
            R"("images":[{"mimeType":"image/png"}],"textures":[{"source":0}],)"
            R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}}}]})");

    // Case 3: Image with bufferView but missing mimeType and buffer data.
    loadGltfJson(
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[]}],)"
            R"("buffers":[{"byteLength":0}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":0}],)"
            R"("images":[{"bufferView":0}],"textures":[{"source":0}],)"
            R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}}}]})");

    // Case 4: Image with bufferView and mimeType, but missing buffer data.
    loadGltfJson(
            R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[]}],)"
            R"("buffers":[{"byteLength":0}],)"
            R"("bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":0}],)"
            R"("images":[{"bufferView":0,"mimeType":"image/png"}],"textures":[{"source":0}],)"
            R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}}}]})");

    delete stbProvider;
    AssetLoader::destroy(&assetLoader);
}

// Issue #10636: when a primitive with KHR_draco_mesh_compression fails to decode, its accessors
// remain without a bufferView. Per the glTF 2.0 spec, accessors without a bufferView must be
// initialized with zeros. Verify that TangentsJobExtended zero-fills the unpacked attributes and
// that both the extended and standard loaders handle the failed decode without crashing.
TEST(glTFIOTangentsJobExtended, InvalidDracoDecodeZeroFillsAttributes) {
    std::vector<uint8_t> const glb = makeInvalidDracoGlb();

    cgltf_options options{};
    cgltf_data* data = nullptr;
    ASSERT_EQ(cgltf_parse(&options, glb.data(), glb.size(), &data), cgltf_result_success);
    ASSERT_EQ(cgltf_load_buffers(&options, data, "."), cgltf_result_success);
    ASSERT_EQ(data->meshes_count, 1u);
    cgltf_primitive const* prim = &data->meshes[0].primitives[0];

    TangentsJobExtended::Params params{
            .in = { .prim = prim, .morphTargetIndex = TangentsJobExtended::kMorphTargetUnused } };
    TangentsJobExtended::run(&params);
    auto& out = params.out;

    ASSERT_EQ(out.vertexCount, 3u);
    ASSERT_NE(out.positions, nullptr);
    for (size_t v = 0; v < out.vertexCount; ++v) {
        EXPECT_FLOAT_EQ(out.positions[v].x, 0.0f);
        EXPECT_FLOAT_EQ(out.positions[v].y, 0.0f);
        EXPECT_FLOAT_EQ(out.positions[v].z, 0.0f);
    }

    free(out.positions);
    free(out.tbn);
    free(out.triangles);
    cgltf_free(data);
}

TEST_F(glTFIOTest, InvalidDracoDecodeLoadsWithZeroFilledBuffers) {
    std::vector<uint8_t> const glb = makeInvalidDracoGlb();

    // Standard loader path.
    {
        AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
        ASSERT_NE(assetLoader, nullptr);

        FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        EXPECT_TRUE(resourceLoader.loadResources(asset));
        EXPECT_EQ(asset->getRenderableEntityCount(), 1u);

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    }

    // Extended loader path.
    if (AssetConfigurationExtended::isSupported()) {
        AssetConfigurationExtended ext{ .gltfPath = "." };
        AssetLoader* assetLoader = AssetLoader::create({
                .engine = mEngine, .materials = mMaterialProvider, .names = mNameManager, .ext = &ext });
        ASSERT_NE(assetLoader, nullptr);

        FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        EXPECT_TRUE(resourceLoader.loadResources(asset));
        EXPECT_EQ(asset->getRenderableEntityCount(), 1u);

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    }
}

// Issue #10202: constrainMaterial() must reject UV indices >= UvMapSize without indexing out of
// bounds on the stack-allocated UvMap, and disable the corresponding texture flags.
TEST(glTFIOMaterialProvider, ConstrainMaterialRejectsOutOfBoundsUvIndices) {
    for (uint8_t const badUv : { uint8_t(UvMapSize), uint8_t(100), uint8_t(127) }) {
        MaterialKey key {
            .hasBaseColorTexture = true,
            .hasNormalTexture = true,
            .hasOcclusionTexture = true,
            .hasEmissiveTexture = true,
            .hasMetallicRoughnessTexture = true,
            .metallicRoughnessUV = badUv,
            .baseColorUV = badUv,
            .hasClearCoatTexture = true,
            .clearCoatUV = badUv,
            .hasClearCoatRoughnessTexture = true,
            .clearCoatRoughnessUV = badUv,
            .hasClearCoatNormalTexture = true,
            .clearCoatNormalUV = badUv,
            .hasClearCoat = true,
            .hasTransmission = true,
            .emissiveUV = badUv,
            .aoUV = badUv,
            .normalUV = badUv,
            .hasTransmissionTexture = true,
            .transmissionUV = badUv,
            .hasSheenColorTexture = true,
            .sheenColorUV = badUv,
            .hasSheenRoughnessTexture = true,
            .sheenRoughnessUV = badUv,
            .hasVolumeThicknessTexture = true,
            .volumeThicknessUV = badUv,
            .hasSheen = true,
            .hasVolume = true,
            .hasSpecular = true,
            .hasSpecularTexture = true,
            .hasSpecularColorTexture = true,
            .specularTextureUV = badUv,
            .specularColorTextureUV = badUv,
        };

        UvMap uvmap {};
        constrainMaterial(&key, &uvmap);

        EXPECT_FALSE(key.hasBaseColorTexture);
        EXPECT_FALSE(key.hasMetallicRoughnessTexture);
        EXPECT_FALSE(key.hasNormalTexture);
        EXPECT_FALSE(key.hasOcclusionTexture);
        EXPECT_FALSE(key.hasEmissiveTexture);
        EXPECT_FALSE(key.hasTransmissionTexture);
        EXPECT_FALSE(key.hasClearCoatTexture);
        EXPECT_FALSE(key.hasClearCoatRoughnessTexture);
        EXPECT_FALSE(key.hasClearCoatNormalTexture);
        EXPECT_FALSE(key.hasSheenColorTexture);
        EXPECT_FALSE(key.hasSheenRoughnessTexture);
        EXPECT_FALSE(key.hasVolumeThicknessTexture);
        EXPECT_FALSE(key.hasSpecularTexture);
        EXPECT_FALSE(key.hasSpecularColorTexture);
        EXPECT_EQ(getNumUvSets(uvmap), 0u);
    }

    // A high-priority texture with an out-of-bounds UV index is dropped without consuming one of
    // the two available Filament UV slots, so lower-priority textures with valid UV indices still
    // receive UV0 and UV1.
    MaterialKey mixedKey {
        .hasBaseColorTexture = true,
        .hasNormalTexture = true,
        .hasEmissiveTexture = true,
        .baseColorUV = 100,
        .emissiveUV = 5,
        .normalUV = 3,
    };
    UvMap mixedUvMap {};
    constrainMaterial(&mixedKey, &mixedUvMap);

    EXPECT_FALSE(mixedKey.hasBaseColorTexture);
    EXPECT_TRUE(mixedKey.hasNormalTexture);
    EXPECT_TRUE(mixedKey.hasEmissiveTexture);
    EXPECT_EQ(mixedUvMap[3], gltfio::UV0);
    EXPECT_EQ(mixedUvMap[5], gltfio::UV1);
    EXPECT_EQ(getNumUvSets(mixedUvMap), 2u);
}

// Issue #10202: processShaderString() must not index uvmap or uvstrings out of bounds when
// MaterialKey carries UV indices >= UvMapSize (including on disabled texture slots) or when UvMap
// holds an out-of-range enum value.
TEST(glTFIOMaterialProvider, ProcessShaderStringHandlesOutOfBoundsUvIndices) {
    MaterialKey key {
        .metallicRoughnessUV = 100,
        .baseColorUV = 200,
        .clearCoatUV = 8,
        .clearCoatRoughnessUV = 9,
        .clearCoatNormalUV = 10,
        .emissiveUV = 250,
        .aoUV = 255,
        .normalUV = 8,
        .transmissionUV = 127,
        .sheenColorUV = 50,
        .sheenRoughnessUV = 60,
        .volumeThicknessUV = 70,
        .specularTextureUV = 80,
        .specularColorTextureUV = 90,
    };

    UvMap uvmap {};
    uvmap[0] = gltfio::UV0;
    uvmap[1] = gltfio::UV1;

    std::string shader =
            "${normal}|${color}|${metallic}|${ao}|${emissive}|${transmission}|"
            "${clearCoat}|${clearCoatRoughness}|${clearCoatNormal}|"
            "${sheenColor}|${sheenRoughness}|${volumeThickness}|${specular}|${specularColor}";
    processShaderString(&shader, uvmap, key);

    EXPECT_EQ(shader,
            "vec2(0)|vec2(0)|vec2(0)|vec2(0)|vec2(0)|vec2(0)|"
            "vec2(0)|vec2(0)|vec2(0)|"
            "vec2(0)|vec2(0)|vec2(0)|vec2(0)|vec2(0)");

    // Also verify valid UV0 / UV1 replacements alongside an invalid UvSet value in uvmap.
    MaterialKey validKey {
        .metallicRoughnessUV = 1,
        .baseColorUV = 0,
        .normalUV = 2,
    };
    uvmap[2] = static_cast<UvSet>(99);
    std::string shader2 = "${color}|${metallic}|${normal}";
    processShaderString(&shader2, uvmap, validKey);
    EXPECT_EQ(shader2, "getUV0()|getUV1()|vec2(0)");
}

// Issue #10202: end-to-end test loading glTF assets whose material textures specify out-of-bounds
// texCoord indices (>= UvMapSize, negative, or multiples of 128/256 that would otherwise wrap into
// [0, UvMapSize - 1] on narrowing conversion) across both UbershaderProvider and JitShaderProvider,
// as well as both the standard and extended asset loaders.
TEST_F(glTFIOTest, OutOfBoundsMaterialTexCoordDoesNotOverflow) {
    MaterialProvider* jitProvider = createJitShaderProvider(mEngine);
    ASSERT_NE(jitProvider, nullptr);

    auto makeGltfWithTexCoord = [](int texCoord) -> std::string {
        std::string const tc = std::to_string(texCoord);
        return std::string(
                R"({"asset":{"version":"2.0"},)"
                R"("extensionsUsed":["KHR_materials_clearcoat","KHR_materials_transmission",)"
                R"("KHR_materials_sheen","KHR_materials_volume","KHR_materials_specular"],)"
                R"("scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],)"
                R"("meshes":[{"primitives":[)"
                R"({"attributes":{"POSITION":0,"TEXCOORD_0":1,"TEXCOORD_8":1},"material":0},)"
                R"({"attributes":{"POSITION":0,"TEXCOORD_0":1,"TEXCOORD_8":1},"material":1},)"
                R"({"attributes":{"POSITION":0,"TEXCOORD_0":1,"TEXCOORD_8":1},"material":2},)"
                R"({"attributes":{"POSITION":0,"TEXCOORD_0":1,"TEXCOORD_8":1},"material":3}]}],)"
                R"("accessors":[)"
                R"({"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[0,0,0],"max":[1,1,0]},)"
                R"({"bufferView":1,"componentType":5126,"count":3,"type":"VEC2"}],)"
                R"("bufferViews":[)"
                R"({"buffer":0,"byteOffset":0,"byteLength":36},)"
                R"({"buffer":0,"byteOffset":36,"byteLength":24}],)"
                R"("buffers":[{"byteLength":60,"uri":"data:application/octet-stream;base64,)"
                R"(AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAIA/"}],)"
                R"("images":[{"uri":"data:image/png;base64,)"
                R"(iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGP4z8DwHwAFAAH/iZk9HQAAAABJRU5ErkJggg=="}],)"
                R"("textures":[{"source":0}],)"
                R"("materials":[)"
                R"({)"
                R"("pbrMetallicRoughness":{)"
                R"("baseColorTexture":{"index":0,"texCoord":0},)"
                R"("metallicRoughnessTexture":{"index":0,"texCoord":)") + tc + R"(}},)"
                R"("normalTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("occlusionTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("emissiveTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("extensions":{)"
                R"("KHR_materials_clearcoat":{)"
                R"("clearcoatFactor":1.0,)"
                R"("clearcoatTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("clearcoatRoughnessTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("clearcoatNormalTexture":{"index":0,"texCoord":)" + tc + R"(}}}},)"
                R"({"extensions":{)"
                R"("KHR_materials_sheen":{)"
                R"("sheenColorFactor":[1,1,1],)"
                R"("sheenColorTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("sheenRoughnessTexture":{"index":0,"texCoord":)" + tc + R"(}}}},)"
                R"({"extensions":{)"
                R"("KHR_materials_transmission":{)"
                R"("transmissionFactor":0.5,)"
                R"("transmissionTexture":{"index":0,"texCoord":)" + tc + R"(}},)"
                R"("KHR_materials_volume":{)"
                R"("thicknessFactor":1.0,)"
                R"("thicknessTexture":{"index":0,"texCoord":)" + tc + R"(}}}},)"
                R"({"extensions":{)"
                R"("KHR_materials_specular":{)"
                R"("specularFactor":1.0,)"
                R"("specularTexture":{"index":0,"texCoord":)" + tc + R"(},)"
                R"("specularColorTexture":{"index":0,"texCoord":)" + tc + R"(}}}})"
                R"(]})";
    };

    for (MaterialProvider* provider : { mMaterialProvider, jitProvider }) {
        for (int const badTexCoord : { -1, 8, 100, 128, 256 }) {
            std::string const gltf = makeGltfWithTexCoord(badTexCoord);

            auto checkWithConfig = [&](AssetConfigurationExtended* ext) {
                AssetLoader* loader = AssetLoader::create({
                        .engine = mEngine,
                        .materials = provider,
                        .names = mNameManager,
                        .ext = ext,
                });
                ASSERT_NE(loader, nullptr);

                FilamentAsset* asset = loader->createAsset(
                        reinterpret_cast<uint8_t const*>(gltf.data()), uint32_t(gltf.size()));
                ASSERT_NE(asset, nullptr);

                ResourceLoader resourceLoader({ mEngine, ".", false });
                EXPECT_TRUE(resourceLoader.loadResources(asset));

                if (provider == jitProvider) {
                    // Material 0 has baseColorTexture at texCoord 0 and other textures at
                    // badTexCoord. Negative texCoord clamps to 0 (requiring only UV0), whereas
                    // texCoord >= UvMapSize (including 128 and 256) clamps to UvMapSize - 1
                    // rather than wrapping to 0 (requiring both UV0 and UV1).
                    FilamentInstance* instance = asset->getInstance();
                    ASSERT_NE(instance, nullptr);
                    ASSERT_EQ(instance->getMaterialInstanceCount(), 4u);
                    Material const* mat0 = instance->getMaterialInstances()[0]->getMaterial();
                    ASSERT_NE(mat0, nullptr);
                    AttributeBitset const required0 = mat0->getRequiredAttributes();
                    EXPECT_TRUE(required0.test(VertexAttribute::UV0)) << "texCoord=" << badTexCoord;
                    EXPECT_EQ(required0.test(VertexAttribute::UV1), badTexCoord >= UvMapSize)
                            << "texCoord=" << badTexCoord;
                }

                loader->destroyAsset(asset);
                AssetLoader::destroy(&loader);
            };

            checkWithConfig(nullptr);
            if (AssetConfigurationExtended::isSupported()) {
                AssetConfigurationExtended ext{ .gltfPath = "." };
                checkWithConfig(&ext);
            }
        }
    }

    jitProvider->destroyMaterials();
    delete jitProvider;
}

// Issue #10495: in FAssetLoader::createPrimitive(), `sizeof(ubyte4) * vertexCount` was computed in
// uint32_t when sizing the dummy vertex buffer. A vertexCount >= 1,073,741,824 wrapped around
// (e.g. 1,073,741,825 * 4 == 0x100000004 -> 4 bytes), allocating an undersized dummy buffer. Also,
// a 64-bit accessor count > UINT32_MAX truncated when assigned to `uint32_t vertexCount`.
TEST_F(glTFIOTest, DummyBufferSizeOverflowIsRejected) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    // 1,073,741,825 * 4 == 0x100000004 (wraps to 4 in uint32_t).
    {
        std::vector<uint8_t> const glb = makeDummyBufferOverflowGlb(1073741825ULL);
        EXPECT_EQ(assetLoader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);
    }

    // 1,073,741,824 * 4 == 0x100000000 (wraps to 0 in uint32_t).
    {
        std::vector<uint8_t> const glb = makeDummyBufferOverflowGlb(1073741824ULL);
        EXPECT_EQ(assetLoader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);
    }

    // 4,294,967,297 == 0x100000001 (wraps to 1 when truncated to uint32_t vertexCount).
    if constexpr (sizeof(cgltf_size) > sizeof(uint32_t)) {
        std::vector<uint8_t> const glb = makeDummyBufferOverflowGlb(4294967297ULL);
        EXPECT_EQ(assetLoader->createAsset(glb.data(), uint32_t(glb.size())), nullptr);
    }

    AssetLoader::destroy(&assetLoader);
}

// Issue #10624: a glTF with a deeply nested node hierarchy (e.g. 7,000 nodes) used to exhaust the
// call stack in FAssetLoader::traversePrimitives() and FAssetLoader::traverseEntities(). Verify
// that both the standard and extended loaders traverse the hierarchy iteratively without crashing
// and preserve the full parent-child transform chain.
TEST_F(glTFIOTest, DeeplyNestedNodesDoNotOverflowStack) {
    constexpr size_t kDepth = 5000;
    std::vector<uint8_t> const glb = makeDeeplyNestedNodesGlb(kDepth);

    auto checkLoad = [&](AssetConfigurationExtended* ext) {
        AssetLoader* assetLoader = AssetLoader::create({
                .engine = mEngine,
                .materials = mMaterialProvider,
                .names = mNameManager,
                .ext = ext,
        });
        ASSERT_NE(assetLoader, nullptr);

        FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        ResourceLoader resourceLoader({ mEngine, ".", false });
        EXPECT_TRUE(resourceLoader.loadResources(asset));

        EXPECT_EQ(asset->getEntityCount(), kDepth);
        ASSERT_EQ(asset->getRenderableEntityCount(), 1u);

        Aabb const bounds = asset->getBoundingBox();
        EXPECT_FLOAT_EQ(bounds.min.x, float(kDepth));
        EXPECT_FLOAT_EQ(bounds.min.y, 0.0f);
        EXPECT_FLOAT_EQ(bounds.min.z, 0.0f);
        EXPECT_FLOAT_EQ(bounds.max.x, float(kDepth + 1));
        EXPECT_FLOAT_EQ(bounds.max.y, 1.0f);
        EXPECT_FLOAT_EQ(bounds.max.z, 0.0f);

        Entity const leaf = asset->getRenderableEntities()[0];
        auto const& tm = mEngine->getTransformManager();
        math::mat4f const worldTransform = tm.getWorldTransform(tm.getInstance(leaf));
        EXPECT_FLOAT_EQ(worldTransform[3].x, float(kDepth));
        EXPECT_FLOAT_EQ(worldTransform[3].y, 0.0f);
        EXPECT_FLOAT_EQ(worldTransform[3].z, 0.0f);

        // Walk from the leaf up to the instance root to verify all kDepth links in the chain.
        FilamentInstance* const instance = asset->getInstance();
        ASSERT_NE(instance, nullptr);
        Entity current = leaf;
        size_t steps = 0;
        while (!current.isNull() && current != instance->getRoot()) {
            current = tm.getParent(tm.getInstance(current));
            ++steps;
        }
        EXPECT_EQ(current, instance->getRoot());
        EXPECT_EQ(steps, kDepth);

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    };

    checkLoad(nullptr);
    if (AssetConfigurationExtended::isSupported()) {
        AssetConfigurationExtended ext{ .gltfPath = "." };
        checkLoad(&ext);
    }
}

#if GLTFIO_DRACO_SUPPORTED
// Issue #10495: in DracoCache.cpp convertAttribs(), `const uint32_t size = target->stride * n;`
// overflowed 32-bit unsigned arithmetic when `stride * n > UINT32_MAX` (e.g. stride = 12 for VEC3
// float and n = 357,913,942 gives 0x100000008, truncating to 8 bytes). Verify that the overflow is
// rejected, no buffer_view is attached, and ~DracoMesh() cleanly destroys without freeing an
// uninitialized buffer pointer.
TEST_F(glTFIOTest, DracoVertexAttributeSizeOverflowIsRejected) {
    auto rawMesh = std::make_unique<draco::Mesh>();
    draco::GeometryAttribute geomAttr;
    geomAttr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false, 12, 0);
    int const attId = rawMesh->AddAttribute(geomAttr, true, 1);
    rawMesh->attribute(attId)->set_unique_id(0);

    constexpr uint32_t kOverflowPointCount = 357913942u; // 357,913,942 * 12 == 0x100000008
    rawMesh->set_num_points(kOverflowPointCount);

    std::unique_ptr<DracoMesh> dracoMesh(DracoMesh::createForTest(rawMesh.release()));
    ASSERT_NE(dracoMesh, nullptr);

    cgltf_accessor accessor{};
    accessor.type = cgltf_type_vec3;
    accessor.component_type = cgltf_component_type_r_32f;
    accessor.stride = 12;
    accessor.count = kOverflowPointCount;

    EXPECT_FALSE(dracoMesh->getVertexAttributes(0, &accessor));
    EXPECT_EQ(accessor.buffer_view, nullptr);
}

// Issue #10495: in DracoCache.cpp getFaceIndices() and convertFaces(), `mesh->num_faces() * 3` was
// computed in 32-bit unsigned arithmetic before widening or multiplying by `sizeof(T)`, and the
// byte size was not checked against UINT32_MAX. Verify both the `numFaces * 3 * sizeof(T)` overflow
// (357,913,942 * 3 * 4 == 0x100000008) and the `numFaces * 3` overflow (1,431,655,766 * 3 ==
// 0x100000002, which wraps to 2 in uint32_t).
TEST_F(glTFIOTest, DracoFaceIndicesSizeOverflowIsRejected) {
    // Case 1: numFaces * 3 fits in uint32_t (1,073,741,826), but numFaces * 3 * sizeof(uint32_t)
    // is 0x100000008 (wraps to 8 in uint32_t).
    {
        constexpr uint32_t kNumFaces = 357913942u;
        auto rawMesh = std::make_unique<draco::Mesh>();
        std::unique_ptr<DracoMesh> dracoMesh(
                DracoMesh::createForTest(rawMesh.release(), kNumFaces));
        ASSERT_NE(dracoMesh, nullptr);

        cgltf_accessor accessor{};
        accessor.type = cgltf_type_scalar;
        accessor.component_type = cgltf_component_type_r_32u;
        accessor.stride = sizeof(uint32_t);
        accessor.count = kNumFaces * 3u;

        EXPECT_FALSE(dracoMesh->getFaceIndices(&accessor));
        EXPECT_EQ(accessor.buffer_view, nullptr);
    }

    // Case 2: numFaces * 3 overflows uint32_t (1,431,655,766 * 3 == 0x100000002, wrapping to 2 in
    // 32-bit arithmetic). Must be rejected both when the accessor declares the wrapped 32-bit
    // count (2) and when it declares the true 64-bit count (4,294,967,298).
    {
        constexpr uint32_t kNumFaces = 1431655766u;
        auto rawMesh = std::make_unique<draco::Mesh>();
        std::unique_ptr<DracoMesh> dracoMesh(
                DracoMesh::createForTest(rawMesh.release(), kNumFaces));
        ASSERT_NE(dracoMesh, nullptr);

        cgltf_accessor accessor{};
        accessor.type = cgltf_type_scalar;
        accessor.component_type = cgltf_component_type_r_16u;
        accessor.stride = sizeof(uint16_t);
        accessor.count = 2u;
        EXPECT_FALSE(dracoMesh->getFaceIndices(&accessor));
        EXPECT_EQ(accessor.buffer_view, nullptr);

        if constexpr (sizeof(cgltf_size) > sizeof(uint32_t)) {
            accessor.count = static_cast<cgltf_size>(kNumFaces) * 3u;
            EXPECT_FALSE(dracoMesh->getFaceIndices(&accessor));
            EXPECT_EQ(accessor.buffer_view, nullptr);
        }
    }
}
#endif

// Issue #10201: when a sparse accessor's indices bufferView uses EXT_meshopt_compression, the
// decoded sparse indices must be validated against the accessor's element count so that an
// out-of-bounds sparse index is rejected rather than writing past the unpacked float buffer.
TEST_F(glTFIOTest, RejectsMeshoptSparseAccessorWithOutOfBoundsIndex) {
    std::vector<uint8_t> const glb = makeMeshoptSparseAccessorOobGlb();

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_FALSE(resourceLoader.loadResources(asset));

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

TEST_F(glTFIOTest, GetEntitiesByPrefix) {
    static char const* const kGltf = R"({
        "asset": { "version": "2.0" },
        "scene": 0,
        "scenes": [{ "nodes": [0, 1, 2, 3, 4] }],
        "nodes": [
            { "name": "prop_chair" },
            { "name": "prop_chair" },
            { "name": "prop_table" },
            { "name": "prop_lamp" },
            { "name": "light_main" }
        ]
    })";

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    FilamentAsset* const asset = assetLoader->createAsset(
            reinterpret_cast<uint8_t const*>(kGltf), uint32_t(std::strlen(kGltf)));
    ASSERT_NE(asset, nullptr);

    // When entities is nullptr, returns the total count of matching entities regardless of maxCount.
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_", nullptr, 0), 4u);
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_", nullptr, 1), 4u);
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_chair", nullptr, 0), 2u);
    EXPECT_EQ(asset->getEntitiesByPrefix("light_", nullptr, 0), 1u);
    EXPECT_EQ(asset->getEntitiesByPrefix("nonexistent_", nullptr, 0), 0u);
    EXPECT_EQ(asset->getEntitiesByPrefix("", nullptr, 0), 5u);

    // When entities is non-null and maxCount is 0, writes nothing and returns 0.
    Entity sentinel{};
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_", &sentinel, 0), 0u);
    EXPECT_TRUE(sentinel.isNull());

    // Retrieve all matching entities using the two-pass count-then-populate pattern.
    size_t const propCount = asset->getEntitiesByPrefix("prop_", nullptr, 0);
    ASSERT_EQ(propCount, 4u);
    std::vector<Entity> props(propCount);
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_", props.data(), props.size()), 4u);
    for (Entity const e : props) {
        EXPECT_FALSE(e.isNull());
        char const* const name = asset->getName(e);
        ASSERT_NE(name, nullptr);
        EXPECT_EQ(std::string_view(name).substr(0, 5), "prop_");
    }

    // Retrieve a capped subset when maxCount is smaller than the number of matching entities.
    Entity partial[2]{};
    EXPECT_EQ(asset->getEntitiesByPrefix("prop_", partial, 2), 2u);
    EXPECT_FALSE(partial[0].isNull());
    EXPECT_FALSE(partial[1].isNull());
    EXPECT_NE(partial[0], partial[1]);

    // Non-matching prefix with non-null output buffer returns 0.
    Entity none[2]{};
    EXPECT_EQ(asset->getEntitiesByPrefix("nonexistent_", none, 2), 0u);
    EXPECT_TRUE(none[0].isNull());

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

TEST_F(glTFIOTest, OutOfBoundsTexCoord) {
    // 1. Direct constrainMaterial() call with out-of-bounds UV indices.
    MaterialKey key = {};
    key.hasBaseColorTexture = true;
    key.baseColorUV = 24;
    key.hasNormalTexture = true;
    key.normalUV = 255;
    key.hasOcclusionTexture = true;
    key.aoUV = 40;

    UvMap uvmap = {};
    constrainMaterial(&key, &uvmap);

    EXPECT_FALSE(key.hasBaseColorTexture);
    EXPECT_EQ(key.baseColorUV, 0u);
    EXPECT_FALSE(key.hasNormalTexture);
    EXPECT_EQ(key.normalUV, 0u);
    EXPECT_FALSE(key.hasOcclusionTexture);
    EXPECT_EQ(key.aoUV, 0u);

    // 2. Loading a glTF asset containing out-of-bounds, negative, and truncated texCoord values.
    const std::string gltfJson = R"({
      "asset": { "version": "2.0" },
      "scenes": [{ "nodes": [0] }],
      "nodes": [{ "mesh": 0 }],
      "meshes": [{ "primitives": [{ "attributes": { "POSITION": 0 }, "material": 0 }] }],
      "accessors": [{
        "type": "VEC3",
        "componentType": 5126,
        "count": 3,
        "bufferView": 0,
        "min": [0, 0, 0],
        "max": [1, 1, 0]
      }],
      "bufferViews": [{ "buffer": 0, "byteLength": 36 }],
      "buffers": [{
        "byteLength": 36,
        "uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAA"
      }],
      "textures": [{}],
      "materials": [{
        "name": "OobMaterial",
        "pbrMetallicRoughness": {
          "baseColorTexture": { "index": 0, "texCoord": 24 },
          "metallicRoughnessTexture": { "index": 0, "texCoord": 256 }
        },
        "normalTexture": { "index": 0, "texCoord": 255 },
        "occlusionTexture": { "index": 0, "texCoord": -1 }
      }]
    })";

    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    FilamentAsset* asset = assetLoader->createAsset(
            reinterpret_cast<const uint8_t*>(gltfJson.data()), gltfJson.size());
    ASSERT_NE(asset, nullptr);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// Returns the local translation of the "animated" node created by makeTranslationAnimationGlb().
static math::float3 getAnimatedNodeTranslation(Engine* engine, FilamentAsset* asset) {
    Entity const node = asset->getFirstEntityByName("animated");
    EXPECT_FALSE(node.isNull());
    TransformManager& tm = engine->getTransformManager();
    return tm.getTransform(tm.getInstance(node))[3].xyz;
}

#define EXPECT_FLOAT3_EQ(actual, expected)              \
    do {                                                \
        math::float3 const a_ = (actual);               \
        math::float3 const e_ = (expected);             \
        EXPECT_FLOAT_EQ(a_.x, e_.x);                    \
        EXPECT_FLOAT_EQ(a_.y, e_.y);                    \
        EXPECT_FLOAT_EQ(a_.z, e_.z);                    \
    } while (0)

// Sanity check for the Animator's sampler import: a valid translation animation must still play
// after the hardening for issues #10134 and #10138.
TEST_F(glTFIOTest, TranslationAnimationPlays) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb = makeTranslationAnimationGlb();
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    ASSERT_TRUE(resourceLoader.loadResources(asset));

    Animator* const animator = asset->getInstance()->getAnimator();
    ASSERT_NE(animator, nullptr);
    ASSERT_EQ(animator->getAnimationCount(), 1u);
    EXPECT_FLOAT_EQ(animator->getAnimationDuration(0), 1.0f);

    // Out-of-range queries must not index past the end of the animation list.
    EXPECT_FLOAT_EQ(animator->getAnimationDuration(1), 0.0f);
    EXPECT_STREQ(animator->getAnimationName(1), "");
    animator->applyAnimation(1, 0.5f);

    animator->applyAnimation(0, 0.5f);
    EXPECT_FLOAT3_EQ(getAnimatedNodeTranslation(mEngine, asset), 0.5f * kTranslationAnimationEnd);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// The timeline used to be read as a packed float array, which ignored the bufferView byteStride.
// With a stride of 8 bytes, that reader would have seen the padding value as the second keyframe
// and reported it as the duration.
TEST_F(glTFIOTest, StridedAnimationTimestampsAreHonored) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb = makeTranslationAnimationGlb({ .timelineByteStride = 8 });
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    ASSERT_TRUE(resourceLoader.loadResources(asset));

    Animator* const animator = asset->getInstance()->getAnimator();
    ASSERT_NE(animator, nullptr);
    ASSERT_EQ(animator->getAnimationCount(), 1u);
    EXPECT_FLOAT_EQ(animator->getAnimationDuration(0), 1.0f);
    EXPECT_NE(animator->getAnimationDuration(0), kTranslationAnimationStridePadding);

    animator->applyAnimation(0, 0.5f);
    EXPECT_FLOAT3_EQ(getAnimatedNodeTranslation(mEngine, asset), 0.5f * kTranslationAnimationEnd);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// Non-finite timestamps are rejected during sampler import, which leaves the animation in the list
// but with no playable channels, so applying it must not move the node. An infinite timestamp used
// to be accepted and reported as the animation's duration. A NaN timestamp would be inserted into
// the std::map that backs the timeline, breaking its strict weak ordering; it was previously
// caught only by accident, because NaN compares equivalent to any existing key and so tripped the
// duplicate-timestamp check.
TEST_F(glTFIOTest, NonFiniteAnimationTimestampsDisableChannel) {
    const float values[] = {
        std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN(),
    };
    for (float const lastTimestamp : values) {
        SCOPED_TRACE(testing::Message() << "lastTimestamp = " << lastTimestamp);

        AssetLoader* assetLoader =
                AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
        ASSERT_NE(assetLoader, nullptr);

        std::vector<uint8_t> const glb =
                makeTranslationAnimationGlb({ .lastTimestamp = lastTimestamp });
        FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
        ASSERT_NE(asset, nullptr);

        // cgltf_validate() does not inspect accessor contents, so the load itself succeeds.
        ResourceLoader resourceLoader({ mEngine, ".", false });
        ASSERT_TRUE(resourceLoader.loadResources(asset));

        Animator* const animator = asset->getInstance()->getAnimator();
        ASSERT_NE(animator, nullptr);
        ASSERT_EQ(animator->getAnimationCount(), 1u);
        EXPECT_FLOAT_EQ(animator->getAnimationDuration(0), 0.0f);

        math::float3 const before = getAnimatedNodeTranslation(mEngine, asset);
        animator->applyAnimation(0, 0.5f);
        EXPECT_FLOAT3_EQ(getAnimatedNodeTranslation(mEngine, asset), before);

        assetLoader->destroyAsset(asset);
        AssetLoader::destroy(&assetLoader);
    }
}

// A buffer that has no URI and is not the GLB BIN chunk survives cgltf_load_buffers() with null
// data, and cgltf_validate() only checks accessors against the declared byteLength. The Animator
// used to read the timeline from that null pointer plus the file-controlled offsets. The same
// situation arises on builds without a file system, where missing external resources are
// tolerated, and with EXT_meshopt_compression fallback buffers.
TEST_F(glTFIOTest, AnimationWithUnloadedBufferIsDisabled) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb =
            makeTranslationAnimationGlb({ .timelineInUnloadedBuffer = true });
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    ASSERT_TRUE(resourceLoader.loadResources(asset));

    Animator* const animator = asset->getInstance()->getAnimator();
    ASSERT_NE(animator, nullptr);
    EXPECT_EQ(animator->getAnimationCount(), 0u);
    EXPECT_FLOAT_EQ(animator->getAnimationDuration(0), 0.0f);

    math::float3 const before = getAnimatedNodeTranslation(mEngine, asset);
    animator->applyAnimation(0, 0.5f);
    EXPECT_FLOAT3_EQ(getAnimatedNodeTranslation(mEngine, asset), before);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// Issue #10134 follow-up: loadResources() used to mark the asset as loaded before validating it,
// so an instance created after a failed load still got an Animator. That Animator would then read
// the animation output accessor, which points past the end of the buffer.
TEST_F(glTFIOTest, FailedLoadDoesNotCreateAnimator) {
    AssetLoader* assetLoader = AssetLoader::create({ mEngine, mMaterialProvider, mNameManager });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb = makeTranslationAnimationGlb(
            { .outputViewByteLength = 4096, .outputAccessorByteOffset = 4000 });
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    EXPECT_FALSE(resourceLoader.loadResources(asset));
    EXPECT_EQ(downcast(asset->getInstance())->mAnimator, nullptr);

    FilamentInstance* const instance = assetLoader->createInstance(asset);
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(downcast(instance)->mAnimator, nullptr);

    // recomputeBoundingBoxes() must not read the source buffers either. It returns early, which
    // leaves the bounding box that the instance copied from the asset untouched.
    Aabb const before = instance->getBoundingBox();
    instance->recomputeBoundingBoxes();
    Aabb const after = instance->getBoundingBox();
    EXPECT_FLOAT3_EQ(after.min, before.min);
    EXPECT_FLOAT3_EQ(after.max, before.max);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

// The extended loader only loads the source buffers for assets that have primitives, so the
// buffers of an animation-only asset are never loaded. The Animator used to read the sampler data
// from the resulting null buffer pointer, offset by the accessor and bufferView offsets.
TEST_F(glTFIOTest, ExtendedAnimationWithoutBufferDataIsDisabled) {
    if (!AssetConfigurationExtended::isSupported()) {
        GTEST_SKIP() << "The extended loader is not supported on this platform.";
    }
    AssetConfigurationExtended ext{ .gltfPath = "." };
    AssetLoader* assetLoader = AssetLoader::create({
            .engine = mEngine, .materials = mMaterialProvider, .names = mNameManager,
            .ext = &ext });
    ASSERT_NE(assetLoader, nullptr);

    std::vector<uint8_t> const glb = makeTranslationAnimationGlb();
    FilamentAsset* const asset = assetLoader->createAsset(glb.data(), uint32_t(glb.size()));
    ASSERT_NE(asset, nullptr);

    ResourceLoader resourceLoader({ mEngine, ".", false });
    ASSERT_TRUE(resourceLoader.loadResources(asset));

    Animator* const animator = asset->getInstance()->getAnimator();
    ASSERT_NE(animator, nullptr);
    EXPECT_EQ(animator->getAnimationCount(), 0u);
    animator->applyAnimation(0, 0.5f);

    assetLoader->destroyAsset(asset);
    AssetLoader::destroy(&assetLoader);
}

#undef EXPECT_FLOAT3_EQ

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
