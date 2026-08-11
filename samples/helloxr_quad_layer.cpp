#include "helloxr_quad_layer.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

namespace helloxr {

using namespace filament;

struct QuadLayer::Impl {
    bool enabled = false;
    uint8_t sampleCount = 1;
    Renderer* renderer = nullptr;
    View* view = nullptr;
    Camera* camera = nullptr;
    utils::Entity cameraEntity;
};

QuadLayer::QuadLayer() : mImpl(std::make_unique<Impl>()) {}

QuadLayer::~QuadLayer() = default;

void QuadLayer::configure(bool const requested, uint32_t const maximumLayerCount,
        uint8_t const projectionSamples, uint8_t const requestedSamples) {
    mImpl->enabled = requested && maximumLayerCount >= 2;
    mImpl->sampleCount = requestedSamples != 0 ? requestedSamples : projectionSamples;
    if (requested && !mImpl->enabled) {
        XRLOG("quad layer disabled: runtime supports only %u composition layer(s)",
                maximumLayerCount);
    }
}

bool QuadLayer::initialize(Engine* engine, Scene* scene, bool const postProcessing,
        bool const transparent) {
    if (!mImpl->enabled) {
        return true;
    }

    mImpl->renderer = engine->createRenderer();
    mImpl->cameraEntity = utils::EntityManager::get().create();
    mImpl->camera = engine->createCamera(mImpl->cameraEntity);
    mImpl->view = engine->createView();
    if (mImpl->renderer == nullptr || mImpl->camera == nullptr || mImpl->view == nullptr) {
        return false;
    }

    mImpl->camera->setProjection(60.0, 1.0, 0.1, 20.0, Camera::Fov::VERTICAL);
    mImpl->camera->lookAt(
            { 1.5f, 0.8f, -1.0f }, { 0.0f, 0.0f, -2.0f }, { 0.0f, 1.0f, 0.0f });

    mImpl->view->setScene(scene);
    mImpl->view->setCamera(mImpl->camera);
    mImpl->view->setViewport({ 0, 0, SIZE, SIZE });
    mImpl->view->setShadowingEnabled(false);
    mImpl->view->setStereoscopicOptions({ .enabled = false });
    mImpl->view->setPostProcessingEnabled(postProcessing);
    mImpl->view->setMultiSampleAntiAliasingOptions({
        .enabled = mImpl->sampleCount > 1,
        .sampleCount = mImpl->sampleCount,
    });
    // The quad is already multi-sampled, so FXAA would add an unnecessary full-screen pass.
    mImpl->view->setAntiAliasing(AntiAliasing::NONE);

    if (transparent) {
        Renderer::ClearOptions const clearOptions{
            .clearColor = { 0.0, 0.0, 0.0, 0.0 },
            .clear = true,
        };
        mImpl->renderer->setClearOptions(clearOptions);
    }

    XRLOG("quad layer: %ux multi-sampling", uint32_t(mImpl->sampleCount));
    return true;
}

void QuadLayer::terminate(Engine* engine) noexcept {
    if (engine == nullptr) {
        return;
    }
    engine->destroy(mImpl->view);
    engine->destroy(mImpl->renderer);
    if (mImpl->camera != nullptr) {
        engine->destroyCameraComponent(mImpl->cameraEntity);
    }
    utils::EntityManager::get().destroy(mImpl->cameraEntity);
    mImpl->view = nullptr;
    mImpl->renderer = nullptr;
    mImpl->camera = nullptr;
    mImpl->cameraEntity = {};
}

bool QuadLayer::render(RenderTarget* target, filament::SwapChain* swapChain, XrSpace space,
        XrSwapchain xrSwapchain, bool const depthTestSupported, Submission* submission) {
    mImpl->view->setRenderTarget(target);
    if (!mImpl->renderer->beginFrame(swapChain)) {
        return false;
    }
    mImpl->renderer->render(mImpl->view);
    mImpl->renderer->endFrame();

    submission->layer = { XR_TYPE_COMPOSITION_LAYER_QUAD };
    submission->layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
    submission->layer.space = space;
    submission->layer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
    submission->layer.subImage.swapchain = xrSwapchain;
    submission->layer.subImage.imageRect = { { 0, 0 }, { int32_t(SIZE), int32_t(SIZE) } };
    submission->layer.subImage.imageArrayIndex = 0;
    submission->layer.pose.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
    submission->layer.pose.position = { -0.7f, 0.0f, -1.5f };
    submission->layer.size = { 0.5f, 0.5f };

    if (depthTestSupported) {
        submission->depthTest = { XR_TYPE_COMPOSITION_LAYER_DEPTH_TEST_FB, nullptr, XR_TRUE,
            XR_COMPARE_OP_LESS_FB };
        submission->layer.next = &submission->depthTest;
    }
    return true;
}

bool QuadLayer::isEnabled() const noexcept {
    return mImpl->enabled;
}

uint8_t QuadLayer::getSampleCount() const noexcept {
    return mImpl->sampleCount;
}

} // namespace helloxr