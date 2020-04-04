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

#include <utils/compiler.h>
#include <utils/Log.h>

#include <assert.h>

namespace filament {
namespace backend {

struct HwFence;
struct HwIndexBuffer;
struct HwProgram;
struct HwRenderPrimitive;
struct HwRenderTarget;
struct HwSamplerGroup;
struct HwStream;
struct HwSwapChain;
struct HwSync;
struct HwTexture;
struct HwTimerQuery;
struct HwUniformBuffer;
struct HwVertexBuffer;

/*
 * A type handle to a h/w resource
 */

//! \privatesection

class HandleBase {
public:
    using HandleId = uint32_t;
    static constexpr const HandleId nullid = HandleId{ std::numeric_limits<HandleId>::max() };

    enum class no_init { };
    static constexpr no_init NO_INIT = { };

    constexpr HandleBase() noexcept : object(nullid) { }

    explicit HandleBase(no_init) noexcept { } // NOLINT

    explicit HandleBase(HandleId id) noexcept : object(id) {
        assert(object != nullid); // usually means an uninitialized handle is used
    }

    HandleBase(HandleBase const& rhs) noexcept = default;

    HandleBase& operator = (HandleBase const& rhs) noexcept = default;

    HandleBase(HandleBase&& rhs) noexcept : object(rhs.object) {
        rhs.object = nullid;
    }

    HandleBase& operator = (HandleBase&& rhs) noexcept {
        std::swap(object, rhs.object);
        return *this;
    }

    void clear() noexcept { object = nullid; }

    explicit operator bool() const noexcept { return object != nullid; }

    bool operator==(const HandleBase& rhs) const noexcept { return object == rhs.object; }
    bool operator!=(const HandleBase& rhs) const noexcept { return object != rhs.object; }

    // get this handle's handleId
    HandleId getId() const noexcept { return object; }

protected:
    HandleId object;
};

template <typename T>
struct Handle : public HandleBase {
    using HandleBase::HandleBase;

    template<typename B, typename = std::enable_if_t<std::is_base_of<T, B>::value> >
    Handle(Handle<B> const& base) noexcept
            : HandleBase(base) {
    }

private:
#if !defined(NDEBUG)
    template <typename U>
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const Handle<U>& h) noexcept;
#endif
};

// Types used by the command stream
// (we use this renaming because the macro-system doesn't deal well with "<" and ">")
using FenceHandle           = Handle<HwFence>;
using IndexBufferHandle     = Handle<HwIndexBuffer>;
using ProgramHandle         = Handle<HwProgram>;
using RenderPrimitiveHandle = Handle<HwRenderPrimitive>;
using RenderTargetHandle    = Handle<HwRenderTarget>;
using SamplerGroupHandle    = Handle<HwSamplerGroup>;
using StreamHandle          = Handle<HwStream>;
using SwapChainHandle       = Handle<HwSwapChain>;
using SyncHandle            = Handle<HwSync>;
using TextureHandle         = Handle<HwTexture>;
using TimerQueryHandle      = Handle<HwTimerQuery>;
using UniformBufferHandle   = Handle<HwUniformBuffer>;
using VertexBufferHandle    = Handle<HwVertexBuffer>;

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_HANDLE_H
