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
#include "WebSocketHandler.h"

#include <CivetServer.h>


#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/ostream.h>

#include <vector>
#include <fstream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <mutex>
#include <string>
#include <string_view>


// If set to 0, this serves HTML from a resgen resource. Use 1 only during local development, which
// serves files directly from the source code tree.
#define SERVE_FROM_SOURCE_TREE 1

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

} // anonymous

#endif // SERVE_FROM_SOURCE_TREE

namespace filament::fgviewer {

using namespace utils;

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
        slog.e << "[fgviewer] DebugServer: bad request at line " <<  __LINE__ << ": " << uri << io::endl;
        return false;
    }
private:
    DebugServer* mServer;
};

DebugServer::DebugServer(int port, ReadbackRequest&& readbackRequester)
        : mReadbackRequester(std::move(readbackRequester)) {
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
        slog.e << "[fgviewer] Unable to start DebugServer, see civetweb.txt for details." << io::endl;
        return;
    }

    mFileHandler = new FileRequestHandler(this);
    mApiHandler = new ApiHandler(this);
    mWebSocketHandler = new WebSocketHandler(this);

    mServer->addHandler("/api", mApiHandler);
    mServer->addWebSocketHandler("/ws", mWebSocketHandler);
    mServer->addHandler("", mFileHandler);

    slog.i << "[fgviewer] DebugServer listening at http://localhost:" << port << io::endl;
}

DebugServer::~DebugServer() {
    mServer->close();

    delete mFileHandler;
    delete mApiHandler;
    delete mWebSocketHandler;
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

void DebugServer::update(ViewHandle h, FrameGraphInfo info) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    const auto it = mViews.find(h);
    if (it == mViews.end()) {
        slog.w << "[fgviewer] Received update for unknown handle " << h;
        return;
    }

    bool has_changed = !(it->second == info);
    if (!has_changed)
        return;

    mViews.erase(h);
    mViews.emplace(h, std::move(info));
    mApiHandler->updateFrameGraph(h);
}

void DebugServer::startMonitoring(const utils::CString& resourceName) {
    slog.i << "[fgviewer] DebugServer: adding " << resourceName.c_str_safe() << " to monitored list." << io::endl;
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    mMonitoredResources.insert(resourceName);
}

void DebugServer::stopMonitoring(const utils::CString& resourceName) {
    slog.i << "[fgviewer] DebugServer: removing " << resourceName.c_str_safe() << " from monitored list." << io::endl;
    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    mMonitoredResources.erase(resourceName);
}

void DebugServer::broadcast(const char* data, size_t len) {
    mWebSocketHandler->broadcast(data, len);
}

