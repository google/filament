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

#ifndef TNT_FILAMENT_DETAILS_INSTANCEBUFFER_H
#define TNT_FILAMENT_DETAILS_INSTANCEBUFFER_H

#include "downcast.h"

#include <filament/InstanceBuffer.h>

#include <backend/Handle.h>

#include <math/mat4.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <cstddef>

namespace filament {

class RenderableManager;
class FEngine;

struct PerRenderableData;

class FInstanceBuffer : public InstanceBuffer {
public:
    FInstanceBuffer(FEngine& engine, const Builder& builder);

    void terminate(FEngine& engine);

    inline size_t getInstanceCount() const noexcept { return mInstanceCount; }

    void setLocalTransforms(math::mat4f const* localTransforms, size_t count, size_t offset);

    void prepare(FEngine& engine, math::mat4f const& rootTransform, const PerRenderableData& ubo,
            backend::Handle<backend::HwBufferObject> handle);

    utils::CString const& getName() const noexcept { return mName; }

private:
    friend class RenderableManager;

    utils::FixedCapacityVector<math::mat4f> mLocalTransforms;
    utils::CString mName;
    size_t mInstanceCount;
};

FILAMENT_DOWNCAST(InstanceBuffer)

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_INSTANCEBUFFER_H
