#ifndef HELLOXR_QUAD_LAYER_H
#define HELLOXR_QUAD_LAYER_H

#include "helloxr_features.h"

#include <memory>

#include <stdint.h>

namespace filament {
class Engine;
class RenderTarget;
class Scene;
class SwapChain;
} // namespace filament

namespace helloxr {

class QuadLayer {
public:
    static constexpr uint32_t SIZE = 1024;

    struct Submission {
        XrCompositionLayerDepthTestFB depthTest;
        XrCompositionLayerQuad layer;
    };

    QuadLayer();
    ~QuadLayer();

    QuadLayer(QuadLayer const&) = delete;
    QuadLayer& operator=(QuadLayer const&) = delete;

    void configure(bool requested, uint32_t maximumLayerCount, uint8_t projectionSamples,
            uint8_t requestedSamples);

    bool initialize(filament::Engine* engine, filament::Scene* scene, bool postProcessing,
            bool transparent);
    void terminate(filament::Engine* engine) noexcept;

    bool render(filament::RenderTarget* target, filament::SwapChain* swapChain, XrSpace space,
            XrSwapchain xrSwapchain, bool depthTestSupported, Submission* submission);

    bool isEnabled() const noexcept;
    uint8_t getSampleCount() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace helloxr

#endif // HELLOXR_QUAD_LAYER_H