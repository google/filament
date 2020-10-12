/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <viewer/SimpleViewer.h>

#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>
#include <filament/LightManager.h>
#include <filament/Material.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <string>
#include <vector>

namespace filament {
namespace viewer {

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

    mSettings.view.shadowType = ShadowType::PCF;
    mSettings.view.dithering = Dithering::TEMPORAL;
    mSettings.view.antiAliasing = AntiAliasing::FXAA;
    mSettings.view.sampleCount = 4;
    mSettings.view.ssao.enabled = true;
    mSettings.view.bloom.enabled = true;

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
    mSettings.view.fog.color = sh3[0];
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

        bool dither = mSettings.view.dithering == Dithering::TEMPORAL;
        ImGui::Checkbox("Dithering", &dither);
        enableDithering(dither);

        bool msaa = mSettings.view.sampleCount != 1;
        ImGui::Checkbox("MSAA 4x", &msaa);
        enableMsaa(msaa);

        ImGui::Checkbox("TAA", &mSettings.view.taa.enabled);

        // this clutters the UI and isn't that useful (except when working on TAA)
        //ImGui::Indent();
        //ImGui::SliderFloat("feedback", &mSettings.view.taa.feedback, 0.0f, 1.0f);
        //ImGui::SliderFloat("filter", &mSettings.view.taa.filterWidth, 0.0f, 2.0f);
        //ImGui::Unindent();

        bool fxaa = mSettings.view.antiAliasing == AntiAliasing::FXAA;
        ImGui::Checkbox("FXAA", &fxaa);
        enableFxaa(fxaa);

        ImGui::Checkbox("SSAO", &mSettings.view.ssao.enabled);
        ImGui::Checkbox("Bloom", &mSettings.view.bloom.enabled);

        if (ImGui::CollapsingHeader("SSAO Options")) {
            auto& ssao = mSettings.view.ssao;

            int quality = (int) ssao.quality;
            int lowpass = (int) ssao.lowPassFilter;
            bool upsampling = ssao.upsampling != View::QualityLevel::LOW;

            ImGui::SliderInt("Quality", &quality, 0, 3);
            ImGui::SliderInt("Low Pass", &lowpass, 0, 2);
            ImGui::Checkbox("High quality upsampling", &upsampling);
            ImGui::SliderFloat("Min Horizon angle", &ssao.minHorizonAngleRad, 0.0f, (float)M_PI_4);

            ssao.upsampling = upsampling ? View::QualityLevel::HIGH : View::QualityLevel::LOW;
            ssao.lowPassFilter = (View::QualityLevel) lowpass;
            ssao.quality = (View::QualityLevel) quality;

            if (ImGui::CollapsingHeader("Dominant Light Shadows (experimental)")) {
                int sampleCount = ssao.ssct.sampleCount;
                ImGui::Checkbox("Enabled##dls", &ssao.ssct.enabled);
                ImGui::SliderFloat("Cone angle", &ssao.ssct.lightConeRad, 0.0f, (float)M_PI_2);
                ImGui::SliderFloat("Shadow Distance", &ssao.ssct.shadowDistance, 0.0f, 10.0f);
                ImGui::SliderFloat("Contact dist max", &ssao.ssct.contactDistanceMax, 0.0f, 100.0f);
                ImGui::SliderFloat("Intensity##dls", &ssao.ssct.intensity, 0.0f, 10.0f);
                ImGui::SliderFloat("Depth bias", &ssao.ssct.depthBias, 0.0f, 1.0f);
                ImGui::SliderFloat("Depth slope bias", &ssao.ssct.depthSlopeBias, 0.0f, 1.0f);
                ImGui::SliderInt("Sample Count", &sampleCount, 1, 32);
                ImGuiExt::DirectionWidget("Direction##dls", ssao.ssct.lightDirection.v);
                ssao.ssct.sampleCount = sampleCount;
            }
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

        bool enableVsm = mSettings.view.shadowType == ShadowType::VSM;
        ImGui::Checkbox("Enable VSM", &enableVsm);
        mSettings.view.shadowType = enableVsm ? ShadowType::VSM : ShadowType::PCF;

        char label[32];
        snprintf(label, 32, "%d", 1 << mVsmMsaaSamplesLog2);
        ImGui::SliderInt("VSM MSAA samples", &mVsmMsaaSamplesLog2, 0, 3, label);

        ImGui::SliderInt("Cascades", &mShadowCascades, 1, 4);
        ImGui::Checkbox("Debug cascades",
                debug.getPropertyAddress<bool>("d.shadowmap.visualize_cascades"));
        ImGui::Checkbox("Enable contact shadows", &mEnableContactShadows);
        ImGui::SliderFloat("Split pos 0", &mSplitPositions[0], 0.0f, 1.0f);
        ImGui::SliderFloat("Split pos 1", &mSplitPositions[1], 0.0f, 1.0f);
        ImGui::SliderFloat("Split pos 2", &mSplitPositions[2], 0.0f, 1.0f);
        ImGui::Unindent();
    }

    if (ImGui::CollapsingHeader("Fog")) {
        ImGui::Indent();
        ImGui::Checkbox("Enable fog", &mSettings.view.fog.enabled);
        ImGui::SliderFloat("Start", &mSettings.view.fog.distance, 0.0f, 100.0f);
        ImGui::SliderFloat("Density", &mSettings.view.fog.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Height", &mSettings.view.fog.height, 0.0f, 100.0f);
        ImGui::SliderFloat("Height falloff", &mSettings.view.fog.heightFalloff, 0.0f, 10.0f);
        ImGui::SliderFloat("Scattering start", &mSettings.view.fog.inScatteringStart, 0.0f, 100.0f);
        ImGui::SliderFloat("Scattering size", &mSettings.view.fog.inScatteringSize, 0.1f, 100.0f);
        ImGui::Checkbox("Color from IBL", &mSettings.view.fog.fogColorFromIbl);
        ImGui::ColorPicker3("Color", mSettings.view.fog.color.v);
        ImGui::Unindent();
    }

    // At this point, all View settings have been modified,
    //  so we can now push them into the Filament View.
    applySettings(mSettings.view, mView);

    if (mEnableSunlight) {
        mScene->addEntity(mSunlight);
        auto sun = lm.getInstance(mSunlight);
        lm.setIntensity(sun, mSunlightIntensity);
        lm.setDirection(sun, normalize(mSunlightDirection));
        lm.setColor(sun, mSunlightColor);
        lm.setShadowCaster(sun, mEnableShadows);
        auto options = lm.getShadowOptions(sun);
        options.vsm.msaaSamples = static_cast<uint8_t>(1u << mVsmMsaaSamplesLog2);
        lm.setShadowOptions(sun, options);
    } else {
        mScene->remove(mSunlight);
    }

    lm.forEachComponent([this, &lm](utils::Entity e, LightManager::Instance ci) {
        auto options = lm.getShadowOptions(ci);
        options.screenSpaceContactShadows = mEnableContactShadows;
        options.shadowCascades = mShadowCascades;
        options.vsm.msaaSamples = static_cast<uint8_t>(1u << mVsmMsaaSamplesLog2);
        std::copy_n(mSplitPositions.begin(), 3, options.cascadeSplitPositions);
        lm.setShadowOptions(ci, options);
        lm.setShadowCaster(ci, mEnableShadows);
    });

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

} // namespace viewer
} // namespace filament
