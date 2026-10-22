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

#include <__chrono/duration.h>
#include <fgviewer/DebugServer.h>
#include <fgviewer/JsonWriter.h>

#include <utils/FixedCapacityVector.h>
#include <utils/Logger.h>
#include <utils/ostream.h>

#include <CivetServer.h>

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string_view>

namespace filament::fgviewer {

using namespace std::chrono_literals;

namespace {

auto const& kSuccessHeader = DebugServer::kSuccessHeader;
auto const& kErrorHeader = DebugServer::kErrorHeader;

auto const error = [](int line, std::string const& uri) {
    LOG(ERROR) << "[fgviewer] DebugServer: 404 at line " << line << ": " << uri;
    return false;
};

} // anonymous

bool ApiHandler::handleGet(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const& uri = request->local_uri;

    if (uri.find("/api/status") == 0) {
        return handleGetStatus(conn, request);
    }

    if (uri.find("/api/image") == 0) {
        return handleGetImage(conn, request);
    }

    if (uri == "/api/monitor") {
        return handleGetMonitor(conn, request);
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
        size_t const qlength = strlen(request->query_string);
        char fgid[9] = {};
        if (mg_get_var(request->query_string, qlength, "fgid", fgid, sizeof(fgid)) < 0) {
            return error(__LINE__, uri);
        }
        uint32_t const id = strtoul(fgid, nullptr, 16);
        std::unique_lock const lock(mServer->mViewsMutex);
        auto const it = mServer->mViews.find(id);
        if (it == mServer->mViews.end()) {
            return error(__LINE__, uri);
        }
        FrameGraphInfo const& result = it->second;
        JsonWriter writer;
        if (!writer.writeFrameGraphInfo(result)) {
            return error(__LINE__, uri);
        }
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "{ %s }", writer.getJsonString());
        return true;
    }

    return error(__LINE__, uri);
}

bool ApiHandler::handlePost(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const& uri = request->local_uri;
    if (uri == "/api/monitor") {
        return handlePostMonitor(conn, request);
    }
    if (uri == "/api/monitor/clear") {
        return handlePostMonitorClear(conn, request);
    }

    return error(__LINE__, uri);
}

bool ApiHandler::handlePostMonitor(struct mg_connection* conn,
        struct mg_request_info const* request) {
    char postData[1024];
    int const postDataLen = mg_read(conn, postData, sizeof(postData) - 1);
    if (postDataLen < 0) {
        LOG(ERROR) << "mg_read failed";
        return error(__LINE__, request->local_uri);
    }
    postData[postDataLen] = '\0';

    // Primitive JSON parsing
    std::string_view body(postData);
    auto fgidPos = body.find("\"fgid\":");
    auto idPos = body.find("\"id\":");
    auto namePos = body.find("\"name\":");
    auto enabledPos = body.find("\"enabled\":");
    if (idPos == std::string_view::npos || enabledPos == std::string_view::npos) {
        LOG(ERROR) << "JSON parsing failed for id/enabled. body=" << body;
        return error(__LINE__, request->local_uri);
    }

    uint32_t fgid = 0;
    if (fgidPos != std::string_view::npos) {
        auto fgidStart = body.find("\"", fgidPos + 7);
        if (fgidStart != std::string_view::npos) {
            fgidStart += 1;
            auto fgidEnd = body.find("\"", fgidStart);
            if (fgidEnd != std::string_view::npos) {
                auto sv = body.substr(fgidStart, fgidEnd - fgidStart);
                try {
                    fgid = std::stoul(std::string(sv), nullptr, 16);
                } catch (...) {
                }
            }
        }
    }

    uint32_t id = 0;
    auto idStart = idPos + 4;
    while (idStart < body.length() && (body[idStart] == ' ' || body[idStart] == ':')) {
        idStart++;
    }
    auto idEnd = body.find(",", idStart);
    if (idEnd == std::string_view::npos) idEnd = body.find("}", idStart);
    try {
        auto sv = body.substr(idStart, idEnd - idStart);
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), id);
        if (ec != std::errc()) {
            LOG(ERROR) << "from_chars failed for id. sv=" << sv;
        }
    } catch (...) {
        LOG(ERROR) << "ID extraction threw an exception";
        return error(__LINE__, request->local_uri);
    }

    std::string_view name = "Unknown";
    if (namePos != std::string_view::npos) {
        auto nameStart = body.find("\"", namePos + 7);
        if (nameStart != std::string_view::npos) {
            nameStart++;
            auto nameEnd = body.find("\"", nameStart);
            if (nameEnd != std::string_view::npos) {
                name = body.substr(nameStart, nameEnd - nameStart);
            }
        }
    }

    bool const enabled = body.find("true", enabledPos) != std::string_view::npos;

    LOG(INFO) << "Monitor updated: fgid=" << fgid << " id=" << id << " name=" << name
              << " enabled=" << enabled;
    mServer->setResourceMonitor(fgid, id, utils::CString(name.data(), name.length()), enabled);
    mg_printf(conn, kSuccessHeader.data(), "application/json");
    mg_printf(conn, "{ \"status\": \"ok\" }");
    return true;
}

