/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <backend/Handle.h>

#ifndef NDEBUG
#   include <string>
#endif

#include <utils/CallStack.h>

using namespace utils;

namespace filament {
namespace backend {

#ifndef NDEBUG

static char const * const kOurNamespace = "filament::";

// removes all occurrences of "what" from "str"
UTILS_NOINLINE
static std::string& removeAll(std::string& str, const std::string& what) noexcept {
    if (!what.empty()) {
        const std::string empty;
        size_t pos = 0;
        while ((pos = str.find(what, pos)) != std::string::npos) {
            str.replace(pos, what.length(), empty);
        }
    }
    return str;
}

template <typename T>
UTILS_NOINLINE
static io::ostream& logHandle(io::ostream& out, std::string& typeName, T id) noexcept {
    return out << removeAll(typeName, kOurNamespace) << " @ " << id;
}

template <typename T>
io::ostream& operator<<(io::ostream& out, const Handle<T>& h) noexcept {
    std::string s(CallStack::typeName<Handle<T>>().c_str());
    return logHandle(out, s, h.object);
}

// Explicit Instantiation of the streaming operators (so they're not inlined)
template io::ostream& operator<<(io::ostream& out, const Handle<HwVertexBuffer>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwIndexBuffer>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwRenderPrimitive>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwProgram>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwSamplerGroup>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwUniformBuffer>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwTexture>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwRenderTarget>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwFence>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwSwapChain>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwStream>& h) noexcept;

#endif

} // namespace backend
} // namespace filament
