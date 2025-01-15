/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <fgviewer/JsonWriter.h>
#include <fgviewer/FrameGraphInfo.h>

#include <iomanip>
#include <sstream>

namespace filament::fgviewer {

namespace {
void writeJSONString(std::ostream& os, const char* str) {
    os << '"';
    const char* p = str;
    while (*p != '\0') {
        switch (*p) {
            case '"': os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            case '\n': os << "\\n"; break;
            case '\t': os << "\\t"; break;
            default: os << *p; break;
        }
        ++p;
    }
    os << '"';
}

void writeViewName(std::ostream& os, const FrameGraphInfo &frameGraph) {
    os << "  \"viewName\": ";
    writeJSONString(os, frameGraph.getViewName());
    os << ",\n";
}

void writeResourceIds(std::ostream& os, const std::vector<ResourceId> &resources) {
    for (size_t j = 0; j < resources.size(); ++j) {
        os << resources[j];
        if (j + 1 < resources.size()) os << ", ";
    }
}

void writePasses(std::ostream& os, const FrameGraphInfo &frameGraph) {
    os << "  \"passes\": [\n";
    auto& passes = frameGraph.getPasses();
    for (size_t i = 0; i < passes.size(); ++i) {
        const FrameGraphInfo::Pass& pass = passes[i];
        os << "    {\n";
        os << "      \"name\": ";
        writeJSONString(os, pass.name.c_str());
        os << ",\n";

        const std::vector<ResourceId>& reads = pass.reads;
        os << "      \"reads\": [";
        writeResourceIds(os, reads);
        os << "],\n";

        const std::vector<ResourceId>& writes = pass.writes;
        os << "      \"writes\": [";
        writeResourceIds(os, writes);
        os << "]\n";

        os << "    }";
        if (i + 1 < passes.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";
}

void writeResources(std::ostream& os, const FrameGraphInfo &frameGraph) {
    os << "  \"resources\": {\n";
    size_t resourceCount = 0;
    auto& resources = frameGraph.getResources();
    for (const auto& [id, resource] : resources) {
        os << "    \"" << id << "\": {\n";
        os << "      \"id\": " << resource.id << ",\n";
        os << "      \"name\": ";
        writeJSONString(os, resource.name.c_str());
        os << ",\n";

        os << "      \"properties\": [\n";
        for (size_t j = 0; j < resource.properties.size(); ++j) {
            const auto& [key, value] = resource.properties[j];
            os << "        {\n";
            os << "          \"key\": ";
            writeJSONString(os, key.c_str());
            os << ",\n";
            os << "          \"value\": ";
            writeJSONString(os, value.c_str());
            os << "\n        }";
            if (j + 1 < resource.properties.size()) os << ",";
            os << "\n";
        }
        os << "      ]\n";
        os << "    }";
        if (++resourceCount < resources.size()) os << ",";
        os << "\n";
    }
    os << "  }\n";
}
}

const char *JsonWriter::getJsonString() const {
    return mJsonString.c_str();
}

size_t JsonWriter::getJsonSize() const {
    return mJsonString.size();
}

bool JsonWriter::writeFrameGraphInfo(const FrameGraphInfo &frameGraph) {
    std::ostringstream os;
    os << "{\n";

    writeViewName(os, frameGraph);
    writePasses(os, frameGraph);
    writeResources(os, frameGraph);

    os << "}\n";

    mJsonString = utils::CString(os.str().c_str());
    return true;
}

} // namespace filament::fgviewer
