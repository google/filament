/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "fgdbg/DebugServer.h"
#include <CivetServer.h>
#include <utils/Log.h>
#include <tsl/robin_set.h>

using namespace utils;
namespace filament::fgdbg {

using namespace utils;

static const std::string_view kSuccessHeader =
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
        "Connection: close\r\n\r\n";

class RestRequestHandler : public CivetHandler {

public:
    RestRequestHandler() = default;

    bool handleGet(CivetServer *server, struct mg_connection *conn) override {
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "{ RestRequestHandler is working! }");
        return true;
    }
};

class WebSocketHandler : public CivetWebSocketHandler {

public:
    WebSocketHandler() = default;

    bool handleConnection(CivetServer *server, const struct mg_connection *conn) override {
        return true;
    }

    void handleReadyState(CivetServer *server, struct mg_connection *conn) override {
        mConnections.insert(conn);
    }

    void handleClose(CivetServer *server, const struct mg_connection *conn) override {
        auto *key = const_cast<struct mg_connection *>(conn);
        mConnections.erase(key);
    }

    void send(int number) {
        auto string = std::to_string(number);
        for (auto connection : mConnections) {
            mg_websocket_write(connection,
                    MG_WEBSOCKET_OPCODE_TEXT,
                    string.c_str(),
                    string.length());
        }
    }

private:
    tsl::robin_set<struct mg_connection*> mConnections;
};

DebugServer::DebugServer() {
    slog.d << "FrameGraph debug server created" << io::endl;

    //TODO (@feresr): Make port param customizable
    const char* kServerOptions[] = {
            "listening_ports", "8082",
            "num_threads", "10",
            "error_log_file", "civetweb.txt",
            nullptr
    };

    mServer = new CivetServer(kServerOptions);
    if (!mServer->getContext()) {
        delete mServer;
        mServer = nullptr;
        slog.e << "Unable to start fgdbg DebugServer, see civetweb.txt for details." << io::endl;
        return;
    }
    slog.i << "Server started successfully" << io::endl;

    mRestRequestHandler = new RestRequestHandler();
    mWebSocketHandler = new WebSocketHandler();

    mServer->addHandler("", mRestRequestHandler);
    mServer->addWebSocketHandler("", mWebSocketHandler);
}

void DebugServer::sendMessage(int number) {
    mWebSocketHandler->send(number);
}

DebugServer::~DebugServer() {
    slog.d << "FrameGraph debug server destroyed" << io::endl;
    delete mRestRequestHandler;
    delete mWebSocketHandler;
}
}
