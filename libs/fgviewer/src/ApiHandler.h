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

#ifndef FGVIEWER_APIHANDLER_H
#define FGVIEWER_APIHANDLER_H

#include <fgviewer/DebugServer.h>

#include <CivetServer.h>

namespace filament::fgviewer {

class DebugServer;
struct FrameGraphInfo;

// Handles the following REST requests, where {id} is an 8-digit hex string.
//
//    GET /api/framegraphs
//    GET /api/framegraph?fg={fgid}
//    GET /api/status
//
class ApiHandler : public CivetHandler {
public:
    explicit ApiHandler(DebugServer* server)
        : mServer(server) {}
    ~ApiHandler() = default;

    bool handleGet(CivetServer* server, struct mg_connection* conn);

    void updateFrameGraph(ViewHandle view_handle);

private:
    const FrameGraphInfo* getFrameGraphInfo(struct mg_request_info const* request);

    bool handleGetStatus(struct mg_connection* conn,
                         struct mg_request_info const* request);

    DebugServer* mServer;

    std::mutex mStatusMutex;
    std::condition_variable mStatusCondition;
    char statusFrameGraphId[9] = {};

    // This variable is to implement a *hanging* effect for /api/status. The call to /api/status
    // will always block until statusMaterialId is updated again. The client is expected to keep
    // calling /api/status (a constant "pull" to simulate a push).
    uint64_t mCurrentStatus = 0;
};

} // filament::fgviewer

#endif // FGVIEWER_APIHANDLER_H
