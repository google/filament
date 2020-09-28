/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by mIcable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GLTFIO_SIMPLEVIEWER_H
#define GLTFIO_SIMPLEVIEWER_H

// NOTE: This is an optional header-only utility to avoid a hard dependency on imgui. To use
// SimpleViewer, please add:
//
//      #define GLTFIO_SIMPLEVIEWER_IMPLEMENTATION
//
// to one of your CPP source files to create the implementation. See gltf_viewer.cpp for an example.

#include <filament/Box.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndirectLight.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <gltfio/Animator.h>
#include <gltfio/FilamentAsset.h>

#include <utils/Entity.h>

#include <math/vec3.h>

namespace gltfio {

/**
 * \class SimpleViewer SimpleViewer.h gltfio/SimpleViewer.h
 * \brief Manages the state for a simple glTF viewer with imgui controls and a tree view.
 *
 * This is a utility that can be used across multiple platforms, including web.
 *
 * \note If you don't need ImGui controls, there is no need to use this class, just use AssetLoader
 * instead.
 */
class SimpleViewer {
public:

    static constexpr int DEFAULT_SIDEBAR_WIDTH = 350;

    /**
     * Constructs a SimpleViewer that has a fixed association with the given Filament objects.
     *
     * Upon construction, the simple viewer may create some additional Filament objects (such as
     * light sources) that it owns.
     */
    SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
            int sidebarWidth = DEFAULT_SIDEBAR_WIDTH);

    /**
     * Destroys the SimpleViewer and any Filament entities that it owns.
     */
    ~SimpleViewer();

    /**
     * Adds the asset's ready-to-render entities into the scene and optionally transforms the root
     * node to make it fit into a unit cube at the origin.
     *
     * The viewer does not claim ownership over the asset or its entities. Clients should use
     * AssetLoader and ResourceLoader to load an asset before passing it in.
     *
     * @param asset The asset to view.
     * @param scale Adds a transform to the root to fit the asset into a unit cube at the origin.
     * @param instanceToAnimate Optional instance from which to get the animator.
     */
    void populateScene(FilamentAsset* asset, bool scale,
            FilamentInstance* instanceToAnimate = nullptr);

    /**
     * Removes the current asset from the viewer.
     *
     * This removes all the asset entities from the Scene, but does not destroy them.
     */
    void removeAsset();

    /**
     * Sets or changes the current scene's IBL to allow the UI manipulate it.
     */
    void setIndirectLight(filament::IndirectLight* ibl, filament::math::float3 const* sh3);

    /**
     * Applies the currently-selected glTF animation to the transformation hierarchy and updates
     * the bone matrices on all renderables.
     */
    void applyAnimation(double currentTime);

    /**
     * Constructs ImGui controls for the current frame and responds to everything that the user has
     * changed since the previous frame.
     *
     * If desired this can be used in conjunction with the filagui library, which allows clients to
     * render ImGui controls with Filament.
     */
    void updateUserInterface();

    /**
     * Retrieves the current width of the ImGui "window" which we are using as a sidebar.
     * Clients can monitor this value to adjust the size of the view.
     */
    int getSidebarWidth() const { return mSidebarWidth; }

    /**
     * Allows clients to inject custom UI.
     */
    void setUiCallback(std::function<void()> callback) { mCustomUI = callback; }

    /**
     * Draws the bounding box of each renderable.
     * Defaults to false.
     */
    void enableWireframe(bool b) { mEnableWireframe = b; }

    /**
     * Enables a built-in light source (useful for creating shadows).
     * Defaults to true.
     */
    void enableSunlight(bool b) { mEnableSunlight = b; }

    /**
     * Enables dithering on the view.
     * Defaults to true.
     */
    void enableDithering(bool b) { mEnableDithering = b; }

    /**
     * Enables FXAA antialiasing in the post-process pipeline.
     * Defaults to true.
     */
    void enableFxaa(bool b) { mEnableFxaa = b; }

    /**
     * Enables hardware-based MSAA antialiasing.
     * Defaults to true.
     */
    void enableMsaa(bool b) { mEnableMsaa = b; }

    /**
     * Enables screen-space ambient occlusion in the post-process pipeline.
     * Defaults to true.
     */
    void enableSSAO(bool b) { mSSAOOptions.enabled = b; }

