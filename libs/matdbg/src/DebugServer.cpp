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

#include <matdbg/DebugServer.h>

#include <CivetServer.h>

#include <utils/Hash.h>
#include <utils/Log.h>

#include <spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>

#include <matdbg/ShaderReplacer.h>
#include <matdbg/ShaderExtractor.h>
#include <matdbg/ShaderInfo.h>
#include <matdbg/JsonWriter.h>

#include <filaflat/ChunkContainer.h>

#include <backend/DriverEnums.h>

#include <sstream>
#include <string>

// If set to 0, this serves HTML from a resgen resource. Use 1 only during local development, which
// serves files directly from the source code tree.
#define SERVE_FROM_SOURCE_TREE 0

#if !SERVE_FROM_SOURCE_TREE
#include "matdbg_resources.h"
#endif

namespace filament {
namespace matdbg {

using namespace utils;
using namespace filament::backend;

using filaflat::ChunkContainer;
using filamat::ChunkType;

static const StaticString kSuccessHeader =
        "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n"
        "Connection: close\r\n\r\n";

class FileRequestHandler : public CivetHandler {
public:
    FileRequestHandler(DebugServer* server) : mServer(server) {}
    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        const struct mg_request_info* request = mg_get_request_info(conn);
        std::string uri(request->request_uri);
        if (uri == "/" || uri == "/index.html") {
            #if SERVE_FROM_SOURCE_TREE
            mg_send_file(conn, "libs/matdbg/web/index.html");
            #else
            mg_printf(conn, kSuccessHeader.c_str(), "text/html");
            mg_write(conn, mServer->mHtml.c_str(), mServer->mHtml.size());
            #endif
            return true;
        }
        if (uri == "/style.css") {
            #if SERVE_FROM_SOURCE_TREE
            mg_send_file(conn, "libs/matdbg/web/style.css");
            #else
            mg_printf(conn, kSuccessHeader.c_str(), "text/css");
            mg_write(conn, mServer->mCss.c_str(), mServer->mCss.size());
            #endif
            return true;
        }
        if (uri == "/script.js") {
            #if SERVE_FROM_SOURCE_TREE
            mg_send_file(conn, "libs/matdbg/web/script.js");
            #else
            mg_printf(conn, kSuccessHeader.c_str(), "text/javascript");
            mg_write(conn, mServer->mJavascript.c_str(), mServer->mJavascript.size());
            #endif
            return true;
        }
        slog.e << "DebugServer: bad request at line " <<  __LINE__ << ": " << uri << io::endl;
        return false;
    }
private:
    DebugServer* mServer;
};

// Handles the following REST requests, where {id} is an 8-digit hex string.
//
//    GET /api/matids
//    GET /api/materials
//    GET /api/material?matid={id}
//    GET /api/shader?matid={id}&type=[glsl|spirv]&[glindex|vkindex|metalindex]={index}
//    GET /api/active
//
class RestRequestHandler : public CivetHandler {
public:
    RestRequestHandler(DebugServer* server) : mServer(server) {}

