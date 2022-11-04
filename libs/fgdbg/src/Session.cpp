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

void Session::update() const {
    // TODO(@raviola): Temporary format
    //  <---------------passes------------------->|<----------resources----------------->
    //  id:name:readId-readId:writeId-writeId, ...|id:name:size,id:name:size,id:name:size
    std::stringstream ss;

    // Passes
    bool first = true;
    for (const auto& pass: passes) {
        if (!first) ss << ",";
        first = false;
        ss << pass.id;
        ss << ":";
        ss << pass.name;
        ss << ":";
        for (int i = 0; i < pass.reads.size(); i++) {
            if (i != 0) ss << "-";
            ss << pass.reads[i];
        }
        ss << ":";
        for (int i = 0; i < pass.writes.size(); i++) {
            if (i != 0) ss << "-";
            ss << pass.writes[i];
        }
    }
    ss << "|";
    // Resources
    first = true;
    for (const auto& resource: resources) {
        if (!first) ss << ",";
        first = false;
        ss << resource.id;
        ss << ":";
        ss << resource.name;
        ss << ":";
        ss << resource.size;
    }

    server->sendMessage(ss.str());
}

void Session::clear() {
    this->passes.clear();
    this->resources.clear();
}

}