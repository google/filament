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


#include "ApiHandler.h"

#include <fgviewer/DebugServer.h>
#include <fgviewer/JsonWriter.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>

#include <CivetServer.h>

namespace filament::fgviewer {

using namespace std::chrono_literals;

namespace {

auto const& kSuccessHeader = DebugServer::kSuccessHeader;
auto const& kErrorHeader = DebugServer::kErrorHeader;

auto const error = [](int line, std::string const& uri) {
    utils::slog.e << "[fgviewer] DebugServer: 404 at line " << line << ": " << uri << utils::io::endl;
    return false;
};

} // anonymous

bool ApiHandler::handleGet(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const &uri = request->local_uri;

    if (uri.find("/api/status") == 0) {
        return handleGetStatus(conn, request);
    }

    if (uri == "/api/framegraphs") {
        std::unique_lock const lock(mServer->mViewsMutex);
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "[");
        int index = 0;
        for (auto const& view: mServer->mViews) {
            bool const last = (++index) == mServer->mViews.size();

            JsonWriter writer;
            if (!writer.writeFrameGraphInfo(view.second)) {
                return error(__LINE__, uri);
            }

            mg_printf(conn, "{ \"fgid\": \"%8.8x\", %s } %s", view.first, writer.getJsonString(),
                    last ? "" : ",");
        }
        mg_printf(conn, "]");
        return true;
    }

    if (uri == "/api/framegraph") {
        const FrameGraphInfo* result = getFrameGraphInfo(conn, request);
        if (!result) {
            return error(__LINE__, uri);
        }

        JsonWriter writer;
        if (!writer.writeFrameGraphInfo(*result)) {
            return error(__LINE__, uri);
        }
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "{ %s }", writer.getJsonString());
        return true;
    }

    return error(__LINE__, uri);
}

void ApiHandler::addFrameGraph(ViewHandle view_handle) {
    std::unique_lock const lock(mStatusMutex);
    snprintf(statusFrameGraphId, sizeof(statusFrameGraphId), "%8.8x", view_handle);
    mStatusCondition.notify_all();
}

const FrameGraphInfo* ApiHandler::getFrameGraphInfo(struct mg_connection* conn,
                                         struct mg_request_info const* request) {
    size_t const qlength = strlen(request->query_string);
    char fgid[9] = {};
    if (mg_get_var(request->query_string, qlength, "fgid", fgid, sizeof(fgid)) < 0) {
        return nullptr;
    }
    uint32_t const id = strtoul(fgid, nullptr, 16);
    std::unique_lock const lock(mServer->mViewsMutex);
    const auto it = mServer->mViews.find(id);
    return it == mServer->mViews.end()
        ? nullptr
        : &(it->second);
}

bool ApiHandler::handleGetStatus(struct mg_connection* conn,
                                 struct mg_request_info const* request) {
    char const* qstr = request->query_string;
    if (qstr && strcmp(qstr, "firstTime") == 0) {
        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        mg_write(conn, "0", 1);
        return true;
    }

    std::unique_lock<std::mutex> lock(mStatusMutex);
    uint64_t const currentStatusCount = mCurrentStatus;
    if (mStatusCondition.wait_for(lock, 10s,
                                  [this, currentStatusCount] {
                                      return currentStatusCount < mCurrentStatus;
                                  })) {
        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        mg_write(conn, statusFrameGraphId, 8);
    } else {
        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        // Use '1' to indicate a no-op.  This ensures that we don't block forever if the client is
        // gone.
        mg_write(conn, "1", 1);
    }
    return true;
}

} // filament::fgviewer