    bool handleGet(CivetServer *server, struct mg_connection *conn) {
        const struct mg_request_info* request = mg_get_request_info(conn);
        std::string uri(request->local_uri);

        const auto error = [request](int line) {
            slog.e << "DebugServer: 404 at " <<  line << ": " << request->query_string << io::endl;
            return false;
        };

        if (uri == "/api/active") {
            mServer->updateActiveVariants();
            mg_printf(conn, kSuccessHeader.c_str(), "application/json");
            mg_printf(conn, "{");
            int index = 0;
            for (const auto& pair : mServer->mMaterialRecords) {
                const auto& record = pair.second;
                ChunkContainer package(record.package, record.packageSize);
                if (!package.parse()) {
                    return error(__LINE__);
                }
                JsonWriter writer;
                if (!writer.writeActiveInfo(package, mServer->mBackend, record.activeVariants)) {
                    return error(__LINE__);
                }
                const bool last = (++index) == mServer->mMaterialRecords.size();
                mg_printf(conn, "\"%8.8x\": %s %s", pair.first, writer.getJsonString(),
                        last ? "" : ",");
            }
            mg_printf(conn, "}");
            return true;
        }

        if (uri == "/api/matids") {
            mg_printf(conn, kSuccessHeader.c_str(), "application/json");
            mg_printf(conn, "[");
            int index = 0;
            for (const auto& record : mServer->mMaterialRecords) {
                const bool last = (++index) == mServer->mMaterialRecords.size();
                mg_printf(conn, "\"%8.8x\" %s", record.first, last ? "" : ",");
            }
            mg_printf(conn, "]");
            return true;
        }

        if (uri == "/api/materials") {
            mg_printf(conn, kSuccessHeader.c_str(), "application/json");
            mg_printf(conn, "[");
            int index = 0;
            for (const auto& record : mServer->mMaterialRecords) {
                const bool last = (++index) == mServer->mMaterialRecords.size();

                ChunkContainer package(record.second.package, record.second.packageSize);
                if (!package.parse()) {
                    return error(__LINE__);
                }

                JsonWriter writer;
                if (!writer.writeMaterialInfo(package)) {
                    return error(__LINE__);
                }

                mg_printf(conn, "{ \"matid\": \"%8.8x\", %s } %s", record.first,
                        writer.getJsonString(), last ? "" : ",");
            }
            mg_printf(conn, "]");
            return true;
        }

        if (!request->query_string) {
            return error(__LINE__);
        }

        const size_t qlength = strlen(request->query_string);
        char matid[9] = {};
        if (mg_get_var(request->query_string, qlength, "matid", matid, sizeof(matid)) < 0) {
            return error(__LINE__);
        }
        const uint32_t id = strtol(matid, nullptr, 16);
        const DebugServer::MaterialRecord* result = mServer->getRecord(id);
        if (result == nullptr) {
            return error(__LINE__);
        }

        ChunkContainer package(result->package, result->packageSize);
        if (!package.parse()) {
            return error(__LINE__);
        }

        if (uri == "/api/material") {
            JsonWriter writer;
            if (!writer.writeMaterialInfo(package)) {
                return error(__LINE__);
            }
            mg_printf(conn, kSuccessHeader.c_str(), "application/json");
            mg_printf(conn, "{ %s }", writer.getJsonString());
            return true;
        }

        char type[6] = {};
        if (mg_get_var(request->query_string, qlength, "type", type, sizeof(type)) < 0) {
            return error(__LINE__);
        }

        char glindex[4] = {};
        char vkindex[4] = {};
        char metalindex[4] = {};
        mg_get_var(request->query_string, qlength, "glindex", glindex, sizeof(glindex));
        mg_get_var(request->query_string, qlength, "vkindex", vkindex, sizeof(vkindex));
        mg_get_var(request->query_string, qlength, "metalindex", metalindex, sizeof(metalindex));

        if (!glindex[0] && !vkindex[0] && !metalindex[0]) {
            return error(__LINE__);
        }

        if (uri != "/api/shader") {
            return error(__LINE__);
        }

        if (glindex[0]) {
            std::vector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialGlsl));
            if (!getGlShaderInfo(package, info.data())) {
                return error(__LINE__);
            }

            const int shaderIndex = std::stoi(glindex);
            if (shaderIndex >= info.size()) {
                return error(__LINE__);
            }

            ShaderExtractor extractor(Backend::OPENGL, result->package, result->packageSize);
            if (!extractor.parse()) {
                return error(__LINE__);
            }

            const auto& item = info[shaderIndex];
            filaflat::ShaderBuilder builder;
            extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);

            mg_printf(conn, kSuccessHeader.c_str(), "application/txt");
            mg_write(conn, builder.data(), builder.size() - 1);
            return true;
        }

        if (vkindex[0]) {
            ShaderExtractor extractor(Backend::VULKAN, result->package, result->packageSize);
            if (!extractor.parse()) {
                return error(__LINE__);
            }

            filaflat::ShaderBuilder builder;
            std::vector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialSpirv));
            if (!getVkShaderInfo(package, info.data())) {
                return error(__LINE__);
            }

            const int shaderIndex = std::stoi(vkindex);
            if (shaderIndex >= info.size()) {
                return error(__LINE__);
            }

            const auto& item = info[shaderIndex];
            extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);

            // TODO: Add a transpiler that depends on "type" and add an MSL type instead of
            // piggybacking on type=GLSL.

            mg_printf(conn, kSuccessHeader.c_str(), "application/text");

            if (true) {
                auto context = spvContextCreate(SPV_ENV_UNIVERSAL_1_0);
                spv_text text = nullptr;
                const uint32_t options = SPV_BINARY_TO_TEXT_OPTION_INDENT |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
                const uint32_t* data = (const uint32_t*) builder.data();
                spvBinaryToText(context, data, builder.size() / 4, options, &text, nullptr);

                mg_write(conn, text->str, text->length);
                spvTextDestroy(text);
                spvContextDestroy(context);
            }

            return true;
        }

        if (metalindex[0]) {
            ShaderExtractor extractor(Backend::METAL, result->package, result->packageSize);
            if (!extractor.parse()) {
                return error(__LINE__);
            }

            filaflat::ShaderBuilder builder;
            std::vector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialMetal));
            if (!getMetalShaderInfo(package, info.data())) {
                return error(__LINE__);
            }

            const int shaderIndex = std::stoi(metalindex);
            if (shaderIndex >= info.size()) {
                return error(__LINE__);
            }

            const auto& item = info[shaderIndex];
            extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, builder);

            mg_printf(conn, kSuccessHeader.c_str(), "application/txt");
            mg_write(conn, builder.data(), builder.size() - 1);
            return true;
        }

        return error(__LINE__);
    }

