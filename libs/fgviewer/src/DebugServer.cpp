/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <fgviewer/DebugServer.h>
#include <fgviewer/FrameGraphInfo.h>

#include "ApiHandler.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <CivetServer.h>

#include <utils/Logger.h>
#include <utils/Mutex.h>
#include <utils/ostream.h>

#include <mutex>
#include <random>
#include <string>
#include <string_view>


// If set to 0, this serves HTML from a resgen resource. Use 1 only during local development, which
// serves files directly from the source code tree.
#define SERVE_FROM_SOURCE_TREE 0

#if SERVE_FROM_SOURCE_TREE

namespace {
std::string const BASE_URL = "libs/fgviewer/web";

} // anonymous

#else

#include "fgviewer_resources.h"
#include <unordered_map>

namespace {

struct Asset {
    std::string_view mime;
    std::string_view data;
};

std::unordered_map<std::string_view, Asset> ASSET_MAP;

constexpr int READBACK_BUFFER = 16;

struct _Random {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<int> distr;

public:
    _Random()
            : rd(),
              gen(rd()),
              distr(-READBACK_BUFFER / 2, READBACK_BUFFER) {}

    int get() { return distr(gen); }
} randGen;


} // anonymous

#endif // SERVE_FROM_SOURCE_TREE

namespace filament::fgviewer {

using namespace utils;
using PixelDataFormat = DebugServer::PixelDataFormat;

uint8_t getNumberOfComponents(PixelDataFormat format) {
    switch (format) {
        case PixelDataFormat::R:
            return 1;
        case PixelDataFormat::RG:
            return 2;
        case PixelDataFormat::RGB:
            return 3;
        case PixelDataFormat::RGBA:
            return 4;
        case PixelDataFormat::L:
            return 1;
        case PixelDataFormat::LA:
            return 2;
    }
    return 0;
}

std::string_view const DebugServer::kSuccessHeader =
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
        "Connection: close\r\n\r\n";

std::string_view const DebugServer::kErrorHeader =
        "HTTP/1.1 404 Not Found\r\nContent-Type: %s\r\n"
        "Connection: close\r\n\r\n";


class FileRequestHandler : public CivetHandler {
public:
    FileRequestHandler(DebugServer* server) : mServer(server) {}
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        auto const& kSuccessHeader = DebugServer::kSuccessHeader;
        struct mg_request_info const* request = mg_get_request_info(conn);
        std::string uri(request->request_uri);
        if (uri == "/") {
            uri = "/index.html";
        }

#if SERVE_FROM_SOURCE_TREE
        if (uri == "/index.html" || uri == "/app.js" || uri == "/api.js") {
            mg_send_file(conn, (BASE_URL + uri).c_str());
            return true;
        }
#else
        auto const& asset_itr = ASSET_MAP.find(uri);
        if (asset_itr != ASSET_MAP.end()) {
            auto const& mime = asset_itr->second.mime;
            auto const& data = asset_itr->second.data;
            mg_printf(conn, kSuccessHeader.data(), mime.data());
            mg_write(conn, data.data(), data.size());
            return true;
        }
#endif
        LOG(ERROR) << "[fgviewer] DebugServer: bad request at line " <<  __LINE__ << ": " << uri;
        return false;
    }
private:
    DebugServer* mServer;
};

DebugServer::DebugServer(int port, ReadbackRequest request)
        : mReadbackRequest(std::move(request)) {
#if !SERVE_FROM_SOURCE_TREE
    ASSET_MAP["/index.html"] = {
        .mime = "text/html",
        .data = {(char const*) FGVIEWER_RESOURCES_INDEX_DATA},
    };
    ASSET_MAP["/app.js"] = {
        .mime = "text/javascript",
        .data = {(char const*) FGVIEWER_RESOURCES_APP_DATA},
    };
    ASSET_MAP["/api.js"] = {
        .mime = "text/javascript",
        .data = {(char const*) FGVIEWER_RESOURCES_API_DATA},
    };
#endif

    // By default the server spawns 50 threads so we override this to 10. According to the civetweb
    // documentation, "it is recommended to use num_threads of at least 5, since browsers often
    /// establish multiple connections to load a single web page, including all linked documents
    // (CSS, JavaScript, images, ...)."  If this count is too small, the web app basically hangs.
    const char* kServerOptions[] = {
        "listening_ports", "8085",
        "num_threads", "10",
        "error_log_file", "civetweb.txt",
        nullptr
    };
    std::string portString = std::to_string(port);
    kServerOptions[1] = portString.c_str();

    mServer = new CivetServer(kServerOptions);
    if (!mServer->getContext()) {
        delete mServer;
        mServer = nullptr;
        LOG(ERROR) << "[fgviewer] Unable to start DebugServer, see civetweb.txt for details.";
        return;
    }

    mFileHandler = new FileRequestHandler(this);
    mApiHandler = new ApiHandler(this);

    mServer->addHandler("/api", mApiHandler);
    mServer->addHandler("", mFileHandler);

    LOG(INFO) << "[fgviewer] DebugServer listening at http://localhost:" << port;
}

