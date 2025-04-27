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

#include <CivetServer.h>

#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/ostream.h>

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

DebugServer::DebugServer(int port) {
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

    mServer->addHandler("/api", mApiHandler);
    mServer->addHandler("", mFileHandler);

    slog.i << "[fgviewer] DebugServer listening at http://localhost:" << port << io::endl;
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

} // namespace filament::fgviewer
