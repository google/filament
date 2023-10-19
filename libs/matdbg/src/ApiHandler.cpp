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

#include <backend/DriverEnums.h>

#include <matdbg/DebugServer.h>
#include <matdbg/JsonWriter.h>
#include <matdbg/ShaderExtractor.h>
#include <matdbg/ShaderInfo.h>

#include <filaflat/ChunkContainer.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>

#include <CivetServer.h>

#include <sstream>
#include <chrono>

namespace filament::matdbg {

using namespace filament::backend;
using namespace std::chrono_literals;

namespace {

auto const& kSuccessHeader = DebugServer::kSuccessHeader;
auto const& kErrorHeader = DebugServer::kErrorHeader;

void spirvToAsm(struct mg_connection* conn, uint32_t const* spirv, size_t size) {
    auto spirvDisassembly = ShaderExtractor::spirvToText(spirv, size / 4);
    mg_printf(conn, kSuccessHeader.data(), "application/txt");
    mg_write(conn, spirvDisassembly.c_str(), spirvDisassembly.size());
}

void spirvToGlsl(ShaderModel shaderModel, struct mg_connection* conn, uint32_t const* spirv,
        size_t size) {
    auto glsl = ShaderExtractor::spirvToGLSL(shaderModel, spirv, size / 4);
    mg_printf(conn, kSuccessHeader.data(), "application/txt");
    mg_printf(conn, glsl.c_str(), glsl.size());
}

} // anonymous

using filaflat::ChunkContainer;
using filamat::ChunkType;
using utils::FixedCapacityVector;

static auto const error = [](int line, std::string const& uri) {
    utils::slog.e << "DebugServer: 404 at line " << line << ": " << uri << utils::io::endl;
    return false;
};

MaterialRecord const* ApiHandler::getMaterialRecord(struct mg_request_info const* request) {
    size_t const qlength = strlen(request->query_string);
    char matid[9] = {};
    if (mg_get_var(request->query_string, qlength, "matid", matid, sizeof(matid)) < 0) {
        return nullptr;
    }
    uint32_t const id = strtoul(matid, nullptr, 16);
    return mServer->getRecord(id);
}

bool ApiHandler::handleGetApiShader(struct mg_connection* conn,
        struct mg_request_info const* request) {
    auto const softError = [conn, request](char const* msg) {
        utils::slog.e << "DebugServer: " << msg << ": " << request->query_string << utils::io::endl;
        mg_printf(conn, kErrorHeader.data(), "application/txt");
        mg_write(conn, msg, strlen(msg));
        return true;
    };

    MaterialRecord const* result = getMaterialRecord(request);
    std::string const& uri = request->local_uri;

    if (!result) {
        return error(__LINE__, uri);
    }

    ChunkContainer package(result->package, result->packageSize);
    if (!package.parse()) {
        return error(__LINE__, uri);
    }

    std::string_view const glsl("glsl");
    std::string_view const msl("msl");
    std::string_view const spirv("spirv");
    size_t const qlength = strlen(request->query_string);

    char type[6] = {};
    if (mg_get_var(request->query_string, qlength, "type", type, sizeof(type)) < 0) {
        return error(__LINE__, uri);
    }

    std::string_view const language(type, strlen(type));

    char glindex[4] = {};
    char vkindex[4] = {};
    char metalindex[4] = {};
    mg_get_var(request->query_string, qlength, "glindex", glindex, sizeof(glindex));
    mg_get_var(request->query_string, qlength, "vkindex", vkindex, sizeof(vkindex));
    mg_get_var(request->query_string, qlength, "metalindex", metalindex, sizeof(metalindex));

    if (!glindex[0] && !vkindex[0] && !metalindex[0]) {
        return error(__LINE__, uri);
    }

    if (glindex[0]) {
        if (language != glsl) {
            return softError("Only GLSL is supported.");
        }

        FixedCapacityVector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialGlsl));
        if (!getShaderInfo(package, info.data(), ChunkType::MaterialGlsl)) {
            return error(__LINE__, uri);
        }

