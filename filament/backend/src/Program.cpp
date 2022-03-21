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

#include "private/backend/Program.h"

using namespace utils;

namespace filament::backend {

// We want these in the .cpp file so they're not inlined (not worth it)
Program::Program() noexcept {}  // = default; does not work with msvc because of noexcept
Program::Program(Program&& rhs) noexcept = default;
Program& Program::operator=(Program&& rhs) noexcept = default;
Program::~Program() noexcept = default;

Program& Program::diagnostics(utils::CString const& name,
        utils::Invocable<io::ostream&(utils::io::ostream&)>&& logger) {
    mName = name;
    mLogger = std::move(logger);
    return *this;
}

Program& Program::shader(Program::Shader shader, void const* data, size_t size) noexcept {
    ShaderBlob blob(size);
    std::copy_n((const uint8_t *)data, size, blob.data());
    mShadersSource[size_t(shader)] = std::move(blob);
    return *this;
}

Program& Program::setUniformBlock(size_t bindingPoint, utils::CString uniformBlockName) noexcept {
    mUniformBlocks[bindingPoint] = std::move(uniformBlockName);
    return *this;
}

Program& Program::setSamplerGroup(size_t bindingPoint, ShaderStageFlags stageFlags,
        const Program::Sampler* samplers, size_t count) noexcept {
    auto& groupData = mSamplerGroups[bindingPoint];
    groupData.stageFlags = stageFlags;
    auto& samplerList = groupData.samplers;
    samplerList.reserve(count);
    samplerList.resize(count);
    std::copy_n(samplers, count, samplerList.data());
    mHasSamplers = true;
    return *this;
}

io::ostream& operator<<(io::ostream& out, const Program& builder) {
    out << "Program{";
    builder.mLogger(out);
    out << "}";
    return out;
}

} // namespace filament::backend