    /**
     * Enables Bloom.
     * Defaults to true.
     */
    void enableBloom(bool bloom) {
        mBloomOptions.enabled = bloom;
    }

    /**
     * Adjusts the intensity of the IBL.
     * See also filament::IndirectLight::setIntensity().
     * Defaults to 30000.0.
     */
    void setIBLIntensity(float brightness) { mIblIntensity = brightness; }

private:
    void updateIndirectLight();

    // Immutable properties set from the constructor.
    filament::Engine* const mEngine;
    filament::Scene* const mScene;
    filament::View* const mView;
    const utils::Entity mSunlight;

    // Properties that can be changed from the application.
    FilamentAsset* mAsset = nullptr;
    Animator* mAnimator = nullptr;
    filament::IndirectLight* mIndirectLight = nullptr;
    std::function<void()> mCustomUI;

    // Properties that can be changed from the UI.
    int mCurrentAnimation = 1;
    bool mResetAnimation = true;
    float mIblIntensity = 30000.0f;
    float mIblRotation = 0.0f;
    float mSunlightIntensity = 100000.0f; // <-- This value is overridden when loading an IBL.
    filament::math::float3 mSunlightColor = filament::Color::toLinear<filament::ACCURATE>({ 0.98, 0.92, 0.89});
    filament::math::float3 mSunlightDirection = {0.6, -1.0, -0.8};
    bool mEnableWireframe = false;
    bool mEnableSunlight = true;
    bool mEnableVsm = false;
    bool mEnableShadows = true;
    int mShadowCascades = 1;
    bool mEnableContactShadows = false;
    std::array<float, 3> mSplitPositions = {0.25f, 0.50f, 0.75f};
    bool mEnableDithering = true;
    bool mEnableFxaa = true;
    bool mEnableMsaa = true;
    filament::View::AmbientOcclusionOptions mSSAOOptions = { .enabled = true };
    filament::View::BloomOptions mBloomOptions = { .enabled = true };
    filament::View::FogOptions mFogOptions = {};
    filament::View::TemporalAntiAliasingOptions mTAAOptions = {};
    int mSidebarWidth;
    uint32_t mFlags;
};

filament::math::mat4f fitIntoUnitCube(const filament::Aabb& bounds);

} // namespace gltfio

#ifdef GLTFIO_SIMPLEVIEWER_IMPLEMENTATION

#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>
#include <filament/Material.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <vector>