        int const shaderIndex = std::stoi(glindex);
        if (shaderIndex >= info.size()) {
            return error(__LINE__, uri);
        }

        ShaderExtractor extractor(ShaderLanguage::ESSL3, result->package, result->packageSize);
        if (!extractor.parse()) {
            return error(__LINE__, uri);
        }

        auto const& item = info[shaderIndex];
        filaflat::ShaderContent content;
        extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, content);

        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        mg_write(conn, content.data(), content.size() - 1);
        return true;
    }

    if (vkindex[0]) {
        ShaderExtractor extractor(ShaderLanguage::SPIRV, result->package, result->packageSize);
        if (!extractor.parse()) {
            return error(__LINE__, uri);
        }

        FixedCapacityVector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialSpirv));
        if (!getShaderInfo(package, info.data(), ChunkType::MaterialSpirv)) {
            return error(__LINE__, uri);
        }

        int const shaderIndex = std::stoi(vkindex);
        if (shaderIndex >= info.size()) {
            return error(__LINE__, uri);
        }

        auto const& item = info[shaderIndex];
        filaflat::ShaderContent content;
        extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, content);

        if (language == spirv) {
            spirvToAsm(conn, (uint32_t const*) content.data(), content.size());
            return true;
        }

        if (language == glsl) {
            spirvToGlsl(item.shaderModel, conn, (uint32_t const*) content.data(), content.size());
            return true;
        }

        return softError("Only SPIRV is supported.");
    }

    if (metalindex[0]) {
        ShaderExtractor extractor(ShaderLanguage::MSL, result->package, result->packageSize);
        if (!extractor.parse()) {
            return error(__LINE__, uri);
        }

        FixedCapacityVector<ShaderInfo> info(getShaderCount(package, ChunkType::MaterialMetal));
        if (!getShaderInfo(package, info.data(), ChunkType::MaterialMetal)) {
            return error(__LINE__, uri);
        }

        int const shaderIndex = std::stoi(metalindex);
        if (shaderIndex >= info.size()) {
            return error(__LINE__, uri);
        }

        auto const& item = info[shaderIndex];
        filaflat::ShaderContent content;
        extractor.getShader(item.shaderModel, item.variant, item.pipelineStage, content);

        if (language == msl) {
            mg_printf(conn, kSuccessHeader.data(), "application/txt");
            mg_write(conn, content.data(), content.size() - 1);
            return true;
        }

        return softError("Only MSL is supported.");
    }
    return error(__LINE__, uri);
}

void ApiHandler::addMaterial(MaterialRecord const* material) {
    std::unique_lock<utils::Mutex> lock(mStatusMutex);
    snprintf(statusMaterialId, sizeof(statusMaterialId), "%8.8x", material->key);
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

    std::unique_lock<utils::Mutex> lock(mStatusMutex);
    uint64_t const currentStatusCount = mCurrentStatus;
    if (mStatusCondition.wait_for(lock, 10s,
                [this, currentStatusCount] { return currentStatusCount < mCurrentStatus; })) {
        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        mg_write(conn, statusMaterialId, 8);
    } else {
        mg_printf(conn, kSuccessHeader.data(), "application/txt");
        // Use '1' to indicate a no-op.  This ensures that we don't block forever if the client is
        // gone.
        mg_write(conn, "1", 1);
    }
    return true;
}

bool ApiHandler::handlePost(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const& uri = request->local_uri;

    // For now we simply use istringstream for parsing, so command arguments are delimited
    // with space characters.
    //
    // The "shader index" is a zero-based index into the list of variants using the order that
    // they appear in the package, where each API (GL / VK / Metal) has its own list.
    //
    // POST body format:
    //   [material id] [api index] [shader index] [shader source....]
    if (uri == "/api/edit") {
        struct mg_request_info const* req_info = mg_get_request_info(conn);
        size_t msgLen = req_info->content_length;

        char buf[1024];
        size_t readLen = 0;
        std::stringstream sstream;
        while (readLen < msgLen) {
            int res = mg_read(conn, buf, sizeof(buf));
            if (res < 0) {
                utils::slog.e << "civet error parsing /api/edit body: " << res << utils::io::endl;
                break;
            }
            if (res == 0) {
                break;
            }
            readLen += res;
            sstream.write(buf, res);
        }
        uint32_t matid;
        int api;
        int shaderIndex;
        sstream >> std::hex >> matid >> std::dec >> api >> shaderIndex;
        std::string shader = sstream.str().substr(sstream.tellg());

        mServer->handleEditCommand(matid, backend::Backend(api), shaderIndex, shader.c_str(),
                shader.size());

        mg_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close");
        return true;
    }
    return error(__LINE__, uri);
}

