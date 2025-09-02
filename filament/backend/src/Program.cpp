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

#include <utils/debug.h>
#include <utils/CString.h>
#include <utils/ostream.h>
#include <utils/Invocable.h>

#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

using namespace utils;

// We want these in the .cpp file, so they're not inlined (not worth it)
Program::Program() noexcept {  // NOLINT(modernize-use-equals-default)
}

Program::Program(Program&& rhs) noexcept = default;

Program& Program::operator=(Program&& rhs) noexcept = default;

Program::~Program() noexcept = default;

Program& Program::priorityQueue(CompilerPriorityQueue priorityQueue) noexcept {
    mPriorityQueue = priorityQueue;
    return *this;
}

Program& Program::diagnostics(CString const& name,
        Invocable<io::ostream&(utils::CString const& name, io::ostream&)>&& logger) {
    mName = name;
    mLogger = std::move(logger);
    return *this;
}

Program& Program::shader(ShaderStage shader, void const* data, size_t size) {
    ShaderBlob blob(size);
    std::copy_n((const uint8_t *)data, size, blob.data());
    mShadersSource[size_t(shader)] = std::move(blob);
    return *this;
}

Program& Program::shaderLanguage(ShaderLanguage shaderLanguage) {
    mShaderLanguage = shaderLanguage;
    return *this;
}

Program& Program::descriptorBindings(backend::descriptor_set_t set,
        DescriptorBindingsInfo descriptorBindings) noexcept {
    mDescriptorBindings[set] = std::move(descriptorBindings);
    return *this;
}

Program& Program::uniforms(uint32_t index, utils::CString name, UniformInfo uniforms) noexcept {
    mBindingUniformsInfo.reserve(mBindingUniformsInfo.capacity() + 1);
    mBindingUniformsInfo.emplace_back(index, std::move(name), std::move(uniforms));
    return *this;
}

Program& Program::attributes(AttributesInfo attributes) noexcept {
    mAttributes = std::move(attributes);
    return *this;
}

Program& Program::specializationConstants(SpecializationConstantsInfo specConstants) noexcept {
    mSpecializationConstants = std::move(specConstants);
    return *this;
}

Program& Program::pushConstants(ShaderStage stage,
        utils::FixedCapacityVector<PushConstant> constants) noexcept {
    mPushConstants[static_cast<uint8_t>(stage)] = std::move(constants);
    return *this;
}

Program& Program::cacheId(uint64_t cacheId) noexcept {
    mCacheId = cacheId;
    return *this;
}

Program& Program::multiview(bool multiview) noexcept {
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