namespace gltfio {

filament::math::mat4f fitIntoUnitCube(const filament::Aabb& bounds) {
    using namespace filament::math;
    auto minpt = bounds.min;
    auto maxpt = bounds.max;
    float maxExtent;
    maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
    maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
    float scaleFactor = 2.0f / maxExtent;
    float3 center = (minpt + maxpt) / 2.0f;
    center.z += 4.0f / scaleFactor;
    return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

SimpleViewer::SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
        int sidebarWidth) :
        mEngine(engine), mScene(scene), mView(view),
        mSunlight(utils::EntityManager::get().create()),
        mSidebarWidth(sidebarWidth) {
    using namespace filament;
    LightManager::Builder(LightManager::Type::SUN)
        .color(mSunlightColor)
        .intensity(mSunlightIntensity)
        .direction(normalize(mSunlightDirection))
        .castShadows(true)
        .sunAngularRadius(1.9)
        .sunHaloSize(10.0)
        .sunHaloFalloff(80.0)
        .build(*engine, mSunlight);
    if (mEnableSunlight) {
        mScene->addEntity(mSunlight);
    }
    view->setAmbientOcclusionOptions({ .upsampling = View::QualityLevel::HIGH });
}

SimpleViewer::~SimpleViewer() {
    mEngine->destroy(mSunlight);
}

void SimpleViewer::populateScene(FilamentAsset* asset, bool scale,
        FilamentInstance* instanceToAnimate) {
    if (mAsset != asset) {
        removeAsset();
        mAsset = asset;
        if (!asset) {
            mAnimator = nullptr;
            return;
        }
        mAnimator = instanceToAnimate ? instanceToAnimate->getAnimator() : asset->getAnimator();
        if (scale) {
            auto& tcm = mEngine->getTransformManager();
            auto root = tcm.getInstance(mAsset->getRoot());
            filament::math::mat4f transform = fitIntoUnitCube(mAsset->getBoundingBox());
            tcm.setTransform(root, transform);
        }

        mScene->addEntities(asset->getLightEntities(), asset->getLightEntityCount());
    }

    auto& tcm = mEngine->getRenderableManager();

    static constexpr int kNumAvailable = 128;
    utils::Entity renderables[kNumAvailable];
    while (size_t numWritten = mAsset->popRenderables(renderables, kNumAvailable)) {
        for (size_t i = 0; i < numWritten; i++) {
            auto ri = tcm.getInstance(renderables[i]);
            tcm.setScreenSpaceContactShadows(ri, true);
        }
        mScene->addEntities(renderables, numWritten);
    }
}

void SimpleViewer::removeAsset() {
    if (mAsset) {
        mScene->removeEntities(mAsset->getEntities(), mAsset->getEntityCount());
    }
    mAsset = nullptr;
    mAnimator = nullptr;
}

void SimpleViewer::setIndirectLight(filament::IndirectLight* ibl,
        filament::math::float3 const* sh3) {
    using namespace filament::math;
    mFogOptions.color = sh3[0];
    mIndirectLight = ibl;
    if (ibl) {
        float3 d = filament::IndirectLight::getDirectionEstimate(sh3);
        float4 c = filament::IndirectLight::getColorEstimate(sh3, d);
        mSunlightDirection = d;
        mSunlightColor = c.rgb;
        mSunlightIntensity = c[3] * ibl->getIntensity();
        updateIndirectLight();
    }
}

void SimpleViewer::updateIndirectLight() {
    using namespace filament::math;
    if (mIndirectLight) {
        mIndirectLight->setIntensity(mIblIntensity);
        mIndirectLight->setRotation(mat3f::rotation(mIblRotation, float3{ 0, 1, 0 }));
    }
}

void SimpleViewer::applyAnimation(double currentTime) {
    if (!mAnimator) {
        return;
    }
    static double startTime = 0;
    const size_t numAnimations = mAnimator->getAnimationCount();
    if (mResetAnimation) {
        startTime = currentTime;
        for (size_t i = 0; i < numAnimations; i++) {
            mAnimator->applyAnimation(i, 0);
        }
        mResetAnimation = false;
    }
    if (numAnimations > 0 && mCurrentAnimation > 0) {
        mAnimator->applyAnimation(mCurrentAnimation - 1, currentTime - startTime);
    }
    mAnimator->updateBoneMatrices();
}

void SimpleViewer::updateUserInterface() {
    using namespace filament;

    auto& tm = mEngine->getTransformManager();
    auto& rm = mEngine->getRenderableManager();
    auto& lm = mEngine->getLightManager();

    // Show a common set of UI widgets for all renderables.
    auto renderableTreeItem = [this, &rm](utils::Entity entity) {
        bool rvis = mScene->hasEntity(entity);
        ImGui::Checkbox("visible", &rvis);
        if (rvis) {
            mScene->addEntity(entity);
        } else {
            mScene->remove(entity);
        }
        auto instance = rm.getInstance(entity);
        bool scaster = rm.isShadowCaster(instance);
        ImGui::Checkbox("casts shadows", &scaster);
        rm.setCastShadows(instance, scaster);
        size_t numPrims = rm.getPrimitiveCount(instance);
        for (size_t prim = 0; prim < numPrims; ++prim) {
            const char* mname = rm.getMaterialInstanceAt(instance, prim)->getName();
            if (mname) {
                ImGui::Text("prim %zu: material %s", prim, mname);
            } else {
                ImGui::Text("prim %zu: (unnamed material)", prim);
            }
        }
    };

    auto lightTreeItem = [this, &lm](utils::Entity entity) {
        bool lvis = mScene->hasEntity(entity);
        ImGui::Checkbox("visible", &lvis);

        if (lvis) {
            mScene->addEntity(entity);
        } else {
            mScene->remove(entity);
        }

        auto instance = lm.getInstance(entity);
        bool lcaster = lm.isShadowCaster(instance);
        ImGui::Checkbox("shadow caster", &lcaster);
        lm.setShadowCaster(instance, lcaster);
    };

    // Declare a std::function for tree nodes, it's an easy way to make a recursive lambda.
    std::function<void(utils::Entity)> treeNode;

    treeNode = [&](utils::Entity entity) {
        auto tinstance = tm.getInstance(entity);
        auto rinstance = rm.getInstance(entity);
        auto linstance = lm.getInstance(entity);
        intptr_t treeNodeId = 1 + entity.getId();

        const char* name = mAsset->getName(entity);
        auto getLabel = [&name, &rinstance, &linstance]() {
            if (name) {
                return name;
            }
            if (rinstance) {
                return "Mesh";
            }
            if (linstance) {
                return "Light";
            }
            return "Node";
        };
        const char* label = getLabel();

        ImGuiTreeNodeFlags flags = 0; // rinstance ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
        std::vector<utils::Entity> children(tm.getChildCount(tinstance));
        if (ImGui::TreeNodeEx((const void*) treeNodeId, flags, "%s", label)) {
            if (rinstance) {
                renderableTreeItem(entity);
            }
            if (linstance) {
                lightTreeItem(entity);
            }
            tm.getChildren(tinstance, children.data(), children.size());
            for (auto ce : children) {
                treeNode(ce);
            }
            ImGui::TreePop();
        }
    };

    // Disable rounding and draw a fixed-height ImGui window that looks like a sidebar.
    ImGui::GetStyle().WindowRounding = 0;
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    ImGui::SetNextWindowSize(ImVec2(mSidebarWidth, height), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(20, height), ImVec2(width, height));

    ImGui::Begin("Filament", nullptr, ImGuiWindowFlags_NoTitleBar);
    if (mCustomUI) {
        mCustomUI();
    }

    DebugRegistry& debug = mEngine->getDebugRegistry();

    if (ImGui::CollapsingHeader("View")) {
        ImGui::Indent();
        ImGui::Checkbox("Dithering", &mEnableDithering);
        ImGui::Checkbox("MSAA 4x", &mEnableMsaa);
        ImGui::Checkbox("TAA", &mTAAOptions.enabled);
        // this clutters the UI and isn't that useful (except when working on TAA)
        //ImGui::Indent();
        //ImGui::SliderFloat("feedback", &mTAAOptions.feedback, 0.0f, 1.0f);
        //ImGui::SliderFloat("filter", &mTAAOptions.filterWidth, 0.0f, 2.0f);
        //ImGui::Unindent();
        ImGui::Checkbox("FXAA", &mEnableFxaa);
        ImGui::Checkbox("SSAO", &mSSAOOptions.enabled);
        ImGui::Checkbox("Bloom", &mBloomOptions.enabled);
        if (ImGui::CollapsingHeader("SSAO Options")) {
            int quality = (int) mSSAOOptions.quality;
            bool upsampling = mSSAOOptions.upsampling != View::QualityLevel::LOW;
            ImGui::SliderInt("Quality", &quality, 0, 3);
            ImGui::Checkbox("High quality upsampling", &upsampling);
            ImGui::SliderFloat("Min Horizon angle", &mSSAOOptions.minHorizonAngleRad, 0.0f, (float)M_PI_4);
            mSSAOOptions.upsampling = upsampling ? View::QualityLevel::HIGH : View::QualityLevel::LOW;
            mSSAOOptions.quality = (View::QualityLevel) quality;
        }
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Light")) {
        ImGui::Indent();
        ImGui::SliderFloat("IBL intensity", &mIblIntensity, 0.0f, 100000.0f);
        ImGui::SliderAngle("IBL rotation", &mIblRotation);
        ImGui::SliderFloat("Sun intensity", &mSunlightIntensity, 50000.0, 150000.0f);
        ImGuiExt::DirectionWidget("Sun direction", mSunlightDirection.v);
        ImGui::Checkbox("Enable sunlight", &mEnableSunlight);
        ImGui::Checkbox("Enable shadows", &mEnableShadows);
        ImGui::Checkbox("Enable VSM", &mEnableVsm);
        ImGui::SliderInt("Cascades", &mShadowCascades, 1, 4);
        ImGui::Checkbox("Debug cascades", debug.getPropertyAddress<bool>("d.shadowmap.visualize_cascades"));
        ImGui::Checkbox("Enable contact shadows", &mEnableContactShadows);
        ImGui::SliderFloat("Split pos 0", &mSplitPositions[0], 0.0f, 1.0f);
        ImGui::SliderFloat("Split pos 1", &mSplitPositions[1], 0.0f, 1.0f);
        ImGui::SliderFloat("Split pos 2", &mSplitPositions[2], 0.0f, 1.0f);
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Fog")) {
        ImGui::Indent();
        ImGui::Checkbox("Enable fog", &mFogOptions.enabled);
        ImGui::SliderFloat("Start", &mFogOptions.distance, 0.0f, 100.0f);
        ImGui::SliderFloat("Density", &mFogOptions.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Height", &mFogOptions.height, 0.0f, 100.0f);
        ImGui::SliderFloat("Height falloff", &mFogOptions.heightFalloff, 0.0f, 10.0f);
        ImGui::SliderFloat("Scattering start", &mFogOptions.inScatteringStart, 0.0f, 100.0f);
        ImGui::SliderFloat("Scattering size", &mFogOptions.inScatteringSize, 0.1f, 100.0f);
        ImGui::Checkbox("Color from IBL", &mFogOptions.fogColorFromIbl);
        ImGui::ColorPicker3("Color", mFogOptions.color.v);
        ImGui::Unindent();
    }

    mView->setDithering(mEnableDithering ? View::Dithering::TEMPORAL : View::Dithering::NONE);
    mView->setAntiAliasing(mEnableFxaa ? View::AntiAliasing::FXAA : View::AntiAliasing::NONE);
    mView->setSampleCount(mEnableMsaa ? 4 : 1);
    mView->setAmbientOcclusionOptions(mSSAOOptions);
    mView->setBloomOptions(mBloomOptions);
    mView->setFogOptions(mFogOptions);
    mView->setTemporalAntiAliasingOptions(mTAAOptions);

    if (mEnableSunlight) {
        mScene->addEntity(mSunlight);
        auto sun = lm.getInstance(mSunlight);
        lm.setIntensity(sun, mSunlightIntensity);
        lm.setDirection(sun, normalize(mSunlightDirection));
        lm.setColor(sun, mSunlightColor);
        lm.setShadowCaster(sun, mEnableShadows);
    } else {
        mScene->remove(mSunlight);
    }

    lm.forEachComponent([this, &lm](utils::Entity e, LightManager::Instance ci) {
        auto options = lm.getShadowOptions(ci);
        options.screenSpaceContactShadows = mEnableContactShadows;
        options.shadowCascades = mShadowCascades;
        std::copy_n(mSplitPositions.begin(), 3, options.cascadeSplitPositions);
        lm.setShadowOptions(ci, options);
        lm.setShadowCaster(ci, mEnableShadows);
    });

    mView->setShadowType(mEnableVsm ? View::ShadowType::VSM : View::ShadowType::PCF);

    if (mAsset != nullptr) {
        if (ImGui::CollapsingHeader("Hierarchy")) {
            ImGui::Indent();
            ImGui::Checkbox("Show bounds", &mEnableWireframe);
            treeNode(mAsset->getRoot());
            ImGui::Unindent();
        }

        if (mAnimator->getAnimationCount() > 0 && ImGui::CollapsingHeader("Animation")) {
            ImGui::Indent();
            int selectedAnimation = mCurrentAnimation;
            ImGui::RadioButton("Disable", &selectedAnimation, 0);
            for (size_t i = 0, count = mAnimator->getAnimationCount(); i < count; ++i) {
                std::string label = mAnimator->getAnimationName(i);
                if (label.empty()) {
                    label = "Unnamed " + std::to_string(i);
                }
                ImGui::RadioButton(label.c_str(), &selectedAnimation, i + 1);
            }
            if (selectedAnimation != mCurrentAnimation) {
                mCurrentAnimation = selectedAnimation;
                mResetAnimation = true;
            }
            ImGui::Unindent();
        }

        if (mEnableWireframe) {
            mScene->addEntity(mAsset->getWireframe());
        } else {
            mScene->remove(mAsset->getWireframe());
        }
    }

    mSidebarWidth = ImGui::GetWindowWidth();
    ImGui::End();

    updateIndirectLight();
}

} // namespace gltfio

#endif // GLTFIO_SIMPLEVIEWER_IMPLEMENTATION

#endif // GLTFIO_SIMPLEVIEWER_H
