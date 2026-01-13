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

#ifndef FGVIEWER_WEBSOCKET_HANDLER_H
#define FGVIEWER_WEBSOCKET_HANDLER_H

#include <CivetServer.h>
#include <utils/Mutex.h>
#include <set>

namespace filament::fgviewer {

class DebugServer;

class WebSocketHandler : public CivetWebSocketHandler {
public:
    explicit WebSocketHandler(DebugServer* server);

    bool handleConnection(CivetServer *server, const struct mg_connection *conn) override;
    void handleReadyState(CivetServer *server, struct mg_connection *conn) override;
    bool handleData(CivetServer *server, struct mg_connection *conn, int bits, char *data, size_t data_len) override;
    void handleClose(CivetServer *server, const struct mg_connection *conn) override;

    void broadcast(const char* data, size_t len);

private:
    DebugServer* mServer;
    utils::Mutex mMutex;
    std::set<struct mg_connection *> mConnections;
};

} // namespace filament::fgviewer

#endif // FGVIEWER_WEBSOCKET_HANDLER_H