private:
    DebugServer* mServer;
};

class WebSocketHandler : public CivetWebSocketHandler {
public:
    WebSocketHandler(DebugServer* server) : mServer(server) {}

    bool handleConnection(CivetServer *server, const struct mg_connection *conn) override {
        return true;
    }

    void handleReadyState(CivetServer *server, struct mg_connection *conn) override {
        mConnection = conn;
    }

    bool handleData(CivetServer *server, struct mg_connection *conn, int bits, char *data,
            size_t size) override {

        // First check if this chunk is a continuation of a partial existing message.
        if (mServer->mChunkedMessageRemaining > 0) {
            const CString chunk(data, size);
            const size_t pos = mServer->mChunkedMessage.size();

            // Append the partial existing message.
            mServer->mChunkedMessage = mServer->mChunkedMessage.insert(pos, chunk);

            // Determine number of outstanding bytes.
            if (size > mServer->mChunkedMessageRemaining) {
                mServer->mChunkedMessageRemaining = 0;
            } else {
                mServer->mChunkedMessageRemaining -= size;
            }

            // Return early and wait for more chunks if some bytes are still outstanding.
            if (mServer->mChunkedMessageRemaining > 0) {
                return true;
            }

            data = mServer->mChunkedMessage.data();
            size = mServer->mChunkedMessage.size();

        // Ignore the handshake message that occurs after startup.
        } else if (size < 8) {
            return true;
        }

        mServer->mChunkedMessageRemaining = 0;

        // Every WebSocket message is prefixed with a command name followed by a space.
        //
        // For now we simply use istringstream for parsing, so command arguments are delimited
        // with space characters.
        //
        // The "API index" matches the values of filament::backend::Backend (zero is invalid).
        //
        // The "shader index" is a zero-based index into the list of variants using the order that
        // they appear in the package, where each API (GL / VK / Metal) has its own list.
        //
        // Commands:
        //
        //     EDIT [material id] [api index] [shader index] [shader length] [shader source....]
        //

        const static StaticString kEditCmd = "EDIT ";
        const static size_t kEditCmdLength = kEditCmd.size();

        if (0 == strncmp(data, kEditCmd.c_str(), kEditCmdLength)) {
            std::istringstream str(data + kEditCmdLength);
            uint32_t matid;
            int api;
            int shaderIndex;
            int shaderLength;
            str >> std::hex >> matid >> std::dec >> api >> shaderIndex >> shaderLength;
            const char* source = data + kEditCmdLength + str.tellg() + 1;
            const size_t remaining = size - kEditCmdLength - str.tellg();

            // Return early and wait for more chunks if some bytes are still outstanding.
            if (remaining < shaderLength + 1) {
                mServer->mChunkedMessage = CString(data, size);
                mServer->mChunkedMessageRemaining = shaderLength + 1 - remaining;
                return true;
            }

            mServer->handleEditCommand(matid, backend::Backend(api), shaderIndex, source,
                    shaderLength);
            return true;
        }

        const std::string firstFewChars(data, std::min(size, size_t(8)));
        slog.e << "Bad WebSocket message. First few characters: "
               << "[" << firstFewChars << "]" << io::endl;
        return false;
    }

    void handleClose(CivetServer *server, const struct mg_connection *conn) override {
        mConnection = nullptr;
    }

    // Notify the JavaScript client that a new material package has been loaded.
    void notify(const DebugServer::MaterialRecord& material) {
        if (mConnection) {
            char matid[9] = {};
            sprintf(matid, "%8.8x", material.key);
            mg_websocket_write(mConnection, MG_WEBSOCKET_OPCODE_TEXT, matid, 8);
        }
    }

private:
    DebugServer* mServer;
    struct mg_connection* mConnection = nullptr;
};

