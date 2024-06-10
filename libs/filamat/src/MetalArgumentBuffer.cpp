/*
* Copyright (C) 2022 The Android Open Source Project
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

#include "MetalArgumentBuffer.h"

#include <sstream>
#include <utility>

namespace filamat {

MetalArgumentBuffer::Builder& filamat::MetalArgumentBuffer::Builder::name(
        const std::string& name) noexcept {
    mName = name;
    return *this;
}

MetalArgumentBuffer::Builder& MetalArgumentBuffer::Builder::texture(size_t index,
        const std::string& name, filament::backend::SamplerType type,
        filament::backend::SamplerFormat format,
        bool multisample) noexcept {

    using namespace filament::backend;

    // All combinations of SamplerType and SamplerFormat are valid except for SAMPLER_3D / SHADOW.
    assert_invariant(type != SamplerType::SAMPLER_3D || format != SamplerFormat::SHADOW);

    // multisample textures have restrictions too
    assert_invariant(!multisample || (
            format != SamplerFormat::SHADOW && (
                    type == SamplerType::SAMPLER_2D || type == SamplerType::SAMPLER_2D_ARRAY)));

    mArguments.emplace_back(TextureArgument { name, index, type, format, multisample });
    return *this;
}

MetalArgumentBuffer::Builder& MetalArgumentBuffer::Builder::sampler(
        size_t index, const std::string& name) noexcept {
    mArguments.emplace_back(SamplerArgument { name , index });
    return *this;
}

MetalArgumentBuffer* MetalArgumentBuffer::Builder::build() {
    assert_invariant(!mName.empty());
    return new MetalArgumentBuffer(*this);
}

std::ostream& MetalArgumentBuffer::Builder::TextureArgument::write(std::ostream& os) const {
    switch (format) {
        case filament::backend::SamplerFormat::INT:
        case filament::backend::SamplerFormat::UINT:
        case filament::backend::SamplerFormat::FLOAT:
            os << "texture";
            break;
        case filament::backend::SamplerFormat::SHADOW:
            os << "depth";
            break;
    }

    switch (type) {
        case filament::backend::SamplerType::SAMPLER_EXTERNAL:
        case filament::backend::SamplerType::SAMPLER_2D:
            os << "2d";
            break;
        case filament::backend::SamplerType::SAMPLER_2D_ARRAY:
            os << "2d_array";
            break;
        case filament::backend::SamplerType::SAMPLER_CUBEMAP:
            os << "cube";
            break;
        case filament::backend::SamplerType::SAMPLER_3D:
            os << "3d";
            break;
        case filament::backend::SamplerType::SAMPLER_CUBEMAP_ARRAY:
            os << "cube_array";
            break;
    }

    if (multisample) {
        os << "_ms";
    }

    switch (format) {
        case filament::backend::SamplerFormat::INT:
            os << "<int>";
            break;
        case filament::backend::SamplerFormat::UINT:
            os << "<uint>";
            break;
        case filament::backend::SamplerFormat::FLOAT:
        case filament::backend::SamplerFormat::SHADOW:
            os << "<float>";
            break;
    }

    os << " " << name << " [[id(" << index << ")]];" << std::endl;
    return os;
}

std::ostream& MetalArgumentBuffer::Builder::SamplerArgument::write(std::ostream& os) const {
    os << "sampler " << name << " [[id(" << index << ")]];" << std::endl;
    return os;
}

MetalArgumentBuffer::MetalArgumentBuffer(Builder& builder) {
    mName = builder.mName;

    std::stringstream ss;
    ss << "struct " << mName << " {" << std::endl;

    auto& args = builder.mArguments;

    // Sort the arguments by index.
    std::sort(std::begin(args), std::end(args), [](auto const& lhs, auto const& rhs) {
        return std::visit([](auto const& x, auto const& y) { return x.index < y.index; }, lhs, rhs);
    });

    // Check that all the indices are unique.
    assert_invariant(
            std::adjacent_find(args.begin(), args.end(), [](auto const& lhs, auto const& rhs) {
                return std::visit(
                        [](auto const& x, auto const& y) { return x.index == y.index; }, lhs, rhs);
            }) == args.end());

    for (const auto& a : builder.mArguments) {
        std::visit([&](auto&& arg) {
            arg.write(ss);
        }, a);
    }

    ss << "}";
    mShaderText = ss.str();
}

void MetalArgumentBuffer::destroy(MetalArgumentBuffer** argumentBuffer) {
    delete *argumentBuffer;
    *argumentBuffer = nullptr;
}

static bool isWhitespace(char c) {
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

bool MetalArgumentBuffer::replaceInShader(std::string& shader,
        const std::string& targetArgBufferName, const std::string& replacement) noexcept {
    // We make some assumptions here, e.g., that the MSL is well-formed and has no comments.
    // This algorithm isn't a full-fledged parser, and isn't foolproof. In particular, we can't tell
    // the difference between source code and comments. However, at this stage, the MSL should have
    // all comments stripped.

    // In order to do the replacement, we look for 4 key locations in the source shader.
    // s: the beginning of the 'struct' token
    // n: the beginning of the argument buffer name
    // b: the beginning of the structure block
    // e: the end of the argument buffer structure
    //
    // s      n               b e
    // struct targetArgBuffer { }

    // We only want to match the definition of the argument buffer, not any of its usages.
    // For example:
    // struct ArgBuffer { };                // this should match
    // void aFunction(ArgBuffer& args);     // this should not

    const auto argBufferNameLength = targetArgBufferName.length();

    // First, find n.
    size_t n = shader.find(targetArgBufferName);
    while (n != std::string::npos) {
        // Now, find b, the opening curly brace {.
        size_t b = shader.find('{', n);
        if (b == std::string::npos) {
            // If there's no { character in the rest of the shader, the arg buffer definition
            // definitely doesn't exit.
            return false;
        }

        // After the arg buffer name, ensure that only whitespace characters exist until b.
        if (!std::all_of(shader.begin() + n + argBufferNameLength, shader.begin() + b,
                    isWhitespace)) {
            // If there is a non-whitespace character, start over by looking for the next occurrence
            // of the arg buffer name.
            n = shader.find(targetArgBufferName, n + 1);
            continue;
        }

        // Now, we find s.
        size_t s = shader.rfind("struct", n);
        if (s == std::string::npos) {
            // If we can't find the "struct" keyword, it's not necessarily an error.
            // Start over and Look for the next occurrence of the arg buffer name.
            n = shader.find(targetArgBufferName, n + 1);
            continue;
        }

        // After the struct keyword, ensure that only whitespace characters exist until n.
        if (!std::all_of(shader.begin() + s + 6, shader.begin() + n, isWhitespace)) {
            // Look for the next occurrence of the arg buffer name.
            n = shader.find(targetArgBufferName, n + 1);
            continue;
        }

        // Now, we find e.
        size_t e = shader.find('}', n);
        if (e == std::string::npos) {
            // If there's no } character in the rest of the shader, the arg buffer definition
            // definitely doesn't exit.
            return false;
        }

        // Perform the replacement.
        shader.replace(s, e - s + 1, replacement);

        // Theoretically we could continue to find and replace other occurrences, but there should
        // only ever be a single definition of the argument buffer structure.
        return true;
    }

    return false;
}

} // namespace filamat
