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

#include "details/BufferObject.h"

#include "details/Engine.h"

#include "FilamentAPI-impl.h"

#include <backend/DriverEnums.h>

#include <filament/BufferObject.h>

#include <utils/CString.h>
#include <utils/Panic.h>
#include <utils/StaticString.h>

#include <utility>

#include <stdint.h>
#include <stddef.h>

namespace filament {

struct BufferObject::BuilderDetails {
    BindingType mBindingType = BindingType::VERTEX;
    uint32_t mByteCount = 0;
};

using BuilderType = BufferObject;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

BufferObject::Builder& BufferObject::Builder::size(uint32_t const byteCount) noexcept {
    mImpl->mByteCount = byteCount;
    return *this;
}

BufferObject::Builder& BufferObject::Builder::bindingType(BindingType const bindingType) noexcept {
    mImpl->mBindingType = bindingType;
    return *this;
}

BufferObject::Builder& BufferObject::Builder::name(const char* name, size_t const len) noexcept {
    return BuilderNameMixin::name(name, len);
}

BufferObject::Builder& BufferObject::Builder::name(utils::StaticString const& name) noexcept {
    return BuilderNameMixin::name(name);
}

BufferObject* BufferObject::Builder::build(Engine& engine) {
    return downcast(engine).createBufferObject(*this);
}

// ------------------------------------------------------------------------------------------------

FBufferObject::FBufferObject(FEngine& engine, const Builder& builder)
        : mByteCount(builder->mByteCount), mBindingType(builder->mBindingType) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    mHandle = driver.createBufferObject(builder->mByteCount, builder->mBindingType,
            backend::BufferUsage::STATIC, builder.getName());
}

void FBufferObject::terminate(FEngine& engine) {
    FEngine::DriverApi& driver = engine.getDriverApi();
    driver.destroyBufferObject(mHandle);
}

void FBufferObject::setBuffer(FEngine& engine, BufferDescriptor&& buffer, uint32_t const byteOffset) {

    FILAMENT_CHECK_PRECONDITION((byteOffset & 0x3) == 0)
            << "byteOffset must be a multiple of 4";

    engine.getDriverApi().updateBufferObject(mHandle, std::move(buffer), byteOffset);
}

} // namespace filament
