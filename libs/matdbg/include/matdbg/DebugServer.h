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

#include <backend/DriverEnums.h>

#include <private/filament/Variant.h>

#include <tsl/robin_map.h>

class CivetServer;

namespace filament {
namespace matdbg {

using MaterialKey = uint32_t;

/**
 * Server-side material debugger.
 *
 * This class manages an HTTP server and a WebSockets server that listen on a secondary thread. It
 * receives material packages from the Filament C++ engine or from a standalone tool such as
 * matinfo.
 */
class DebugServer {
public:
    DebugServer(backend::Backend backend, int port);
    ~DebugServer();

    /**
     * Notifies the debugger that the given material package is being loaded into the engine
     * and returns a unique identifier for the material.
     */
    MaterialKey addMaterial(const utils::CString& name, const void* data, size_t size,
            void* userdata = nullptr);

    /**
     * Notifies the debugger that the given material has been deleted.
     */
    void removeMaterial(MaterialKey key);

    using EditCallback = void(*)(void* userdata, const utils::CString& name, const void*, size_t);
    using QueryCallback = void(*)(void* userdata, VariantList* variants);

    /**
     * Sets up a callback that allows the Filament engine to listen for shader edits. The callback
     * might be triggered from a secondary thread.
     */
    void setEditCallback(EditCallback callback) { mEditCallback = callback; }

    /**
     * Sets up a callback that can ask the Filament engine which shader variants are active. The
     * callback might be triggered from a secondary thread.
     */
    void setQueryCallback(QueryCallback callback) { mQueryCallback = callback; }

    bool isReady() const { return mServer; }

private:
    struct MaterialRecord {
        void* userdata;
        const uint8_t* package;
        size_t packageSize;
        utils::CString name;
        MaterialKey key;
        VariantList activeVariants;
    };

    const MaterialRecord* getRecord(const MaterialKey& key) const;

    void updateActiveVariants();

    /**
     *  Replaces the entire content of a particular shader variant. The given shader index uses the
     *  same ordering that the variants have within the package.
     */
    bool handleEditCommand(const MaterialKey& mat, backend::Backend api, int shaderIndex,
            const char* newShaderContent, size_t newShaderLength);

    const backend::Backend mBackend;

    CivetServer* mServer;
    tsl::robin_map<MaterialKey, MaterialRecord> mMaterialRecords;
    utils::CString mHtml;
    utils::CString mJavascript;
    utils::CString mCss;

    utils::CString mChunkedMessage;
    size_t mChunkedMessageRemaining = 0;

    EditCallback mEditCallback = nullptr;
    QueryCallback mQueryCallback = nullptr;

    class FileRequestHandler* mFileHandler = nullptr;
    class RestRequestHandler* mRestHandler = nullptr;
    class WebSocketHandler* mWebSocketHandler = nullptr;

    friend class FileRequestHandler;
    friend class RestRequestHandler;
    friend class WebSocketHandler;
};

} // namespace matdbg
} // namespace filament

#endif  // MATDBG_DEBUGSERVER_H
