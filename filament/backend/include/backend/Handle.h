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

#ifndef TNT_FILAMENT_BACKEND_HANDLE_H
#define TNT_FILAMENT_BACKEND_HANDLE_H

#if !defined(NDEBUG)
#include <utils/ostream.h>
#endif
#include <utils/debug.h>

#include <type_traits> // FIXME: STL headers are not allowed in public headers
#include <utility>

#include <stdint.h>

namespace filament::backend {

struct HwBufferObject;
struct HwFence;
struct HwIndexBuffer;
struct HwProgram;
struct HwRenderPrimitive;
struct HwRenderTarget;
struct HwStream;
struct HwSwapChain;
struct HwTexture;
struct HwTimerQuery;
struct HwVertexBufferInfo;
struct HwVertexBuffer;
struct HwDescriptorSetLayout;
struct HwDescriptorSet;

/*
 * A handle to a backend resource. HandleBase is for internal use only.
 * HandleBase *must* be a trivial for the purposes of calls, that is, it cannot have user-defined
 * copy or move constructors.
 */

//! \privatesection

class HandleBase {
public:
    using HandleId = uint32_t;
    static constexpr const HandleId nullid = HandleId{ UINT32_MAX };

    constexpr HandleBase() noexcept: object(nullid) {}

    // whether this Handle is initialized
    explicit operator bool() const noexcept { return object != nullid; }

    // clear the handle, this doesn't free associated resources
    void clear() noexcept { object = nullid; }

    // get this handle's handleId
    HandleId getId() const noexcept { return object; }

    // initialize a handle, for internal use only.
    explicit HandleBase(HandleId id) noexcept : object(id) {
        assert_invariant(object != nullid); // usually means an uninitialized handle is used
    }

protected:
    HandleBase(HandleBase const& rhs) noexcept = default;
    HandleBase& operator=(HandleBase const& rhs) noexcept = default;

    HandleBase(HandleBase&& rhs) noexcept
            : object(rhs.object) {
        rhs.object = nullid;
    }

    HandleBase& operator=(HandleBase&& rhs) noexcept {
        if (this != &rhs) {
            object = rhs.object;
            rhs.object = nullid;
        }
        return *this;
    }

private:
    HandleId object;
};

/**
 * Type-safe handle to backend resources
 * @tparam T Type of the resource
 */
template<typename T>
struct Handle : public HandleBase {

    Handle() noexcept = default;

    Handle(Handle const& rhs) noexcept = default;
    Handle(Handle&& rhs) noexcept = default;

    // Explicitly redefine copy/move assignment operators rather than just using default here.
    // Because it doesn't make a call to the parent's method automatically during the std::move
    // function call(https://en.cppreference.com/w/cpp/algorithm/move) in certain compilers like
    // NDK 25.1.8937393 and below (see b/371980551)
    Handle& operator=(Handle const& rhs) noexcept {
        HandleBase::operator=(rhs);
        return *this;
    }
    Handle& operator=(Handle&& rhs) noexcept {
        HandleBase::operator=(std::move(rhs));
        return *this;
    }

    explicit Handle(HandleId id) noexcept : HandleBase(id) { }

    // compare handles of the same type
    bool operator==(const Handle& rhs) const noexcept { return getId() == rhs.getId(); }
    bool operator!=(const Handle& rhs) const noexcept { return getId() != rhs.getId(); }
    bool operator<(const Handle& rhs) const noexcept { return getId() < rhs.getId(); }
    bool operator<=(const Handle& rhs) const noexcept { return getId() <= rhs.getId(); }
    bool operator>(const Handle& rhs) const noexcept { return getId() > rhs.getId(); }
    bool operator>=(const Handle& rhs) const noexcept { return getId() >= rhs.getId(); }

    // type-safe Handle cast
    template<typename B, typename = std::enable_if_t<std::is_base_of<T, B>::value> >
    Handle(Handle<B> const& base) noexcept : HandleBase(base) { } // NOLINT(hicpp-explicit-conversions,google-explicit-constructor)

private:
#if !defined(NDEBUG)
    template <typename U>
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const Handle<U>& h) noexcept;
#endif
};

// Types used by the command stream
// (we use this renaming because the macro-system doesn't deal well with "<" and ">")
using BufferObjectHandle        = Handle<HwBufferObject>;
using FenceHandle               = Handle<HwFence>;
using IndexBufferHandle         = Handle<HwIndexBuffer>;
using ProgramHandle             = Handle<HwProgram>;
using RenderPrimitiveHandle     = Handle<HwRenderPrimitive>;
using RenderTargetHandle        = Handle<HwRenderTarget>;
using StreamHandle              = Handle<HwStream>;
using SwapChainHandle           = Handle<HwSwapChain>;
using TextureHandle             = Handle<HwTexture>;
using TimerQueryHandle          = Handle<HwTimerQuery>;
using VertexBufferHandle        = Handle<HwVertexBuffer>;
using VertexBufferInfoHandle    = Handle<HwVertexBufferInfo>;
using DescriptorSetLayoutHandle = Handle<HwDescriptorSetLayout>;
using DescriptorSetHandle       = Handle<HwDescriptorSet>;

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_HANDLE_H
