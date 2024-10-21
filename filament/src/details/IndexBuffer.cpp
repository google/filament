/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/IndexBuffer.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <utils/CString.h>

namespace filament {

struct IndexBuffer::BuilderDetails {
    uint32_t mIndexCount = 0;
    IndexType mIndexType = IndexType::UINT;
};

using BuilderType = IndexBuffer;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

IndexBuffer::Builder& IndexBuffer::Builder::indexCount(uint32_t indexCount) noexcept {
    mImpl->mIndexCount = indexCount;
    return *this;
}

IndexBuffer::Builder& IndexBuffer::Builder::bufferType(IndexType indexType) noexcept {
    mImpl->mIndexType = indexType;
    return *this;
}

IndexBuffer* IndexBuffer::Builder::build(Engine& engine) {
    return downcast(engine).createIndexBuffer(*this);
}

// ------------------------------------------------------------------------------------------------

FIndexBuffer::FIndexBuffer(FEngine& engine, const IndexBuffer::Builder& builder)
        : mIndexCount(builder->mIndexCount) {
    auto& name = builder.getName();
    const char* const tag = name.empty() ? "(no tag)" : name.c_str_safe();
    FILAMENT_CHECK_PRECONDITION(
            builder->mIndexType == IndexType::UINT || builder->mIndexType == IndexType::USHORT)
            << "Invalid index type " << static_cast<int>(builder->mIndexType) << ", tag=" << tag;
    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createIndexBuffer(
            (backend::ElementType)builder->mIndexType,
            uint32_t(builder->mIndexCount),
            backend::BufferUsage::STATIC);
    if (auto name = builder.getName(); !name.empty()) {
        driver.setDebugTag(mHandle.getId(), std::move(name));
    }
}

void FIndexBuffer::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyIndexBuffer(mHandle);
}

void FIndexBuffer::setBuffer(FEngine& engine, BufferDescriptor&& buffer, uint32_t byteOffset) {
    engine.getDriverApi().updateIndexBuffer(mHandle, std::move(buffer), byteOffset);
}

} // namespace filament
