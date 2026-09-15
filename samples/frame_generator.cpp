/*
 * Copyright (C) 2025 The Android Open Source Project
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

/*
 *  This tool loads an object, applies a material, and renders a sequence of frames
 *  while varying material parameters, saving each frame as a PNG image.
 *
 *  The parameters file format is line-based. Each line specifies a parameter to vary:
 *      param_name start_value [end_value]
 *
 *  - param_name: The name of the material parameter.
 *  - start_value: The value at the first frame.
 *  - end_value: (Optional) The value at the last frame. If omitted, defaults to start_value.
 *
 *  Values can be scalars or vectors. Vectors are enclosed in curly braces with comma-separated components.
 *  Examples:
 *      roughness 0.0 1.0
 *      baseColor {0.1, 0.2, 0.3} {1.0, 0.0, 0.0}
 *      clearCoat 1.0
 *
 *  Comments start with '#' or '//'. Empty lines are ignored.
 */

#include "common/arguments.h"
#include "common/SampleConfig.h"

#include <imageio/ImageEncoder.h>

#include <image/ColorTransform.h>
#include <image/LinearImage.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/DesktopAssetLoader.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/IBL.h>
#include <filamentapp/MeshAssimp.h>

#include <filament/Color.h>
#include <filament/Engine.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/getopt.h>
#include <utils/Path.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamat;
using namespace utils;
using namespace image;

namespace {

struct App {
    SampleConfig config;
    FilamentApp2* filamentApp;
};

struct Param {
    utils::CString name;
    std::vector<float> start;
    std::vector<float> end;
};

constexpr int FRAME_TO_SKIP = 10;

std::vector<char> g_materialBuffer;
Path g_materialPath;
Path g_paramsPath;
bool g_lightOn = false;
bool g_skyboxOn = true;
Skybox* g_skybox = nullptr;
int g_materialVariantCount = 1;
int g_currentFrame = 0;
std::atomic_int g_savedFrames(0);
std::vector<Param> g_parameters;
std::vector<Param> g_activeParameters;
utils::CString g_prefix;
uint32_t g_clearColor = 0x000000;
uint32_t g_width = 512;
uint32_t g_height = 512;

std::unique_ptr<MeshAssimp> g_meshSet;
filament::app::AssetLoader* g_assetLoader = nullptr;
std::map<utils::CString, MaterialInstance*> g_meshMaterialInstances;
const Material* g_material = nullptr;
MaterialInstance* g_materialInstance = nullptr;
Entity g_light;

SampleConfig g_config;
float g_meshScale = 1.0f;
FilamentApp2* g_filamentApp = nullptr;

// Prints the usage message to the console.
// Parses command line arguments and populates the SampleConfig object.
// Cleans up Filament resources (entities, materials, etc.) before exit.
void cleanup(Engine* engine, View*, Scene*) {
    for (auto& renderable: g_meshSet->getRenderables()) {
        if (engine->getRenderableManager().getInstance(renderable)) {
            engine->destroy(renderable);
        }
    }

    for (auto const& material : g_meshMaterialInstances) {
        engine->destroy(material.second);
    }

    if (g_skybox) {
        engine->destroy(g_skybox);
    }

    engine->destroy(g_materialInstance);
    engine->destroy(g_material);

    g_meshSet.reset(nullptr);
    g_assetLoader = nullptr;

    engine->destroy(g_light);
    EntityManager& em = EntityManager::get();
    em.destroy(g_light);
}

// Helper function to get the size of a file.
std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

// Reads the compiled material file and creates a Filament Material instance.
void readMaterial(Engine* engine) {
    long const fileSize = getFileSize(g_materialPath.c_str());
    if (fileSize <= 0) {
        return;
    }

    std::ifstream in(g_materialPath.c_str(), std::ifstream::in | std::ios::binary);
    if (in.is_open()) {
        g_materialBuffer.resize(static_cast<unsigned long>(fileSize));
        if (in.read(g_materialBuffer.data(), fileSize)) {
            g_material = Material::Builder()
                    .package(g_materialBuffer.data(), size_t(fileSize))
                    .build(*engine);
            g_materialInstance = g_material->createInstance();
        }
    }
}

std::vector<float> parseFloats(std::istream& stream) {
    std::vector<float> values;
    stream >> std::ws;
    if (stream.peek() == '{') {
        char c;
        stream >> c; // consume '{'
        while (stream.good()) {
            stream >> std::ws;
            if (stream.peek() == '}') {
                stream >> c; // consume '}'
                break;
            }
            float v;
            stream >> v;
            if (!stream.fail()) {
                values.push_back(v);
            }
            stream >> std::ws;
            if (stream.peek() == ',') {
                stream >> c; // consume ','
            }
        }
    } else {
        float v;
        stream >> v;
        if (!stream.fail()) {
            values.push_back(v);
        }
    }
    return values;
}

// Reads the parameters file which defines how material properties change over frames.
void readParameters() {
    std::ifstream in(g_paramsPath.c_str(), std::ifstream::in);
    if (in.is_open()) {
        char line[512];
        while (in.getline(line, sizeof(line))) {
            if (line[0] == '\0' || line[0] == '#' || (line[0] == '/' && line[1] == '/')) continue;
            std::istringstream lineStream(line);
            Param param;
            char tempName[256];
            lineStream >> tempName;
            param.name = utils::CString(tempName);
            param.start = parseFloats(lineStream);
            param.end = parseFloats(lineStream);

            if (param.end.empty()) {
                param.end = param.start;
            }

            if (param.start.size() != param.end.size()) {
                std::cerr << "Error: Parameter " << param.name.c_str_safe()
                          << " has mismatching dimensions: " << param.start.size() << " vs "
                          << param.end.size() << std::endl;
                continue;
            }
            if (param.start.empty() || param.start.size() > 4) {
                std::cerr << "Error: Parameter " << param.name.c_str_safe()
                          << " has invalid dimension: " << param.start.size() << std::endl;
                continue;
            }

            g_parameters.push_back(param);
        }
    }
}

void setParameter(MaterialInstance* mi, const utils::CString& name,
        const std::vector<float>& values) {
    if (values.size() == 1) {
        mi->setParameter(name.c_str(), values[0]);
    } else if (values.size() == 2) {
        mi->setParameter(name.c_str(), float2{values[0], values[1]});
    } else if (values.size() == 3) {
        mi->setParameter(name.c_str(), float3{values[0], values[1], values[2]});
    } else if (values.size() == 4) {
        mi->setParameter(name.c_str(), float4{values[0], values[1], values[2], values[3]});
    }
}

// Sets up the scene: loads mesh, material, lights, and camera.
void setup(Engine* engine, View*, Scene* scene) {
    g_meshSet = std::make_unique<MeshAssimp>(*engine, g_assetLoader);

    readMaterial(engine);
    readParameters();

    if (!g_materialInstance) {
        std::cerr << "The source material " << g_materialPath << " is invalid." << std::endl;
        return;
    }

    for (const auto& fname : g_config.positionalArgs) {
        Path filename(fname.c_str_safe());
        g_meshSet->addFromFile(filename, g_meshMaterialInstances);
    }

    auto& tcm = engine->getTransformManager();
    auto const ei = tcm.getInstance(g_meshSet->getRenderables()[0]);
    tcm.setTransform(ei, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
            tcm.getWorldTransform(ei));

    auto& rcm = engine->getRenderableManager();
    for (auto const renderable : g_meshSet->getRenderables()) {
        auto const instance = rcm.getInstance(renderable);
        if (!instance) continue;

        rcm.setCastShadows(instance, true);

        for (size_t i = 0; i < rcm.getPrimitiveCount(instance); i++) {
            rcm.setMaterialInstanceAt(instance, i, g_materialInstance);
        }

        scene->addEntity(renderable);
    }

    g_light = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::SUN)
            .color(Color::toLinear<ACCURATE>(sRGBColor{0.98f, 0.92f, 0.89f}))
            .intensity(110000.0f)
            .direction({0.6f, -1.0f, -0.8f})
            //.castShadows(true)
            .build(*engine, g_light);

