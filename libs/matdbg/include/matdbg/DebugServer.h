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

#ifndef MATDBG_DEBUGSERVER_H
#define MATDBG_DEBUGSERVER_H

#include <utils/CString.h>

#include <tsl/robin_map.h>

class CivetServer;

namespace filament {
namespace matdbg {

enum ServerMode { ENGINE, STANDALONE };

/**
 * Server-side material debugger.
 *
 * This class manages an HTTP server and a WebSockets server that listen on a secondary thread. It
 * receives material packages from the Filament C++ engine or from a standalone tool such as
 * matinfo.
 */
class DebugServer {
public:
    DebugServer(ServerMode mode, int port = 8080);
    ~DebugServer();

    /**
     * Notifies the debugger that the given material package is being loaded into the engine.
     */
    void addMaterial(const utils::CString& name, const void* data, size_t size,
            void* userdata = nullptr);

    using EditCallback = void(*)(void* userdata, const utils::CString& name, const void*, size_t);

    /**
     * Sets up a callback that allows the Filament engine to listen for shader edits. The callback
     * might be triggered from a secondary thread.
     */
    void setEditCallback(EditCallback callback) { mEditCallback = callback; }

private:
    using MaterialKey = uint32_t;

    struct MaterialRecord {
        void* userdata;
        uint8_t* package;
        size_t packageSize;
        utils::CString name;
        MaterialKey key;
    };

    const MaterialRecord* getRecord(const MaterialKey& key) const;

    const ServerMode mServerMode;
    CivetServer* mServer;
    tsl::robin_map<MaterialKey, MaterialRecord> mMaterialRecords;
    utils::CString mHtml;
    utils::CString mJavascript;
    utils::CString mCss;
    EditCallback mEditCallback = nullptr;

    class FileRequestHandler* mFileHandler;
    class RestRequestHandler* mRestHandler;
    class WebSocketHandler* mWebSocketHandler;

    friend class FileRequestHandler;
    friend class RestRequestHandler;
    friend class WebSocketHandler;
};

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_DEBUGSERVER_H
