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

    static constexpr uint32_t FLAG_COLLAPSED = (1 << 0);
    static constexpr int DEFAULT_SIDEBAR_WIDTH = 350;

    /**
     * Constructs a SimpleViewer that has a fixed association with the given Filament objects.
     *
     * Upon construction, the simple viewer may create some additional Filament objects (such as
     * light sources) that it owns.
     */
    SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
            uint32_t flags = 0, int sidebarWidth = DEFAULT_SIDEBAR_WIDTH);

    /**
     * Destroys the SimpleViewer and any Filament entities that it owns.
     */
    ~SimpleViewer();

    /**
     * Sets or changes the asset that is being viewed.
     *
     * This adds all the asset's entities into the scene and optionally transforms the asset to make
     * it fit into a unit cube at the origin. The viewer does not claim ownership over the asset or
     * its entities. Clients should use AssetLoader and ResourceLoader to load an asset before
     * passing it in.
     *
     * @param asset The asset to view.
     * @param scale Adds a transform to the root to fit the asset into a unit cube at the origin.
     */
    void setAsset(FilamentAsset* asset, bool scale);

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
     * Enables depth prepass on the view.
     * Defaults to true.
     */
    void enablePrepass(bool b) { mEnablePrepass = b; }

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
    void enableSSAO(bool b) { mEnableSsao = b; }

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
    float mSunlightIntensity = 100000.0f;
    filament::math::float3 mSunlightColor = filament::Color::toLinear<filament::ACCURATE>({ 0.98, 0.92, 0.89});
    filament::math::float3 mSunlightDirection = {0.6, -1.0, -0.8};
    bool mEnableWireframe = false;
    bool mEnableSunlight = true;
    bool mEnableShadows = true;
    bool mEnableDithering = true;
    bool mEnablePrepass = true;
    bool mEnableFxaa = true;
    bool mEnableMsaa = true;
    bool mEnableSsao = true;
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
    float maxExtent = 0;
    maxExtent = std::max(maxpt.x - minpt.x, maxpt.y - minpt.y);
    maxExtent = std::max(maxExtent, maxpt.z - minpt.z);
    float scaleFactor = 2.0f / maxExtent;
    float3 center = (minpt + maxpt) / 2.0f;
    center.z += 4.0f / scaleFactor;
    return mat4f::scaling(float3(scaleFactor)) * mat4f::translation(-center);
}

SimpleViewer::SimpleViewer(filament::Engine* engine, filament::Scene* scene, filament::View* view,
        uint32_t flags, int sidebarWidth) :
        mEngine(engine), mScene(scene), mView(view),
        mSunlight(utils::EntityManager::get().create()),
        mSidebarWidth(sidebarWidth),
        mFlags(flags) {
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
}

SimpleViewer::~SimpleViewer() {
    mEngine->destroy(mSunlight);
}

void SimpleViewer::setAsset(FilamentAsset* asset, bool scale) {
    if (mAsset == asset) {
        return;
    }
    removeAsset();
    mAsset = asset;
    if (!asset) {
        mAnimator = nullptr;
        return;
    }
    mAnimator = asset->getAnimator();
    if (scale) {
        auto& tcm = mEngine->getTransformManager();
        auto root = tcm.getInstance(mAsset->getRoot());
        filament::math::mat4f transform = fitIntoUnitCube(mAsset->getBoundingBox());
        tcm.setTransform(root, transform);
    }
    mScene->addEntities(mAsset->getEntities(), mAsset->getEntityCount());
}

void SimpleViewer::removeAsset() {
    if (mAsset) {
        const auto begin = mAsset->getEntities();
        const auto end = begin + mAsset->getEntityCount();
        for (auto entity = begin; entity != end; ++entity) {
            mScene->remove(*entity);
        }
    }
    mAsset = nullptr;
    mAnimator = nullptr;
}

