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
#include <set>
#include <iostream>

namespace filament::fgdbg {

void Session::addPasses(const std::vector<Pass>& passes) {
    for (auto& pass: passes) {
        addPass(pass);
    }
}

void Session::addPass(const Pass& pass) {
    mPasses.insert({ pass.id, pass });
}

void Session::addResources(const std::vector<Resource>& resources) {
    for (auto& resource: resources) {
        addResource(resource);
    }
}

void Session::addResource(const Resource& resource) {
    mResources.insert({ resource.id, resource });
}

void Session::update() {
    if (!verify()) return;
    std::string payload = serialize();
    std::size_t currentHash = std::hash<std::string>{}(payload);
    if (mHash == currentHash) return;
    mHash = currentHash;
    mServer->sendMessage(payload);
}

void Session::clear() {
    mPasses.clear();
    mResources.clear();
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
std::string Session::serialize() const {
    std::stringstream ss;
    ss << "passes:";
    for (const auto& [id, pass]: mPasses) {
        ss << "\n ";
        ss << id << ":\n  ";
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
    for (const auto& [id, resource]: mResources) {
        ss << "\n ";
        ss << id << ":\n  ";
        ss << "name: " << resource.name << "\n  ";
        ss << "size: " << resource.size;
    }
    return ss.str();
}

bool Session::verify() {
    std::set<size_t> rw;
    for (auto& [id, pass]: mPasses) {
        rw.insert(pass.reads.begin(), pass.reads.end());
        rw.insert(pass.writes.begin(), pass.writes.end());
    }
    for (const auto& resource_id: rw) {
        if (mResources.count(resource_id) == 0) {
            std::cout << "fgdbg: Error verifying data before sever upload - "
                      << "Trying to read/write from non-existing resource id: " << resource_id
                      << std::endl;
            return false;
        }
    }

    return true;
}

}