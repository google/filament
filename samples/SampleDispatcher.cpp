#include "common/SampleConfig.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <samples/SampleDispatcher.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace filament::app;

#define FILAMENT_SAMPLES_LIST(V)                                                                   \
    V(hellotriangle)                                                                               \
    V(hellomorphing)                                                                               \
    V(hellopbr)                                                                                    \
    V(helloskinning)                                                                               \
    V(hellouvmorphing)                                                                             \
    V(procedural_effect)                                                                           \
    V(procedural_texture_quad)     \
    V(rendertarget)                                                                                \
    V(sample_full_pbr)                                                                             \
    V(sample_normal_map)                                                                           \
    V(shadowtest)                                                                                  \
    V(skinningtest)                                                                                \
    V(strobecolor)                                                                                 \
    V(suzanne)                                                                                     \
    V(texturedquad)                                                                                \
    V(vbotest)

// Currently not working samples
//    V(sample_cloth)
//    V(lightbulb)
//

utils::FixedCapacityVector<utils::CString> getSampleNames() {
    return {
#define V(name) utils::CString(#name),
        FILAMENT_SAMPLES_LIST(V)
#undef V
    };
}

#define SAMPLES_CASE(sampleName)                                                                   \
    extern std::unique_ptr<FilamentApp2> create_ ## sampleName ## _App(SampleConfig config,        \
            DisplayManager* dm, filament::app::AssetLoader* loader);                               \
    funcs[#sampleName] = [&]() -> AppPtr { return create_ ## sampleName ## _App(config, dm, loader); };

std::unique_ptr<FilamentApp2> dispatchSample(const utils::CString& name, SampleConfig config,
        DisplayManager* dm, filament::app::AssetLoader* loader) {

    using AppPtr = std::unique_ptr<FilamentApp2>;
    using F = std::function<AppPtr()>;

    std::unordered_map<std::string_view, F> funcs;

#define V(name) SAMPLES_CASE(name)
    FILAMENT_SAMPLES_LIST(V)
#undef V

    if (auto itr = funcs.find(name.c_str()); itr != funcs.end()) {
        return itr->second();
    }
    return nullptr;
}
