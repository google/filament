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
#include <unordered_map>

namespace filament::fgdbg {
class Session {

public:
    Session(std::string name,
            std::shared_ptr<DebugServer> server) : mName{ std::move(name) }, mServer{ server } {}

    void addPasses(const std::vector<Pass>& passes);
    void addPass(const Pass& pass);

    void addResources(const std::vector<Resource>& resources);
    void addResource(const Resource& resource);

    void clear();

    /**
     * Send session information to server
     */
    void update();

private:
    const std::string mName;
    std::shared_ptr<DebugServer> mServer;

    std::unordered_map<size_t, Pass> mPasses{};
    std::unordered_map<size_t, Resource> mResources{};

    /**
     * Ensures that all [Passes] read/write from existing resources.
     * @return: true if the data should be uploaded to the server, false otherwise
     */
    bool verify();

    [[nodiscard]] std::string serialize() const;

    std::size_t mHash = 0;
};
}

#endif //FGDBG_SESSION_H