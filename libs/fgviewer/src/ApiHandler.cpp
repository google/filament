/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>

#include <CivetServer.h>

#include <sstream>
#include <chrono>

namespace filament::fgviewer {

using namespace std::chrono_literals;

namespace {

auto const& kSuccessHeader = DebugServer::kSuccessHeader;
auto const& kErrorHeader = DebugServer::kErrorHeader;

} // anonymous

static auto const error = [](int line, std::string const& uri) {
    utils::slog.e << "DebugServer: 404 at line " << line << ": " << uri << utils::io::endl;
    return false;
};

bool ApiHandler::handleGetApiFgInfo(struct mg_connection* conn,
        struct mg_request_info const* request) {
    auto const softError = [conn, request](char const* msg) {
        utils::slog.e << "DebugServer: " << msg << ": " << request->query_string << utils::io::endl;
        mg_printf(conn, kErrorHeader.data(), "application/txt");
        mg_write(conn, msg, strlen(msg));
        return true;
    };

    // TODO: Implement the method
    return true;
}

void ApiHandler::addFrameGraph(FrameGraphInfo const* framegraph) {
    // TODO: Implement the method
}


bool ApiHandler::handleGet(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const& uri = request->local_uri;

    // TODO: Implement the method
    return true;
}

} // filament::fgviewer
