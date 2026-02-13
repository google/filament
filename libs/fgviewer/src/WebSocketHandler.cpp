/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "WebSocketHandler.h"

#include <fgviewer/DebugServer.h>

#include <utils/Logger.h>

namespace filament::fgviewer {

using namespace utils;

WebSocketHandler::WebSocketHandler(DebugServer* server)
        : mServer(server) {}

bool WebSocketHandler::handleConnection(CivetServer* server, const struct mg_connection* conn) {
    LOG(INFO) << "[fgviewer] WebSocket connected.";
    return true;
}

void WebSocketHandler::handleReadyState(CivetServer* server, struct mg_connection* conn) {
    std::unique_lock<utils::Mutex> lock(mMutex);
    mConnections.insert(conn);
}

bool WebSocketHandler::handleData(CivetServer* server, struct mg_connection* conn, int bits,
        char* data, size_t data_len) {
    // For now, we don't expect any data from the client.
    return true;
}

void WebSocketHandler::handleClose(CivetServer* server, const struct mg_connection* conn) {
    LOG(INFO) << "[fgviewer] WebSocket disconnected.";
    std::unique_lock<utils::Mutex> lock(mMutex);
    mConnections.erase(const_cast<struct mg_connection*>(conn));
}

void WebSocketHandler::broadcast(const char* data, size_t len) {
    std::unique_lock<utils::Mutex> lock(mMutex);
    for (auto* conn: mConnections) {
        mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_BINARY, data, len);
    }
}

} // namespace filament::fgviewer
