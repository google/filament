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

#include "ApiHandler.h"

#include <CivetServer.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Log.h>

#include <string>
#include <string_view>

namespace filament::fgviewer {

namespace {
std::string const BASE_URL = "libs/fgviewer/web";

FrameGraphInfoKey getKeybyString(const utils::CString &input,
                                              uint32_t seed) {
    return utils::hash::murmurSlow(reinterpret_cast<uint8_t const*>(
        input.c_str()), input.size(), 0);
}
} // anonymous

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

        if (uri == "/index.html" || uri == "/app.js" || uri == "/api.js") {
            mg_send_file(conn, (BASE_URL + uri).c_str());
            return true;
        }
        slog.e << "fgviewer DebugServer: bad request at line " <<  __LINE__ << ": " << uri << io::endl;
        return false;
    }
private:
    DebugServer* mServer;
};

DebugServer::DebugServer(int port) {
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
        slog.e << "Unable to start fgviewer DebugServer, see civetweb.txt for details." << io::endl;
        return;
    }

    mFileHandler = new FileRequestHandler(this);
    mApiHandler = new ApiHandler(this);

    mServer->addHandler("/api", mApiHandler);
    mServer->addHandler("", mFileHandler);

    slog.i << "fgviewer DebugServer listening at http://localhost:" << port << io::endl;
}

DebugServer::~DebugServer() {
    mServer->close();

    delete mFileHandler;
    delete mApiHandler;
    delete mServer;
}

void DebugServer::addView(const utils::CString &name, FrameGraphInfo info) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    const FrameGraphInfoKey key = getKeybyString(name, 0);
    mViews.insert({key, info});
}

void DebugServer::removeView(const utils::CString& name) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    const FrameGraphInfoKey key = getKeybyString(name, 0);
    mViews.erase(key);
}

void DebugServer::updateView(const utils::CString& name, FrameGraphInfo info) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    const FrameGraphInfoKey key = getKeybyString(name, 0);
    mViews[key] = info;
}

} // namespace filament::fgviewer
