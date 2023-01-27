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

#ifndef FGDBG_SESSION_H
#define FGDBG_SESSION_H

#include "DebugServer.h"
#include "Resource.h"
#include "Pass.h"
#include <memory>
#include <string>
#include <utility>

namespace filament::fgdbg {
class Session {

public:
    Session(const std::string& name,
            std::shared_ptr<DebugServer> server) : name{ name }, server{ std::move(server) } {}

    void addPasses(const std::vector<Pass>& passes);
    void addPass(const Pass& pass);

    void addResources(const std::vector<Resource>& resources);
    void addResource(const Resource& resource);

    void clear();

    /**
     * Send session information to server
     */
    void update() const;

private:
    const std::string& name;
    std::shared_ptr<DebugServer> server;

    // TODO(@raviola) These might need to be a set/map in order to guarantee there are no two [Pass]
    //  or [Resource] with the same id. This will also allow for updates by key.
    std::vector<Pass> passes{};
    std::vector<Resource> resources{};
};
}

#endif //FGDBG_SESSION_H