DebugServer::DebugServer(Backend backend, int port) : mBackend(backend) {
    #if !SERVE_FROM_SOURCE_TREE
    mHtml = CString((const char*) MATDBG_RESOURCES_INDEX_DATA, MATDBG_RESOURCES_INDEX_SIZE - 1);
    mJavascript = CString((const char*) MATDBG_RESOURCES_SCRIPT_DATA, MATDBG_RESOURCES_SCRIPT_SIZE - 1);
    mCss = CString((const char*) MATDBG_RESOURCES_STYLE_DATA, MATDBG_RESOURCES_STYLE_SIZE - 1);
    #endif

    // By default the server spawns 50 threads so we override this to 2. This limits the server
    // to having no more than 2 HTTP clients, which is perfectly fine for debugging purposes.
    const char* kServerOptions[] = {
        "listening_ports", "8080",
        "num_threads", "2",
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

    mFileHandler = new FileRequestHandler(this);
    mRestHandler = new RestRequestHandler(this);
    mWebSocketHandler = new WebSocketHandler(this);

    mServer->addHandler("/api", mRestHandler);
    mServer->addHandler("", mFileHandler);
    mServer->addWebSocketHandler("", mWebSocketHandler);

    slog.i << "DebugServer listening at http://localhost:" << port << io::endl;
}

DebugServer::~DebugServer() {
    for (auto& pair : mMaterialRecords) {
        delete [] pair.second.package;
    }
    delete mFileHandler;
    delete mRestHandler;
    delete mServer;
}

void DebugServer::addMaterial(const CString& name, const void* data, size_t size, void* userdata) {
    filaflat::ChunkContainer* container = new filaflat::ChunkContainer(data, size);
    if (!container->parse()) {
        slog.e << "DebugServer: unable to parse material package: " << name.c_str() << io::endl;
        return;
    }

    const uint32_t seed = 42;
    auto words = (const uint32_t*) data;
    MaterialKey key = utils::hash::murmur3(words, size / 4, seed);

    // Retain a copy of the package to permit queries after the client application has
    // freed up the original material package.
    uint8_t* package = new uint8_t[size];
    memcpy(package, data, size);

    MaterialRecord info = {userdata, package, size, name, key};
    mMaterialRecords.insert({key, info});
    mWebSocketHandler->notify(info);
}

const DebugServer::MaterialRecord* DebugServer::getRecord(const MaterialKey& key) const {
    const auto& iter = mMaterialRecords.find(key);
    return iter == mMaterialRecords.end() ? nullptr : &iter->second;
}

void DebugServer::updateActiveVariants() {
    if (mQueryCallback) {
        for (auto& pair : mMaterialRecords) {
            uint64_t& result = mMaterialRecords[pair.first].activeVariants;
            mQueryCallback(pair.second.userdata, &result);
        }
    }
}

bool DebugServer::handleEditCommand(const MaterialKey& key, backend::Backend api, int shaderIndex,
            const char* source, size_t size) {
    const auto error = [](int line) {
        slog.e << "DebugServer: Unable to apply shader edit at line " << line << io::endl;
        return false;
    };

    if (mMaterialRecords.find(key) == mMaterialRecords.end()) {
        return error(__LINE__);
    }
    MaterialRecord& material = mMaterialRecords[key];
    filaflat::ChunkContainer package(material.package, material.packageSize);
    if (!package.parse()) {
        return error(__LINE__);
    }

    std::vector<ShaderInfo> infos;
    size_t shaderCount;
    switch (api) {
        case backend::Backend::OPENGL: {
            shaderCount = getShaderCount(package, ChunkType::MaterialGlsl);
            infos.resize(shaderCount);
            if (!getGlShaderInfo(package, infos.data())) {
                return error(__LINE__);
            }
            break;
        }
        case backend::Backend::VULKAN: {
            shaderCount = getShaderCount(package, ChunkType::MaterialSpirv);
            infos.resize(shaderCount);
            if (!getVkShaderInfo(package, infos.data())) {
                return error(__LINE__);
            }
            break;
        }
        case backend::Backend::METAL: {
            shaderCount = getShaderCount(package, ChunkType::MaterialMetal);
            infos.resize(shaderCount);
            if (!getMetalShaderInfo(package, infos.data())) {
                return error(__LINE__);
            }
            break;
        }
        default:
            error(__LINE__);
    }

    if (shaderIndex < 0 || shaderIndex >= shaderCount) {
        return error(__LINE__);
    }

    const ShaderInfo info = infos[shaderIndex];
    ShaderReplacer editor(api, package.getData(), package.getSize());
    if (!editor.replaceShaderSource(info.shaderModel, info.variant, info.pipelineStage, source, size)) {
        return error(__LINE__);
    }

    delete [] material.package;

    material.package = editor.getEditedPackage();
    material.packageSize = editor.getEditedSize();

    if (mEditCallback) {
        mEditCallback(material.userdata, material.name, material.package, material.packageSize);
    }

    return true;
}

} // namespace matdbg
} // namespace filament