DebugServer::~DebugServer() {
    mServer->close();

    delete mFileHandler;
    delete mApiHandler;
    delete mServer;
}

ViewHandle DebugServer::createView(utils::CString name) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    ViewHandle handle = mViewCounter++;
    mViews.emplace(handle, FrameGraphInfo(std::move(name)));
    mApiHandler->updateFrameGraph(handle);

    return handle;
}

void DebugServer::destroyView(ViewHandle h) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    mViews.erase(h);
}

void DebugServer::update(ViewHandle h, FrameGraphInfo&& info) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    const auto it = mViews.find(h);
    if (it == mViews.end()) {
        LOG(WARNING) << "[fgviewer] Received update for unknown handle " << h;
        return;
    }

    bool const hasChanged = !(it->second == info);
    if (!hasChanged) {
        return;
    }

    mViews.erase(h);
    mViews.emplace(h, std::move(info));
    mApiHandler->updateFrameGraph(h);
}

void DebugServer::tick() {
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    for (auto& [key, monitor]: mMonitoredResources) {
        if (!monitor.enabled || monitor.inProgress || monitor.readbackTick > mTickCount) {
            continue;
        }
        monitor.readbackTick = mTickCount + READBACK_BUFFER + randGen.get();
        monitor.inProgress = true;
        auto name = monitor.name;
        auto viewId = monitor.viewId;
        auto id = monitor.resourceId;
        mReadbackRequest(viewId, id, name,
                [this, key, id, name](PixelBuffer buffer, uint32_t width, uint32_t height,
                        PixelDataFormat format, FormatInfo info) {
                    if (!buffer.data() || width == 0 || height == 0) {
                        std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
                        mMonitoredResources[key].inProgress = false;
                        return;
                    }

                    size_t const pixelCount = width * height;
                    PixelBuffer converted(pixelCount * 4);
                    uint8_t* dst = converted.data();

                    if (info.isDepth) {
                        // Depth Buffer Normalization
                        // Transforms high-precision floating point or integer depth buffers
                        // (e.g. D16, D24, D32F) into a normalized 0-255 spectrum using an automatic
                        // min/max mapping. Needed to make the micro-variations in Z-depth visibly
                        // distinguishable in the UI.
                        std::vector<float> depthFloats(pixelCount);

                        if (info.isD32F) {
                            const float* src = reinterpret_cast<const float*>(buffer.data());
                            for (size_t i = 0; i < pixelCount; ++i) {
                                depthFloats[i] = src[i];
                            }
                        } else if (info.isD16) {
                            const uint16_t* src = reinterpret_cast<const uint16_t*>(buffer.data());
                            for (size_t i = 0; i < pixelCount; ++i) {
                                depthFloats[i] = static_cast<float>(src[i]) / 65535.0f;
                            }
                        } else {
                            const uint32_t* src = reinterpret_cast<const uint32_t*>(buffer.data());
                            for (size_t i = 0; i < pixelCount; ++i) {
                                uint32_t raw = src[i];
                                if (info.isD24) {
                                    uint32_t d24 = raw & 0x00FFFFFF;
                                    depthFloats[i] = static_cast<float>(d24) / 16777215.0f;
                                } else {
                                    depthFloats[i] = static_cast<float>(raw) / 4294967295.0f;
                                }
                            }
                        }

                        float minVal = std::numeric_limits<float>::max();
                        float maxVal = std::numeric_limits<float>::lowest();
                        for (size_t i = 0; i < pixelCount; ++i) {
                            float v = depthFloats[i];
                            if (v < minVal) minVal = v;
                            if (v > maxVal) maxVal = v;
                        }

                        float range = maxVal - minVal;
                        if (range < 1e-6f) range = 1.0f;

                        for (size_t i = 0; i < pixelCount; ++i) {
                            float val = (depthFloats[i] - minVal) / range;
                            val = std::pow(val, 0.5f); // simple gamma curve
                            uint8_t c = static_cast<uint8_t>(std::clamp(val, 0.0f, 1.0f) * 255.0f);
                            dst[i * 4 + 0] = c;
                            dst[i * 4 + 1] = c;
                            dst[i * 4 + 2] = c;
                            dst[i * 4 + 3] = 255;
                        }
                    } else if (info.samples > 1) {
                        // MSAA Downsampling
                        // Converts multi-sampled buffers into standard 1x buffers by skipping over
                        // the extra samples. Needed to prevent the UI image from appearing
                        // stretched/distorted by incorrect pitch.
                        const uint8_t* src = reinterpret_cast<const uint8_t*>(buffer.data());
                        for (size_t i = 0; i < pixelCount; ++i) {
                            size_t srcIdx = i * 4 * info.samples;
                            dst[i * 4 + 0] = src[srcIdx + 0];
                            dst[i * 4 + 1] = src[srcIdx + 1];
                            dst[i * 4 + 2] = src[srcIdx + 2];
                            dst[i * 4 + 3] = src[srcIdx + 3];
                        }
                    } else {
                        if (info.isFloat) {
                            // HDR Tonemapping & Exposure
                            // Applies a basic curve to float color targets (RGB16F, R11G11B10F,
                            // RGB9E5). Needed because raw IEEE float bytes cannot be displayed
                            // natively, and HDR values typically exceed 1.0 intensity which
                            // requires mapping back into sRGB 0-255 boundaries.
                            const float* src = reinterpret_cast<const float*>(buffer.data());
                            for (size_t i = 0; i < pixelCount; ++i) {
                                float r = src[i * 4 + 0];
                                float g = src[i * 4 + 1];
                                float b = src[i * 4 + 2];

                                dst[i * 4 + 0] = static_cast<uint8_t>(
                                        std::pow(std::clamp(r, 0.0f, 1.0f), 1.0f / 2.2f) * 255.0f);
                                dst[i * 4 + 1] = static_cast<uint8_t>(
                                        std::pow(std::clamp(g, 0.0f, 1.0f), 1.0f / 2.2f) * 255.0f);
                                dst[i * 4 + 2] = static_cast<uint8_t>(
                                        std::pow(std::clamp(b, 0.0f, 1.0f), 1.0f / 2.2f) * 255.0f);
                                dst[i * 4 + 3] = 255;
                            }
                        } else if (info.isR8) {
                            // Single-Channel Grayscale Expand
                            // Copies the Red channel into Green and Blue for visualization.
                            // Needed because raw 8-bit monochromatic targets would otherwise be
                            // parsed as standard RGBA by the PNG encoder, resulting in a garbled
                            // red tint or alignment skew.
                            const uint8_t* src = buffer.data();
                            for (size_t i = 0; i < pixelCount * 4; i += 4) {
                                uint8_t r = src[i];
                                dst[i + 0] = r;
                                dst[i + 1] = r;
                                dst[i + 2] = r;
                                dst[i + 3] = 255;
                            }
                        } else {
                            // Standard 8-bit Buffer
                            // Force alpha to 255 (opaque) since UI should visualize the RGB
                            // contents even if the render pass outputs A=0 logic.
                            const uint8_t* src = buffer.data();
                            for (size_t i = 0; i < pixelCount * 4; i += 4) {
                                dst[i + 0] = src[i + 0];
                                dst[i + 1] = src[i + 1];
                                dst[i + 2] = src[i + 2];
                                dst[i + 3] = 255;
                            }
                        }
                    }

                    int const components = getNumberOfComponents(format);
                    int pngSize = 0;
                    unsigned char* pngData = stbi_write_png_to_mem(converted.data(),
                            width * components, width, height, components, &pngSize);

                    if (pngData) {
                        std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
                        mMonitoredResources[key].lastImage.assign(pngData, pngData + pngSize);
                        free(pngData);
                    }

                    {
                        std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
                        mMonitoredResources[key].inProgress = false;
                    }
                });
    }

    mTickCount++;
}

