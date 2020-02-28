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

#ifndef TNT_FILAMENT_VIRTUALRESOURCE_H
#define TNT_FILAMENT_VIRTUALRESOURCE_H

namespace filament {

class FrameGraph;

namespace fg {

struct PassNode;

struct VirtualResource {
    VirtualResource() noexcept = default;
    VirtualResource(VirtualResource const&) = default;
    virtual void resolve(FrameGraph& fg) noexcept = 0;
    virtual void preExecuteDevirtualize(FrameGraph& fg) noexcept = 0;
    virtual void preExecuteDestroy(FrameGraph& fg) noexcept = 0;
    virtual void postExecuteDestroy(FrameGraph& fg) noexcept = 0;
    virtual void postExecuteDevirtualize(FrameGraph& fg) noexcept = 0;
    virtual ~VirtualResource();

    // computed during compile()
    PassNode* first = nullptr;              // pass that needs to instantiate the resource
    PassNode* last = nullptr;               // pass that can destroy the resource
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_VIRTUALRESOURCE_H