    if (g_lightOn) {
        scene->addEntity(g_light);
    }

    g_activeParameters.clear();
    g_activeParameters.reserve(g_parameters.size());
    for (const auto& p : g_parameters) {
        if (g_material->hasParameter(p.name.c_str())) {
            setParameter(g_materialInstance, p.name, p.start);
            g_activeParameters.push_back(p);
        }
    }

    auto const ibl = g_filamentApp->getIBL();
    if (!ibl || !g_skyboxOn) {
        g_skybox = Skybox::Builder().color({
                float((g_clearColor >> 16) & 0xFF) / 255.0f,
                float((g_clearColor >>  8) & 0xFF) / 255.0f,
                float((g_clearColor      ) & 0xFF) / 255.0f,
                1.0f
        }).build(*engine);
        scene->setSkybox(g_skybox);
    }
}

// Called every frame to update material parameters based on the current frame index.
void render(Engine*, View*, Scene*, Renderer*) {
    int const frame = g_currentFrame - FRAME_TO_SKIP - 1;
    if (frame >= 0 && frame < g_materialVariantCount) {
        float const t = (g_materialVariantCount > 1) ? (float(frame) / float(g_materialVariantCount - 1)) : 0.0f;
        for (auto const& [name, start, end] : g_activeParameters) {
            std::vector<float> current(start.size());
            for (size_t i = 0; i < current.size(); ++i) {
                current[i] = start[i] + t * (end[i] - start[i]);
            }
            setParameter(g_materialInstance, name, current);
        }
    }
}

