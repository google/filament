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

#include <fgviewer/FrameGraphInfo.h>

namespace filament::fgviewer {

class FrameGraphInfo::FrameGraphInfoImpl {
public:
    void setViewName(utils::CString name) {
        viewName = std::move(name);
    }

    void setPasses(std::vector<Pass> sortedPasses) {
        passes = std::move(sortedPasses);
    }

    void setResources(std::unordered_map<ResourceId, Resource> resourceMap) {
        resources = std::move(resourceMap);
    }

private:
    utils::CString viewName;
    // The order of the passes in the vector indicates the execution
    // order of the passes.
    std::vector<Pass> passes;
    std::unordered_map<ResourceId, Resource> resources;
};


FrameGraphInfo::FrameGraphInfo(utils::CString viewName)
    : pImpl(std::make_unique<FrameGraphInfoImpl>()) {
    pImpl->setViewName(std::move(viewName));
}

FrameGraphInfo::~FrameGraphInfo() = default;

FrameGraphInfo::FrameGraphInfo(FrameGraphInfo&& rhs) noexcept = default;

void FrameGraphInfo::setResources(
    std::unordered_map<ResourceId, Resource> resources) {
    pImpl->setResources(std::move(resources));
}

void FrameGraphInfo::setPasses(std::vector<Pass> sortedPasses) {
    pImpl->setPasses(std::move(sortedPasses));
}
} // namespace filament::fgviewer