void DebugServer::tick() {
    static constexpr uint32_t UPDATE_INTERVAL = 100; // Update every 30 frames

    mFrameCount++;
    if (mFrameCount % UPDATE_INTERVAL != 0) {
        return;
    }

    std::unique_lock<utils::Mutex> lock(mMonitoredResourcesMutex);
    if (mMonitoredResources.empty()) {
        return;
    }

    // Cycle through monitored resources
    auto it = mMonitoredResources.begin();
    std::advance(it, mCurrentMonitoredResourceIndex % mMonitoredResources.size());
    utils::CString resourceToUpdate = *it;

    slog.i << "[fgviewer] DebugServer: tick triggering readback for " << resourceToUpdate.c_str_safe() << io::endl;

    mCurrentMonitoredResourceIndex++;

    // Request readback from the engine
    mReadbackRequester(resourceToUpdate, 
        [this, resourceName = std::move(resourceToUpdate)](PixelBuffer pixelBuffer, uint32_t width, uint32_t height, PixelDataFormat format) {
            // Encode to PNG and broadcast
            if (pixelBuffer.empty()) {
                slog.w << "[fgviewer] Readback for " << resourceName.c_str_safe() << " failed or returned empty buffer." << io::endl;
                return;
            }

            // For simplicity, convert all to RGBA UBYTE for PNG. More robust format handling
            // would be needed for a production system.
            PixelBuffer rgbaBuffer;
            rgbaBuffer.resize(width * height * 4);

            // Simple conversion, assuming incoming is RGBA UBYTE for now, needs real conversion
            // based on `format` if different formats are supported.
            // For float textures (e.g., R32F, RGBA16F), a more complex tonemapping or scaling
            // would be required before saving to 8-bit PNG.
            // For now, if it's not RGBA UBYTE, we'll just copy, which might result in weird images.
            if (format == Format::RGBA &&
                pixelBuffer.size() == width * height * 4) {
                std::copy(pixelBuffer.begin(), pixelBuffer.end(), rgbaBuffer.begin());
            } else if (format == Format::RGB &&
                       pixelBuffer.size() == width * height * 3) {
                // Convert RGB to RGBA
                for (size_t i = 0; i < width * height; ++i) {
                    rgbaBuffer[i * 4 + 0] = pixelBuffer[i * 3 + 0];
                    rgbaBuffer[i * 4 + 1] = pixelBuffer[i * 3 + 1];
                    rgbaBuffer[i * 4 + 2] = pixelBuffer[i * 3 + 2];
                    rgbaBuffer[i * 4 + 3] = 255; // Alpha
                }
            } else {
                // Fallback for unsupported formats or if dimensions mismatch, fill with red.
                slog.w << "[fgviewer] Unsupported pixel format or size mismatch for PNG conversion: "
                       << (int)format << ", size: " << pixelBuffer.size() << io::endl;
                std::fill(rgbaBuffer.begin(), rgbaBuffer.end(), 255);
            }

            PixelBuffer pngData;
            auto stb_write_to_vector = [](void* context, void* data, int size) {
                auto* buffer = static_cast<PixelBuffer*>(context);
                buffer->insert(buffer->end(), static_cast<uint8_t*>(data),
                               static_cast<uint8_t*>(data) + size);
            };

            int success = stbi_write_png_to_func(stb_write_to_vector, &pngData, (int)width, (int)height, 4, rgbaBuffer.data(), (int)width * 4);
            slog.i << "[fgviewer] stbi_write_png_to_func result: " << success << " (bytes: " << pngData.size() << ")" << io::endl;

            utils::slog.e << "pixel=" << (void*) pixelBuffer.data() << utils::io::endl;
//            auto x = pixelBuffer.data();
//            for (size_t i = 0; i < 16; ++i) {
//                utils::slog.e << "[" << i << "]=" << (int) x[i] << utils::io::endl;
//            }


            if (success && !pngData.empty()) {
                char const* actualName = resourceName.c_str_safe();
                size_t actualNameLen = strlen(actualName);
                {
                    std::ofstream debugFile("/tmp/filament_debug.png", std::ios::binary);
                    if (debugFile.is_open()) {
                        debugFile.write(reinterpret_cast<const char*>(pngData.data()), pngData.size());
                        debugFile.close();
                        slog.i << "[fgviewer] Wrote PNG to /tmp/filament_debug.png" << io::endl;
                    }
                }
                // Prefix with resource name + null terminator to identify the texture on the client side
                std::vector<uint8_t> message;
                message.reserve(actualNameLen + 1 + pngData.size());
                message.insert(message.end(), (uint8_t*)actualName, (uint8_t*)actualName + actualNameLen);
                message.push_back('\0');
                message.insert(message.end(), pngData.begin(), pngData.end());
                utils::slog.e << "sending bytes=" << message.size() <<
                        " resource-size="<< actualNameLen <<  utils::io::endl;
                mWebSocketHandler->broadcast((const char*)message.data(), message.size());
            } else {
                slog.e << "[fgviewer] Failed to encode PNG for " << resourceName.c_str_safe() << io::endl;
            }
        }
    );
}

} // namespace filament::fgviewer
