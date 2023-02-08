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

#include "fgdbg/Session.h"
#include <sstream>

namespace filament::fgdbg {

void Session::addPasses(const std::vector<Pass>& passes) {
    this->passes.insert(this->passes.end(), passes.begin(), passes.end());
}

void Session::addPass(const Pass& pass) {
    this->passes.push_back(pass);
}

void Session::addResources(const std::vector<Resource>& resources) {
    this->resources.insert(this->resources.end(),
            resources.begin(),
            resources.end());
}

void Session::addResource(const Resource& resource) {
    this->resources.push_back(resource);
}

/**
 * Uploads the session info to the server, format (YAML):
 passes:
  6:                        <-- pass id
    name: foo
    reads: [4, 9]
    writes: [9, 4]
  3:                        <-- pass id
    name: foo
    reads: [9, 4]
    writes: []
resources:
  9:                        <-- resource id
    name: foo
    size: 100
  4:                        <-- resource id
    name: foo
    size: 30
**/
void Session::update() const {
    std::stringstream ss;
    ss << "passes:";
    for (const auto& pass: passes) {
        ss << "\n ";
        ss << pass.id << ":\n  ";
        ss << "name: " << pass.name << "\n  ";
        ss << "reads: " << "[";
        for (int i = 0; i < pass.reads.size(); i++) {
            if (i != 0) ss << ",";
            ss << pass.reads[i];
        }
        ss << "]";
        ss << "\n  ";
        ss << "writes: " << "[";
        for (int i = 0; i < pass.writes.size(); i++) {
            if (i != 0) ss << ",";
            ss << pass.writes[i];
        }
        ss << "]";
    }
    ss << "\n";

    ss << "resources:";
    for (const auto& resource: resources) {
        ss << "\n ";
        ss << resource.id << ":\n  ";
        ss << "name: " << resource.name << "\n  ";
        ss << "size: " << resource.size;
    }

    server->sendMessage(ss.str());
}

void Session::clear() {
    this->passes.clear();
    this->resources.clear();
}

}