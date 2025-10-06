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

#include <backend/Program.h>
#include <backend/DriverEnums.h>

#include <utils/CString.h>
#include <utils/Invocable.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

using namespace utils;

// We want these in the .cpp file, so they're not inlined (not worth it)
Program::Program(
        utils::InternPool<SpecializationConstant>* specializationConstantsInternPool) noexcept
        : mSpecializationConstantsInternPool(specializationConstantsInternPool) {}

Program::Program(Program&& rhs) noexcept
        : mSpecializationConstantsInternPool(rhs.mSpecializationConstantsInternPool),
          mShadersSource(std::move(rhs.mShadersSource)),
          mShaderLanguage(std::move(rhs.mShaderLanguage)),
          mName(std::move(rhs.mName)),
          mCacheId(std::move(rhs.mCacheId)),
          mPriorityQueue(std::move(rhs.mPriorityQueue)),
          mLogger(std::move(rhs.mLogger)),
          mSpecializationConstants(std::move(rhs.mSpecializationConstants)),
          mPushConstants(std::move(rhs.mPushConstants)),
          mDescriptorBindings(std::move(rhs.mDescriptorBindings)),
          mAttributes(std::move(rhs.mAttributes)),
          mBindingUniformsInfo(std::move(rhs.mBindingUniformsInfo)),
          mMultiview(std::move(rhs.mMultiview)) {
    rhs.mSpecializationConstantsInternPool = nullptr;
    rhs.mSpecializationConstants.clear();
}

Program& Program::operator=(Program&& rhs) noexcept {
    mSpecializationConstantsInternPool = rhs.mSpecializationConstantsInternPool;
    mShadersSource = std::move(rhs.mShadersSource);
    mShaderLanguage = std::move(rhs.mShaderLanguage);
    mName = std::move(rhs.mName);
    mCacheId = std::move(rhs.mCacheId);
    mPriorityQueue = std::move(rhs.mPriorityQueue);
    mLogger = std::move(rhs.mLogger);
    mSpecializationConstants = std::move(rhs.mSpecializationConstants);
    mPushConstants = std::move(rhs.mPushConstants);
    mDescriptorBindings = std::move(rhs.mDescriptorBindings);
    mAttributes = std::move(rhs.mAttributes);
    mBindingUniformsInfo = std::move(rhs.mBindingUniformsInfo);
    mMultiview = std::move(rhs.mMultiview);

    rhs.mSpecializationConstantsInternPool = nullptr;
    rhs.mSpecializationConstants.clear();
    return *this;
}

Program::~Program() noexcept {
    if (mSpecializationConstantsInternPool) {
        mSpecializationConstantsInternPool->release(mSpecializationConstants);
    }
}

Program& Program::priorityQueue(CompilerPriorityQueue const priorityQueue) noexcept {
    mPriorityQueue = priorityQueue;
    return *this;
}

Program& Program::diagnostics(CString const& name,
        Invocable<io::ostream&(CString const& name, io::ostream&)>&& logger) {
    mName = name;
    mLogger = std::move(logger);
    return *this;
}

Program& Program::shader(ShaderStage shader, void const* data, size_t const size) {
    ShaderBlob blob(size);
    std::copy_n((const uint8_t *)data, size, blob.data());
    mShadersSource[size_t(shader)] = std::move(blob);
    return *this;
}

Program& Program::shaderLanguage(ShaderLanguage const shaderLanguage) {
    mShaderLanguage = shaderLanguage;
    return *this;
}

Program& Program::descriptorBindings(descriptor_set_t const set,
        DescriptorBindingsInfo descriptorBindings) noexcept {
    mDescriptorBindings[set] = std::move(descriptorBindings);
    return *this;
}

Program& Program::uniforms(uint32_t index, CString name, UniformInfo uniforms) {
    mBindingUniformsInfo.reserve(mBindingUniformsInfo.capacity() + 1);
    mBindingUniformsInfo.emplace_back(index, std::move(name), std::move(uniforms));
    return *this;
}

Program& Program::attributes(AttributesInfo attributes) noexcept {
    mAttributes = std::move(attributes);
    return *this;
}

Program& Program::specializationConstants(SpecializationConstantsInfo specConstants) noexcept {
    FILAMENT_CHECK_PRECONDITION(mSpecializationConstantsInternPool);
    // NOTE: This is allowed because InternPool treats an empty list as having infinite
    // references held.
    mSpecializationConstantsInternPool->release(mSpecializationConstants);
    mSpecializationConstants = specConstants;
    mSpecializationConstantsInternPool->acquire(mSpecializationConstants);
    return *this;
}

Program& Program::pushConstants(ShaderStage stage,
        FixedCapacityVector<PushConstant> constants) noexcept {
    mPushConstants[static_cast<uint8_t>(stage)] = std::move(constants);
    return *this;
}

Program& Program::cacheId(uint64_t const cacheId) noexcept {
    mCacheId = cacheId;
    return *this;
}

Program& Program::multiview(bool const multiview) noexcept {
    mMultiview = multiview;
    return *this;
}

io::ostream& operator<<(io::ostream& out, const Program& builder) {
    out << "Program{";
    builder.mLogger(builder.mName, out);
    out << "}";
    return out;
}

} // namespace filament::backend
