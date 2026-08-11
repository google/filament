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

    XrCompositionLayerBaseHeader const* getLayer() const noexcept;
    bool isEnabled() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace helloxr

#endif // HELLOXR_JETPACK_UI_H