std::vector<uint8_t> DebugServer::getImage(ViewHandle viewId, uint32_t resourceId) const {
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    uint64_t key = ((uint64_t) viewId << 32) | resourceId;
    auto it = mMonitoredResources.find(key);
    if (it != mMonitoredResources.end()) {
        return it->second.lastImage;
    }
    return {};
}

void DebugServer::clearResourceMonitors() {
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    for (auto& [key, monitor] : mMonitoredResources) {
        monitor.enabled = false;
        monitor.inProgress = false;
        monitor.lastImage.clear();
    }
}

std::vector<DebugServer::ActiveMonitor> DebugServer::getMonitoredResources() const {
    std::vector<ActiveMonitor> out;
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    for (auto const& [key, monitor] : mMonitoredResources) {
        if (monitor.enabled) {
            out.push_back({monitor.viewId, monitor.resourceId, monitor.name});
        }
    }
    return out;
}

void DebugServer::setResourceMonitor(ViewHandle viewId, uint32_t resourceId,
        utils::CString const& resourceName, bool enabled) {
    LOG(INFO) << "[fgviewer] DebugServer(" << this << ")::setResourceMonitor: view=" << viewId
              << " id=" << resourceId << ", " << resourceName.c_str() << " -> "
              << (enabled ? "enabled" : "disabled");
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    uint64_t key = ((uint64_t) viewId << 32) | resourceId;
    mMonitoredResources[key].viewId = viewId;
    mMonitoredResources[key].resourceId = resourceId;
    mMonitoredResources[key].enabled = enabled;
    mMonitoredResources[key].name = resourceName;
    if (!enabled) {
        mMonitoredResources[key].inProgress = false;
    }
}

} // namespace filament::fgviewer