bool ApiHandler::handleGet(CivetServer* server, struct mg_connection* conn) {
    struct mg_request_info const* request = mg_get_request_info(conn);
    std::string const& uri = request->local_uri;

    if (uri == "/api/active") {
        mServer->updateActiveVariants();

        // Careful not to lock the above line.
        std::unique_lock<utils::Mutex> lock(mServer->mMaterialRecordsMutex);
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "{");

        // If the backend has not been resolved to Vulkan, Metal, etc., then return an empty
        // list. This can occur if the server is matinfo rather than an actual Filament session.
        if (mServer->mBackend == backend::Backend::DEFAULT) {
            mg_printf(conn, "}");
            return true;
        }

        int index = 0;
        for (auto const& pair: mServer->mMaterialRecords) {
            auto const& record = pair.second;
            ChunkContainer package(record.package, record.packageSize);
            if (!package.parse()) {
                return error(__LINE__, uri);
            }
            JsonWriter writer;
            if (!writer.writeActiveInfo(package, mServer->mBackend, record.activeVariants)) {
                return error(__LINE__, uri);
            }
            bool const last = (++index) == mServer->mMaterialRecords.size();
            mg_printf(conn, "\"%8.8x\": %s %s", pair.first, writer.getJsonString(),
                    last ? "" : ",");
        }
        mg_printf(conn, "}");
        return true;
    }

    if (uri == "/api/matids") {
        std::unique_lock<utils::Mutex> lock(mServer->mMaterialRecordsMutex);
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "[");
        int index = 0;
        for (auto const& record: mServer->mMaterialRecords) {
            bool const last = (++index) == mServer->mMaterialRecords.size();
            mg_printf(conn, "\"%8.8x\" %s", record.first, last ? "" : ",");
        }
        mg_printf(conn, "]");
        return true;
    }

    if (uri == "/api/materials") {
        std::unique_lock<utils::Mutex> lock(mServer->mMaterialRecordsMutex);
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "[");
        int index = 0;
        for (auto const& record: mServer->mMaterialRecords) {
            bool const last = (++index) == mServer->mMaterialRecords.size();

            ChunkContainer package(record.second.package, record.second.packageSize);
            if (!package.parse()) {
                return error(__LINE__, uri);
            }

            JsonWriter writer;
            if (!writer.writeMaterialInfo(package)) {
                return error(__LINE__, uri);
            }

            mg_printf(conn, "{ \"matid\": \"%8.8x\", %s } %s", record.first, writer.getJsonString(),
                    last ? "" : ",");
        }
        mg_printf(conn, "]");
        return true;
    }

    if (uri == "/api/material") {
        MaterialRecord const* result = getMaterialRecord(request);
        if (!result) {
            return error(__LINE__, uri);
        }

        ChunkContainer package(result->package, result->packageSize);
        if (!package.parse()) {
            return error(__LINE__, uri);
        }

        JsonWriter writer;
        if (!writer.writeMaterialInfo(package)) {
            return error(__LINE__, uri);
        }
        mg_printf(conn, kSuccessHeader.data(), "application/json");
        mg_printf(conn, "{ %s }", writer.getJsonString());
        return true;
    }

    if (uri == "/api/shader") {
        return handleGetApiShader(conn, request);
    }

    if (uri.find("/api/status") == 0) {
        return handleGetStatus(conn, request);
    }

    return error(__LINE__, uri);
}

} // filament::matdbg
