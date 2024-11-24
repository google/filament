/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <CivetServer.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Log.h>

#include <tsl/robin_set.h>

#include <sstream>
#include <string>
#include <string_view>


namespace filament::fgviewer {

using namespace utils;

DebugServer::DebugServer(int port) {
    // By default the server spawns 50 threads so we override this to 10. According to the civetweb
    // documentation, "it is recommended to use num_threads of at least 5, since browsers often
    /// establish multiple connections to load a single web page, including all linked documents
    // (CSS, JavaScript, images, ...)."  If this count is too small, the web app basically hangs.
    const char* kServerOptions[] = {
        "listening_ports", "8080",
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
        slog.e << "Unable to start DebugServer, see civetweb.txt for details." << io::endl;
        return;
    }

    slog.i << "DebugServer listening at http://localhost:" << port << io::endl;
}

DebugServer::~DebugServer() {
    mServer->close();

    delete mServer;
}

void DebugServer::addView(const utils::CString& name, FrameGraphInfo info) {    
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    mViews.insert({name, info});
}

void DebugServer::removeView(const utils::CString& name) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    mViews.erase(name);
}   

void DebugServer::updateView(const utils::CString& name, FrameGraphInfo info) {
    std::unique_lock<utils::Mutex> lock(mViewsMutex);
    mViews[name] = info;
}   

} // namespace filament::fgviewer
