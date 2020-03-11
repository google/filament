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

namespace filament {
namespace backend {

// We want these in the .cpp file so they're not inlined (not worth it)
Program::Program() noexcept {}  // = default; does not work with msvc because of noexcept
Program::Program(Program&& rhs) noexcept = default;
Program& Program::operator=(Program&& rhs) noexcept = default;
Program::~Program() noexcept = default;

Program& Program::diagnostics(utils::CString const& name, uint8_t variant) {
    mName = name;
    mVariant = variant;
    return *this;
}

Program& Program::diagnostics(utils::CString&& name, uint8_t variant) noexcept {
    name.swap(mName);
    mVariant = variant;
    return *this;
}

Program& Program::shader(Program::Shader shader, void const* data, size_t size) noexcept {
    std::vector<uint8_t> blob(size);
    std::copy_n((const uint8_t *)data, size, blob.data());
    mShadersSource[size_t(shader)] = std::move(blob);
    return *this;
}

Program& Program::setUniformBlock(size_t bindingPoint, utils::CString uniformBlockName) noexcept {
    mUniformBlocks[bindingPoint] = std::move(uniformBlockName);
    return *this;
}

Program& Program::setSamplerGroup(size_t bindingPoint,
        const Program::Sampler* samplers, size_t count) noexcept {
    auto& samplerList = mSamplerGroups[bindingPoint];
    samplerList.clear();
    samplerList.insert(samplerList.begin(), samplers, samplers + count);
    mHasSamplers = true;
    return *this;
}


#if !defined(NDEBUG)
io::ostream& operator<<(io::ostream& out, const Program& builder) {
    return out << "Program(" << builder.mName.c_str_safe() << ")";
}

#endif

} // namespace backend
} // namespace filament
