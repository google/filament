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

#include <utils/compiler.h>
#include <utils/CallStack.h>
#include <utils/ostream.h>

#ifndef NDEBUG
#   include <utils/CString.h>
#   include <string_view>
#endif

#include <stddef.h>

using namespace utils;

namespace filament::backend {

#ifndef NDEBUG

static char const * const kOurNamespace = "filament::backend::";

// removes all occurrences of "what" from "str"
UTILS_NOINLINE
static CString& removeAll(CString& str, const std::string_view what) noexcept {
    if (!what.empty()) {
        const CString empty;
        size_t pos = 0;
        while ((pos = std::string_view{ str.data(), str.size() }.find(what, pos)) != std::string_view::npos) {
            str.replace(pos, what.length(), empty);
        }
    }
    return str;
}

template <typename T>
UTILS_NOINLINE
static io::ostream& logHandle(io::ostream& out, CString& typeName, T id) noexcept {
    return out << removeAll(typeName, kOurNamespace) << " @ " << id;
}

template <typename T>
io::ostream& operator<<(io::ostream& out, const Handle<T>& h) noexcept {
    CString s{ CallStack::typeName<Handle<T>>() };
    return logHandle(out, s, h.getId());
}

// Explicit Instantiation of the streaming operators (so they're not inlined)
template io::ostream& operator<<(io::ostream& out, const Handle<HwVertexBuffer>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwIndexBuffer>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwRenderPrimitive>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwProgram>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwTexture>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwRenderTarget>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwFence>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwSwapChain>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwStream>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwTimerQuery>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwBufferObject>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwDescriptorSet>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwDescriptorSetLayout>& h) noexcept;
template io::ostream& operator<<(io::ostream& out, const Handle<HwVertexBufferInfo>& h) noexcept;

#endif

} // namespace filament::backend
