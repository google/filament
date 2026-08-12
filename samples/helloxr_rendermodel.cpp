/*
 * Copyright (C) 2026 The Android Open Source Project
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

// XR_FB_render_model: asks the runtime for the controller models it wants drawn, which arrive as
// glTF binaries, and renders them at the tracked grip poses.

#include "helloxr_features.h"
#include "helloxr_input.h"

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/MaterialProvider.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

// The generated ubershader archive sits at a different place in the install tree than in the build
// tree. Do not try to unify these with an extra -I of the gltfio directory: that puts gltfio/math.h
// on the include path as <math.h>, which libc++'s <cmath> then picks up instead of the C header.
#if defined(__ANDROID__)
#include <gltfio/materials/uberarchive.h>
#else
#include <materials/uberarchive.h>
#endif

#include <utils/Entity.h>

#include <cstring>
#include <string>

using namespace filament;
using namespace filament::math;

namespace helloxr {
namespace {

constexpr uint32_t kHandCount = 2;
constexpr char const* kHandPaths[kHandCount] = { "/user/hand/left", "/user/hand/right" };
constexpr char const* kModelPaths[kHandCount] = {
    "/model_fb/controller/left",
    "/model_fb/controller/right",
};

class RenderModels final : public Feature {
public:
    char const* name() const override { return "render models"; }

    std::vector<char const*> requiredExtensions() const override {
        return { XR_FB_RENDER_MODEL_EXTENSION_NAME };
    }

    bool initialize(FeatureContext const& context) override {
        mContext = context;

        if (mContext.input == nullptr || !loadFunctions()) {
            return false;
        }

        auto* provider = gltfio::createUbershaderProvider(mContext.engine,
                UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);
        mMaterials = provider;
        mAssetLoader = gltfio::AssetLoader::create({ mContext.engine, provider });
        mResourceLoader = new gltfio::ResourceLoader({ mContext.engine });

        // Without a decoder for every mime type the models use, the resource loader leaves the
        // material's baseColorMap unbound and the controller renders solid black. Meta's models
        // arrive as KTX2 through KHR_texture_basisu, so the Basis transcoder is the one that
        // actually matters here.
        mStbDecoder = gltfio::createStbProvider(mContext.engine);
        mKtx2Decoder = gltfio::createKtx2Provider(mContext.engine);
        mResourceLoader->addTextureProvider("image/png", mStbDecoder);
        mResourceLoader->addTextureProvider("image/jpeg", mStbDecoder);
        mResourceLoader->addTextureProvider("image/ktx2", mKtx2Decoder);

        // Models are fetched lazily: a runtime only hands out a model key once the matching
        // controller is actually connected, and controllers come and go during a session.
        return true;
    }

    void update(XrTime displayTime) override {
        auto& tcm = mContext.engine->getTransformManager();
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            Hand& state = mHands[hand];
            XrPosef pose = {};
            bool const tracked = mContext.input->getGripPose(hand, &pose);

            if (tracked && state.asset == nullptr && !state.loadFailed) {
                loadModel(hand);
            }
            if (state.asset == nullptr) {
                continue;
            }

            // Dropping the entities out of the scene is how an untracked controller disappears.
            if (tracked != state.visible) {
                if (tracked) {
                    mContext.scene->addEntities(state.asset->getEntities(),
                            state.asset->getEntityCount());
                } else {
                    mContext.scene->removeEntities(state.asset->getEntities(),
                            state.asset->getEntityCount());
                }
                state.visible = tracked;
                XRLOG("render models: %s controller %s", kHandPaths[hand],
                        tracked ? "tracked, now drawn" : "lost tracking, hidden");
            }
            if (!tracked) {
                continue;
            }

            auto const instance = tcm.getInstance(state.asset->getRoot());
            tcm.setTransform(instance, mat4f(poseToMat4(pose)));
        }
    }

    void terminate() override {
        for (Hand& state: mHands) {
            if (state.asset != nullptr) {
                mContext.scene->removeEntities(state.asset->getEntities(),
                        state.asset->getEntityCount());
                mAssetLoader->destroyAsset(state.asset);
                state.asset = nullptr;
            }
        }
        delete mResourceLoader;
        mResourceLoader = nullptr;
        delete mStbDecoder;
        mStbDecoder = nullptr;
        delete mKtx2Decoder;
        mKtx2Decoder = nullptr;
        if (mAssetLoader != nullptr) {
            gltfio::AssetLoader::destroy(&mAssetLoader);
        }
        if (mMaterials != nullptr) {
            mMaterials->destroyMaterials();
            delete mMaterials;
            mMaterials = nullptr;
        }
    }

private:
    struct Hand {
        gltfio::FilamentAsset* asset = nullptr;
        bool visible = false;
        bool loadFailed = false;
    };

    template<typename Fn>
    bool load(char const* fnName, Fn* out) const {
        return XR_SUCCEEDED(xrGetInstanceProcAddr(mContext.instance, fnName,
                       reinterpret_cast<PFN_xrVoidFunction*>(out))) &&
               *out != nullptr;
    }

    bool loadFunctions() {
        if (!load("xrEnumerateRenderModelPathsFB", &mEnumeratePaths) ||
                !load("xrGetRenderModelPropertiesFB", &mGetProperties) ||
                !load("xrLoadRenderModelFB", &mLoadModel)) {
            XRLOG("render models: XR_FB_render_model entry points unavailable");
            return false;
        }
        return true;
    }

    bool loadModel(uint32_t hand) {
        uint32_t pathCount = 0;
        if (XR_FAILED(mEnumeratePaths(mContext.session, 0, &pathCount, nullptr))) {
            return false;
        }
        std::vector<XrRenderModelPathInfoFB> paths(pathCount,
                { XR_TYPE_RENDER_MODEL_PATH_INFO_FB });
        if (XR_FAILED(mEnumeratePaths(mContext.session, pathCount, &pathCount, paths.data()))) {
            return false;
        }

        XrPath wanted = XR_NULL_PATH;
        xrStringToPath(mContext.instance, kModelPaths[hand], &wanted);
        bool offered = false;
        for (auto const& info: paths) {
            if (info.path == wanted) {
                offered = true;
                break;
            }
        }
        if (!offered) {
            XRLOG("render models: runtime offers no model for %s", kModelPaths[hand]);
            mHands[hand].loadFailed = true;
            return false;
        }

        XrRenderModelCapabilitiesRequestFB capabilities = {
            XR_TYPE_RENDER_MODEL_CAPABILITIES_REQUEST_FB
        };
        capabilities.flags = XR_RENDER_MODEL_SUPPORTS_GLTF_2_0_SUBSET_2_BIT_FB;
        XrRenderModelPropertiesFB properties = { XR_TYPE_RENDER_MODEL_PROPERTIES_FB };
        properties.next = &capabilities;
        if (XR_FAILED(mGetProperties(mContext.session, wanted, &properties)) ||
                properties.modelKey == XR_NULL_RENDER_MODEL_KEY_FB) {
            // Runtimes withhold the key until the physical controller is connected, so keep trying.
            return false;
        }

        XrRenderModelLoadInfoFB loadInfo = { XR_TYPE_RENDER_MODEL_LOAD_INFO_FB };
        loadInfo.modelKey = properties.modelKey;

        XrRenderModelBufferFB buffer = { XR_TYPE_RENDER_MODEL_BUFFER_FB };
        if (XR_FAILED(mLoadModel(mContext.session, &loadInfo, &buffer))) {
            return false;
        }
        std::vector<uint8_t> bytes(buffer.bufferCountOutput);
        buffer.bufferCapacityInput = uint32_t(bytes.size());
        buffer.buffer = bytes.data();
        if (XR_FAILED(mLoadModel(mContext.session, &loadInfo, &buffer))) {
            return false;
        }

        if (!mContext.dumpPrefix.empty()) {
            std::string const path = mContext.dumpPrefix + "_controller_" +
                                     (hand == 0 ? "left" : "right") + ".glb";
            if (FILE* file = fopen(path.c_str(), "wb")) {
                fwrite(bytes.data(), 1, bytes.size(), file);
                fclose(file);
                XRLOG("render models: wrote %s", path.c_str());
            }
        }

        gltfio::FilamentAsset* asset =
                mAssetLoader->createAsset(bytes.data(), uint32_t(bytes.size()));        if (asset == nullptr) {
            XRLOG("render models: could not parse the glTF for %s", kModelPaths[hand]);
            mHands[hand].loadFailed = true;
            return false;
        }
        // Render models embed their buffers and images, so no external resource callbacks needed.
        mResourceLoader->loadResources(asset);
        asset->releaseSourceData();

        mHands[hand].asset = asset;
        XRLOG("render models: %s is '%s' (%u bytes, %zu entities, %zu textures decoded so far)",
                kModelPaths[hand], properties.modelName, uint32_t(bytes.size()),
                size_t(asset->getEntityCount()),
                mStbDecoder->getDecodedCount() + mKtx2Decoder->getDecodedCount());
        return true;
    }

    FeatureContext mContext;
    Hand mHands[kHandCount];

    gltfio::MaterialProvider* mMaterials = nullptr;
    gltfio::AssetLoader* mAssetLoader = nullptr;
    gltfio::ResourceLoader* mResourceLoader = nullptr;
    gltfio::TextureProvider* mStbDecoder = nullptr;
    gltfio::TextureProvider* mKtx2Decoder = nullptr;

    PFN_xrEnumerateRenderModelPathsFB mEnumeratePaths = nullptr;
    PFN_xrGetRenderModelPropertiesFB mGetProperties = nullptr;
    PFN_xrLoadRenderModelFB mLoadModel = nullptr;
};

} // anonymous namespace

std::unique_ptr<Feature> createRenderModels() {
    return std::make_unique<RenderModels>();
}

} // namespace helloxr
