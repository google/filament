/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_BUFFEROBJECT_H
#define TNT_FILAMENT_DETAILS_BUFFEROBJECT_H

#include "downcast.h"

#include <backend/Handle.h>

#include <filament/BufferObject.h>

#include <utils/compiler.h>

namespace filament {

class FEngine;

class FBufferObject : public BufferObject {
public:
    FBufferObject(FEngine& engine, const Builder& builder);

    // frees driver resources, object becomes invalid
    void terminate(FEngine& engine);

    backend::Handle<backend::HwBufferObject> getHwHandle() const noexcept { return mHandle; }

    size_t getByteCount() const noexcept { return mByteCount; }

    BindingType getBindingType() const noexcept { return mBindingType; }

private:
    friend class BufferObject;
    void setBuffer(FEngine& engine, BufferDescriptor&& buffer, uint32_t byteOffset = 0);
    backend::Handle<backend::HwBufferObject> mHandle;
    uint32_t mByteCount;
    BindingType mBindingType;
};

FILAMENT_DOWNCAST(BufferObject)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_BUFFEROBJECT_H