void SimpleViewer::setIndirectLight(filament::IndirectLight* ibl,
        filament::math::float3 const* sh3) {
    using namespace filament::math;
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

    ImGuiTreeNodeFlags headerFlags = (mFlags & FLAG_COLLAPSED) ? 0 : ImGuiTreeNodeFlags_DefaultOpen;

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
        size_t numPrims = rm.getPrimitiveCount(instance);
        for (size_t prim = 0; prim < numPrims; ++prim) {
            const Material* mat = rm.getMaterialInstanceAt(instance, prim)->getMaterial();
            const char* mname = mat->getName();
            if (mname) {
                ImGui::Text("prim %zu: material %s", prim, mname);
            } else {
                ImGui::Text("prim %zu: (unnamed material)", prim);
            }
        }
    };

    // Declare a std::function for tree nodes, it's an easy way to make a recursive lambda.
    std::function<void(utils::Entity)> treeNode;

    treeNode = [&](utils::Entity entity) {
        auto tinstance = tm.getInstance(entity);
        auto rinstance = rm.getInstance(entity);
        intptr_t treeNodeId = 1 + entity.getId();

        const char* name = mAsset->getName(entity);
        const char* label = name ? name : (rinstance ? "Mesh" : "Node");

        ImGuiTreeNodeFlags flags = rinstance ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
        std::vector<utils::Entity> children(tm.getChildCount(tinstance));
        if (ImGui::TreeNodeEx((const void*) treeNodeId, flags, "%s", label)) {
            if (rinstance) {
                renderableTreeItem(entity);
            }
            tm.getChildren(tinstance, children.data(), children.size());
            for (auto ce : children) {
                treeNode(ce);
            }
            ImGui::TreePop();
        }
    };

    auto animationsTreeItem = [&]() {
        size_t count = mAnimator->getAnimationCount();
        int selectedAnimation = mCurrentAnimation;
        ImGui::RadioButton("Disable", &selectedAnimation, 0);
        for (size_t i = 0; i < count; ++i) {
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

    if (ImGui::CollapsingHeader("View")) {
        ImGui::Checkbox("Dithering", &mEnableDithering);
        ImGui::Checkbox("Depth prepass", &mEnablePrepass);
        ImGui::Checkbox("FXAA", &mEnableFxaa);
        ImGui::Checkbox("MSAA 4x", &mEnableMsaa);
        ImGui::Checkbox("SSAO", &mEnableSsao);
    }

    mView->setDepthPrepass(
            mEnablePrepass ? View::DepthPrepass::ENABLED : View::DepthPrepass::DISABLED);
    mView->setDithering(mEnableDithering ? View::Dithering::TEMPORAL : View::Dithering::NONE);
    mView->setAntiAliasing(mEnableFxaa ? View::AntiAliasing::FXAA : View::AntiAliasing::NONE);
    mView->setSampleCount(mEnableMsaa ? 4 : 1);
    mView->setAmbientOcclusion(
            mEnableSsao ? View::AmbientOcclusion::SSAO : View::AmbientOcclusion::NONE);

    if (ImGui::CollapsingHeader("Light", headerFlags)) {
        ImGui::SliderFloat("IBL intensity", &mIblIntensity, 0.0f, 100000.0f);
        ImGui::SliderAngle("IBL rotation", &mIblRotation);
        ImGui::SliderFloat("Sun intensity", &mSunlightIntensity, 50000.0, 150000.0f);
        ImGuiExt::DirectionWidget("Sun direction", mSunlightDirection.v);
        ImGui::Checkbox("Enable sunlight", &mEnableSunlight);
        ImGui::Checkbox("Enable shadows", &mEnableShadows);
    }

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

    if (mAsset != nullptr) {
        if (ImGui::CollapsingHeader("Model", headerFlags)) {
            if (mAnimator->getAnimationCount() > 0) {
                intptr_t animationsNodeId = -1;
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
                if (ImGui::TreeNodeEx((const void*) animationsNodeId, flags, "Animations")) {
                    animationsTreeItem();
                    ImGui::TreePop();
                }
            }
            ImGui::Checkbox("Wireframe", &mEnableWireframe);
            treeNode(mAsset->getRoot());
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