// Called after rendering to capture the frame and save it as a PNG file.
void postRender(Engine*, View* view, Scene*, Renderer* renderer) {
    int frame = g_currentFrame - FRAME_TO_SKIP - 1;
    // Account for the back buffer
    if (frame >= 1 && frame < g_materialVariantCount + 1) {
        frame -= 1;

        const Viewport& vp = view->getViewport();
        uint8_t const* pixels = new uint8_t[vp.width * vp.height * 3];

        struct CaptureState {
            View* view = nullptr;
            int currentFrame = 0;
        };

        backend::PixelBufferDescriptor buffer(pixels, vp.width * vp.height * 3,
                backend::PixelBufferDescriptor::PixelDataFormat::RGB,
                backend::PixelBufferDescriptor::PixelDataType::UBYTE,
                [](void* buffer, size_t, void* user) {
                    CaptureState const* state = static_cast<CaptureState*>(user);
                    const Viewport& v = state->view->getViewport();

                    LinearImage const image(toLinear<uint8_t>(v.width, v.height, v.width * 3,
                            static_cast<uint8_t*>(buffer)));

                    int const digits = int(log10(double(g_materialVariantCount))) + 1;

                    char nameBuf[512];
                    snprintf(nameBuf, sizeof(nameBuf), "./%s%0*d.png", g_prefix.c_str_safe(),
                            digits, state->currentFrame);
                    utils::CString const name(nameBuf);
                    Path const out(name.c_str());

                    std::ofstream outputStream(out, std::ios::binary | std::ios::trunc);
                    ImageEncoder::encode(outputStream, ImageEncoder::Format::PNG, image, "",
                            name.c_str());

                    delete[] static_cast<uint8_t*>(buffer);
                    delete state;

                    ++g_savedFrames;
                },
                new CaptureState { view, frame }
        );

        renderer->readPixels(
                uint32_t(vp.left), uint32_t(vp.bottom), vp.width, vp.height, std::move(buffer));
    }

    if (g_savedFrames.load() == g_materialVariantCount) {
        g_filamentApp->close();
    }

    g_currentFrame++;
}

} // namespace

// Main entry point: parses args, validates inputs, and runs the Filament application.

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    g_config = config;
    g_assetLoader = loader;
    app->config = config;
    g_meshScale = config.getFloat("scale", 1.0f);
    utils::CString clearColorStr = config.getString("clear-color");
    if (!clearColorStr.empty()) {
        char* end = nullptr;
        g_clearColor = uint32_t(strtoul(clearColorStr.c_str(), &end, 16));
    }
    g_width = uint32_t(config.getInt("size", 512));
    g_height = g_width;
    g_materialPath = config.getString("material").c_str();
    g_paramsPath = config.getString("params").c_str();
    g_prefix = config.getString("prefix");
    g_lightOn = config.getBool("light-on");
    if (config.getBool("skybox-off")) {
        g_skyboxOn = false;
    }
    g_materialVariantCount = config.getInt("count", 1);
    config.width = g_width;
    config.height = g_height;
    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .preRender(render)
                        .postRender(postRender)
                        .build();
    app->filamentApp = fApp.get();
    g_filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeFloat("scale", 's', "Applies uniform scale", 1.0f),
        samples::Parameter::makeString("clear-color", 'b',
                "Clear color for the render target [hex]", ""),
        samples::Parameter::makeInt("size", 'S', "Size of the square render window", 512, 1),
        samples::Parameter::makeString("material", 'm',
                "Path to a compiled material file (see matc)", "", true),
        samples::Parameter::makeString("params", 'p', "Path to a parameters file", "", true),
        samples::Parameter::makeString("prefix", 'P', "Prefix for the rendered output frames", ""),
        samples::Parameter::makeBool("light-on", 'l', "Turn on the directional light", false),
        samples::Parameter::makeBool("skybox-off", 'Y', "Turn off the skybox", false),
        samples::Parameter::makeInt("count", 'C', "Number of material variants to render", 1, 1,
                256),
    };
}

#ifndef __ANDROID__
int main(int const argc, char* argv[]) {
    samples::CommandLineSpecification spec = {
        .sampleDescription =
                "SAMPLE_FRAME_GENERATOR tests a material by varying float parameters\n\n"
                "This tool loads an object, applies the specified material and renders N\n"
                "frames as specified by the -c flag. For each frame rendered, the material\n"
                "parameters are recomputed based on the start and end values specified in the\n"
                "params file (see -p). Each frame is finally saved as a PNG.\n\n"
                "The --params and --material parameters are mandatory.\n\n"
                "Example of a parameters file that varies only the roughness:\n\n"
                "       # default\n"
                "       baseColor  {1,1,1}\n"
                "       metallic   1.0\n"
                "       # interpolated\n"
                "       roughness  0.0 1.0",
        .positionalArgsDescription = { "mesh files (.obj, .fbx)" },
        .requiredPositionalArgCount = 1,
        .parameters = createAppParameters(),
    };

    SampleConfig config;
    samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);

    for (const auto& fname : config.positionalArgs) {
        Path const filename(fname.c_str_safe());
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
    }

    config.title = "Frame Generator";
    config.headless = true;
    auto loader = new filament::app::DesktopAssetLoader();
    auto fApp = createSampleApp(config, dm.get(), loader);
    fApp->run();
    delete loader;

    return 0;
}
#endif
