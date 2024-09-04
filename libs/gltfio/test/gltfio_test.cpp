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

#include <gtest/gtest.h>

#include <backend/PixelBufferDescriptor.h>

#include <filament/Engine.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <gltfio/math.h>
#include <math/mathfwd.h>
#include <utils/EntityManager.h>
#include <utils/NameComponentManager.h>
#include <utils/Path.h>

#include "materials/uberarchive.h"

#include <fstream>
#include <unordered_map>

using namespace filament;
using namespace backend;
using namespace gltfio;
using namespace utils;

char const* ANIMATED_MORPH_CUBE_GLB = "AnimatedMorphCube.glb";

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

class glTFData {
public:
    glTFData(Path filename, Engine* engine, MaterialProvider* materialProvider,
            NameComponentManager* nameManager)
        : mAssetLoader(AssetLoader::create({engine, materialProvider, nameManager})),
          mResourceLoader(new ResourceLoader({
                  engine, filename.getAbsolutePath().c_str(), false, /* normalizeSkinningWeights */
          })),
          mStbDecoder(createStbProvider(engine)), mKtxDecoder(createKtx2Provider(engine)) {
        mResourceLoader->addTextureProvider("image/png", mStbDecoder);
        mResourceLoader->addTextureProvider("image/ktx2", mKtxDecoder);

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

        AssetLoader::destroy(&mAssetLoader);
    }

    FilamentAsset* getAsset() const { return mAsset; }

    AssetLoader* mAssetLoader;
    ResourceLoader* mResourceLoader = nullptr;
    TextureProvider* mStbDecoder = nullptr;
    TextureProvider* mKtxDecoder = nullptr;
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

        for (auto fname: {ANIMATED_MORPH_CUBE_GLB}) {
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
