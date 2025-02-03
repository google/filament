/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef FGVIEWER_FRAMEGRAPHINFO_H
#define FGVIEWER_FRAMEGRAPHINFO_H

#include <utils/CString.h>

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace filament::fgviewer {
using ResourceId = uint32_t;

class FrameGraphInfo {
public:
    explicit FrameGraphInfo(utils::CString viewName);

    ~FrameGraphInfo();

    FrameGraphInfo(FrameGraphInfo &&rhs) noexcept;

    FrameGraphInfo(FrameGraphInfo const &) = delete;

    bool operator==(const FrameGraphInfo& rhs) const;

    struct Pass {
        Pass(utils::CString name, std::vector<ResourceId> reads,
             std::vector<ResourceId> writes);

        bool operator==(const Pass& rhs) const;

        utils::CString name;
        std::vector<ResourceId> reads;
        std::vector<ResourceId> writes;
    };

    struct Resource {
        bool operator==(const Resource& rhs) const;

        struct Property {
            bool operator==(const Property& rhs) const;

            utils::CString name;
            utils::CString value;
        };
        
        Resource(ResourceId id, utils::CString name,
                 std::vector<Property> properties);

        ResourceId id;
        utils::CString name;
        // We use a vector of Property here to store the resource properties,
        // so different kinds of resources could choose different types of
        // properties to record.
        // ex.
        // Texture2D --> { {"name","XXX"}, {"sizeX", "1024"}, {"sizeY", "768"} }
        // Buffer1D --> { {"name", "XXX"}, {"size", "512"} }
        std::vector<Property> properties;
    };

    void setResources(std::unordered_map<ResourceId, Resource> resources);

    // The incoming passes should be sorted by the execution order.
    void setPasses(std::vector<Pass> sortedPasses);

    const char* getViewName() const;

    const std::vector<Pass>& getPasses() const;

    const std::unordered_map<ResourceId, Resource>& getResources() const;

private:
    utils::CString viewName;
    // The order of the passes in the vector indicates the execution
    // order of the passes.
    std::vector<Pass> passes;
    std::unordered_map<ResourceId, Resource> resources;
};
} // namespace filament::fgviewer

#endif //FGVIEWER_FRAMEGRAPHINFO_H
