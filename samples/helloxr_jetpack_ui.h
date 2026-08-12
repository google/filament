#ifndef HELLOXR_JETPACK_UI_H
#define HELLOXR_JETPACK_UI_H

#include "helloxr_features.h"

#include <functional>
#include <memory>
#include <vector>

#include <stdint.h>

namespace helloxr {

class JetpackUiLayer {
public:
    struct Submission {
        XrCompositionLayerDepthTestFB depthTest;
        XrCompositionLayerQuad layer;
    };

    static constexpr int32_t PIXEL_WIDTH = 1024;
    static constexpr int32_t PIXEL_HEIGHT = 640;
    static constexpr float WIDTH_METERS = 1.0f;
    static constexpr float HEIGHT_METERS =
            WIDTH_METERS * float(PIXEL_HEIGHT) / float(PIXEL_WIDTH);
    static constexpr float CENTER_X = -1.0f;
    static constexpr float CENTER_Y = -0.2f;
    static constexpr float PLANE_Z = -1.5f;

    enum class TouchAction : int32_t {
        DOWN = 0,
        MOVE = 1,
        UP = 2,
        CANCEL = 3,
    };

    JetpackUiLayer();
    ~JetpackUiLayer();

    JetpackUiLayer(JetpackUiLayer const&) = delete;
    JetpackUiLayer& operator=(JetpackUiLayer const&) = delete;

    void initializeJava(void* javaVm, void* activity);
    void requestExtensions(bool requested,
            std::function<bool(char const*)> const& supports,
            std::vector<char const*>* extensions);
    void configure(uint32_t maximumLayerCount);

    bool initialize(XrInstance instance, XrSession session, XrSpace space,
            bool depthTestSupported);
    void terminate() noexcept;

    XrCompositionLayerBaseHeader const* getLayer(Submission* submission) const noexcept;
    XrPosef getPose() const noexcept;
    void setPose(XrPosef const& pose) noexcept;
    bool isEnabled() const noexcept;
    void injectTouch(float u, float v, TouchAction action) const;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace helloxr

#endif // HELLOXR_JETPACK_UI_H