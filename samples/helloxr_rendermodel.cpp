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

        if (!loadFunctions() || !createActions()) {
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
        if (mActionSet == XR_NULL_HANDLE) {
            return;
        }

        XrActiveActionSet activeActionSet = { mActionSet, XR_NULL_PATH };
        XrActionsSyncInfo syncInfo = { XR_TYPE_ACTIONS_SYNC_INFO };
        syncInfo.countActiveActionSets = 1;
        syncInfo.activeActionSets = &activeActionSet;
        if (XR_FAILED(xrSyncActions(mContext.session, &syncInfo))) {
            return;
        }

        auto& tcm = mContext.engine->getTransformManager();
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            Hand& state = mHands[hand];
            if (state.space == XR_NULL_HANDLE) {
                continue;
            }

            XrSpaceLocation location = { XR_TYPE_SPACE_LOCATION };
            constexpr XrSpaceLocationFlags kTracked = XR_SPACE_LOCATION_POSITION_VALID_BIT |
                                                      XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
            bool const tracked =
                    XR_SUCCEEDED(xrLocateSpace(state.space, mContext.appSpace, displayTime,
                            &location)) &&
                    (location.locationFlags & kTracked) == kTracked;

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
            tcm.setTransform(instance, mat4f(poseToMat4(location.pose)));
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
            if (state.space != XR_NULL_HANDLE) {
                xrDestroySpace(state.space);
                state.space = XR_NULL_HANDLE;
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
        if (mPoseAction != XR_NULL_HANDLE) {
            xrDestroyAction(mPoseAction);
            mPoseAction = XR_NULL_HANDLE;
        }
        if (mActionSet != XR_NULL_HANDLE) {
            xrDestroyActionSet(mActionSet);
            mActionSet = XR_NULL_HANDLE;
        }
    }

private:
    struct Hand {
        XrSpace space = XR_NULL_HANDLE;
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

    bool createActions() {
        XrActionSetCreateInfo setInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
        strcpy(setInfo.actionSetName, "helloxr");
        strcpy(setInfo.localizedActionSetName, "helloxr");
        setInfo.priority = 0;
        if (XR_FAILED(xrCreateActionSet(mContext.instance, &setInfo, &mActionSet))) {
            XRLOG("render models: xrCreateActionSet failed");
            return false;
        }

        XrPath handPaths[kHandCount] = {};
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            xrStringToPath(mContext.instance, kHandPaths[hand], &handPaths[hand]);
        }

        XrActionCreateInfo actionInfo = { XR_TYPE_ACTION_CREATE_INFO };
        strcpy(actionInfo.actionName, "grip_pose");
        strcpy(actionInfo.localizedActionName, "Grip pose");
        actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
        actionInfo.countSubactionPaths = kHandCount;
        actionInfo.subactionPaths = handPaths;
        if (XR_FAILED(xrCreateAction(mActionSet, &actionInfo, &mPoseAction))) {
            XRLOG("render models: xrCreateAction failed");
            return false;
        }

        // The Touch profile is what the render models correspond to; a runtime that remaps to some
        // other physical controller still reports poses through these bindings.
        XrPath profile = XR_NULL_PATH;
        xrStringToPath(mContext.instance, "/interaction_profiles/oculus/touch_controller",
                &profile);
        XrActionSuggestedBinding bindings[kHandCount] = {};
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            XrPath binding = XR_NULL_PATH;
            xrStringToPath(mContext.instance, (std::string(kHandPaths[hand]) + "/input/grip/pose")
                                                      .c_str(),
                    &binding);
            bindings[hand] = { mPoseAction, binding };
        }
        XrInteractionProfileSuggestedBinding suggested = {
            XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING
        };
        suggested.interactionProfile = profile;
        suggested.countSuggestedBindings = kHandCount;
        suggested.suggestedBindings = bindings;
        if (XR_FAILED(xrSuggestInteractionProfileBindings(mContext.instance, &suggested))) {
            XRLOG("render models: xrSuggestInteractionProfileBindings failed");
            return false;
        }

        XrSessionActionSetsAttachInfo attachInfo = {
            XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO
        };
        attachInfo.countActionSets = 1;
        attachInfo.actionSets = &mActionSet;
        if (XR_FAILED(xrAttachSessionActionSets(mContext.session, &attachInfo))) {
            XRLOG("render models: xrAttachSessionActionSets failed");
            return false;
        }

        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            XrActionSpaceCreateInfo spaceInfo = { XR_TYPE_ACTION_SPACE_CREATE_INFO };
            spaceInfo.action = mPoseAction;
            spaceInfo.subactionPath = handPaths[hand];
            spaceInfo.poseInActionSpace.orientation.w = 1.0f;
            if (XR_FAILED(xrCreateActionSpace(mContext.session, &spaceInfo,
                        &mHands[hand].space))) {
                XRLOG("render models: xrCreateActionSpace failed for %s", kHandPaths[hand]);
                return false;
            }
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
    XrActionSet mActionSet = XR_NULL_HANDLE;
    XrAction mPoseAction = XR_NULL_HANDLE;
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