bool ApiHandler::handlePostMonitorClear(struct mg_connection* conn,
                                        struct mg_request_info const* request) {
    mServer->clearResourceMonitors();
    mg_printf(conn, kSuccessHeader.data(), "application/json");
    mg_printf(conn, "{ \"status\": \"ok\" }");
    return true;
}

bool ApiHandler::handleGetMonitor(struct mg_connection* conn,
                                  struct mg_request_info const* request) {
    auto const resources = mServer->getMonitoredResources();
    mg_printf(conn, kSuccessHeader.data(), "application/json");
    mg_printf(conn, "[");
    bool first = true;
    for (auto const& res : resources) {
        if (!first) mg_printf(conn, ",");
        mg_printf(conn, "{\"fgid\":%u, \"id\":%u, \"name\":\"%s\"}",
                  res.viewId, res.resourceId, res.name.c_str());
        first = false;
    }
    mg_printf(conn, "]");
    return true;
}

void ApiHandler::updateFrameGraph(ViewHandle view) {
    std::unique_lock const lock(mStatusMutex);
    snprintf(statusFrameGraphId, sizeof(statusFrameGraphId), "%8.8x", view);
    mCurrentStatus++;
    mStatusCondition.notify_all();
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

bool ApiHandler::handleGetImage(struct mg_connection* conn, struct mg_request_info const* request) {
    char const* qstr = request->query_string;
    if (!qstr) return false;

    uint32_t fgid = 0;
    uint32_t id = 0;

    size_t const qlength = strlen(qstr);

    char fgid_str[16] = {};
    if (mg_get_var(qstr, qlength, "fgid", fgid_str, sizeof(fgid_str)) > 0) {
        fgid = static_cast<uint32_t>(std::strtoul(fgid_str, nullptr, 16));
    }

    char id_str[16] = {};
    if (mg_get_var(qstr, qlength, "id", id_str, sizeof(id_str)) > 0) {
        id = static_cast<uint32_t>(std::strtoul(id_str, nullptr, 10));
    }

    std::vector<uint8_t> pngData = mServer->getImage(fgid, id);
    if (!pngData.empty()) {
        mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: image/png\r\n"
                "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                "Content-Length: %zu\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Connection: close\r\n"
                "\r\n",
                pngData.size());
        mg_write(conn, pngData.data(), pngData.size());
    } else {
        // Return an empty transparent 1x1 PNG or a 404
        mg_printf(conn, "HTTP/1.1 404 Not Found\r\n"
                        "Content-Length: 0\r\n"
                        "Connection: close\r\n"
                        "\r\n");
    }

    return true;
}

} // filament::fgviewer
