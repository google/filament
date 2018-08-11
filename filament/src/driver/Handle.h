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

#ifndef TNT_FILAMENT_DRIVER_HANDLE_H
#define TNT_FILAMENT_DRIVER_HANDLE_H

#include <assert.h>

#include <algorithm>
#include <limits>
#include <type_traits>

#include <utils/compiler.h>
#include <utils/Log.h>

namespace filament {

struct HwBase;
struct HwVertexBuffer;
struct HwFence;
struct HwIndexBuffer;
struct HwProgram;
struct HwRenderPrimitive;
struct HwRenderTarget;
struct HwSamplerBuffer;
struct HwTexture;
struct HwUniformBuffer;
struct HwSwapChain;
struct HwStream;

/*
 * A type handle to a h/w resource
 */

class HandleBase {
public:
    using HandleId = uint32_t;
    static constexpr const HandleId nullid = HandleId{ std::numeric_limits<HandleId>::max() };

    enum class no_init { };
    static constexpr no_init NO_INIT = { };

    HandleBase() noexcept : object(nullid) { }

    explicit HandleBase(no_init) noexcept { } // NOLINT

    explicit HandleBase(HandleId id) noexcept : object(id) {
        assert(object != nullid); // usually means an uninitialized handle is used
    }

    HandleBase(HandleBase const& rhs) noexcept = default;
    HandleBase& operator = (HandleBase const& rhs) noexcept = default;

#ifndef NDEBUG
    // implement move ctor and copy operator for safety
    HandleBase(HandleBase&& rhs) noexcept : object(nullid) {
        std::swap(object, rhs.object);
    }
    HandleBase& operator = (HandleBase&& rhs) noexcept {
        std::swap(object, rhs.object);
        return *this;
    }
#endif

    void clear() noexcept { object = nullid; }

    explicit operator bool() const noexcept { return object != nullid; }

    bool operator==(const HandleBase& rhs) noexcept { return object == rhs.object; }
    bool operator!=(const HandleBase& rhs) noexcept { return object != rhs.object; }

    // get this handle's handleId
    HandleId getId() const noexcept { return object; }

protected:
    HandleId object;
};

template <typename T>
struct Handle : public HandleBase {
    using HandleBase::HandleBase;

private:
#if !defined(NDEBUG)
    template <typename U>
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const Handle<U>& h) noexcept;
#endif
};


} // namespace filament

#endif // TNT_FILAMENT_DRIVER_HANDLE_H
