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

#include "driver/Program.h"

using namespace utils;

namespace filament {

// We want these in the .cpp file so they're not inlined (not worth it)
Program::Program() noexcept {
    std::fill(mUniformInterfaceBlocks.begin(), mUniformInterfaceBlocks.end(), nullptr);
    std::fill(mSamplerInterfaceBlocks.begin(), mSamplerInterfaceBlocks.end(), nullptr);
}

Program::Program(const Program& rhs) = default;
Program::Program(Program&& rhs) noexcept = default;
Program& Program::operator=(const Program& rhs) = default;
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

Program& Program::withSamplerBindings(const SamplerBindingMap* bindings) {
    mSamplerBindings = bindings;
    return *this;
}

Program& Program::shader(Program::Shader shader, CString const& source) {
    mShadersSource[size_t(shader)] = source;
    return *this;
}

Program& Program::shader(Program::Shader shader, CString&& source) noexcept {
    source.swap(mShadersSource[size_t(shader)]);
    return *this;
}

Program& Program::addUniformBlock(size_t index, const UniformInterfaceBlock* ib) {
    mUniformInterfaceBlocks[index] = ib;
    return *this;
}

Program& Program::addSamplerBlock(size_t index, const SamplerInterfaceBlock* sb) {
    mSamplerInterfaceBlocks[index] = sb;
    mSamplerCount++;
    return *this;
}

#if !defined(NDEBUG)
io::ostream& operator<<(io::ostream& out, const Program& builder) {
    // FIXME: maybe do better here!
    return out << "Program(" << builder.mName.c_str_safe() << ")";
}
#endif

} // namespace filament
