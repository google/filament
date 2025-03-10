// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_VULKAN_PLATFORM_H_
#define SRC_DAWN_COMMON_VULKAN_PLATFORM_H_

#if !defined(DAWN_ENABLE_BACKEND_VULKAN)
#error "vulkan_platform.h included without the Vulkan backend enabled"
#endif
#if defined(VULKAN_CORE_H_)
#error "vulkan.h included before vulkan_platform.h"
#endif

#include <cstddef>
#include <cstdint>

#include "dawn/common/Platform.h"

// vulkan.h defines non-dispatchable handles to opaque pointers on 64bit architectures and uint64_t
// on 32bit architectures. This causes a problem in 32bit where the handles cannot be used to
// distinguish between overloads of the same function.
// Change the definition of non-dispatchable handles to be opaque structures containing a uint64_t
// and overload the comparison operators between themselves and VK_NULL_HANDLE (which will be
// redefined to be nullptr). This keeps the type-safety of having the handles be different types
// (like vulkan.h on 64 bit) but makes sure the types are different on 32 bit architectures.

#if DAWN_PLATFORM_IS(64_BIT)
#define DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object) using object = struct object##_T*;
#elif DAWN_PLATFORM_IS(32_BIT)
#define DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object) using object = uint64_t;
#else
#error "Unsupported platform"
#endif

// Define a placeholder Vulkan handle for use before we include vulkan.h
DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(VkSomeHandle)

// Find out the alignment of native handles. Logically we would use alignof(VkSomeHandleNative) so
// why bother with the wrapper struct? It turns out that on Linux Intel x86 alignof(uint64_t) is 8
// but alignof(struct{uint64_t a;}) is 4. This is because this Intel ABI doesn't say anything about
// double-word alignment so for historical reasons compilers violated the standard and use an
// alignment of 4 for uint64_t (and double) inside structures.
// See https://stackoverflow.com/questions/44877185
// One way to get the alignment inside structures of a type is to look at the alignment of it
// wrapped in a structure. Hence VkSameHandleNativeWrappe

namespace dawn::native::vulkan {

namespace detail {
template <typename T>
struct WrapperStruct {
    T member;
};

template <typename T>
static constexpr size_t AlignOfInStruct = alignof(WrapperStruct<T>);

static constexpr size_t kNativeVkHandleAlignment = AlignOfInStruct<VkSomeHandle>;
static constexpr size_t kUint64Alignment = AlignOfInStruct<uint64_t>;

// Simple handle types that supports "nullptr_t" as a 0 value.
template <typename Tag, typename HandleType>
class alignas(detail::kNativeVkHandleAlignment) VkHandle {
  public:
    // Default constructor and assigning of VK_NULL_HANDLE
    VkHandle() = default;
    VkHandle(std::nullptr_t) {}

    // Use default copy constructor/assignment
    VkHandle(const VkHandle<Tag, HandleType>& other) = default;
    VkHandle& operator=(const VkHandle<Tag, HandleType>&) = default;

    // Comparisons between handles
    bool operator==(VkHandle<Tag, HandleType> other) const { return mHandle == other.mHandle; }
    bool operator!=(VkHandle<Tag, HandleType> other) const { return mHandle != other.mHandle; }

    // Comparisons between handles and VK_NULL_HANDLE
    bool operator==(std::nullptr_t) const { return mHandle == 0; }
    bool operator!=(std::nullptr_t) const { return mHandle != 0; }

    // Implicit conversion to real Vulkan types.
    operator HandleType() const { return GetHandle(); }

    const HandleType& GetHandle() const { return mHandle; }

    HandleType& operator*() { return mHandle; }

    static VkHandle<Tag, HandleType> CreateFromHandle(HandleType handle) {
        return VkHandle{handle};
    }

  private:
    explicit VkHandle(HandleType handle) : mHandle(handle) {}

    HandleType mHandle = 0;
};
}  // namespace detail

template <typename Tag, typename HandleType>
HandleType* AsVkArray(detail::VkHandle<Tag, HandleType>* handle) {
    return reinterpret_cast<HandleType*>(handle);
}

}  // namespace dawn::native::vulkan

#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)                       \
    DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object)                  \
    namespace dawn::native::vulkan {                                    \
    using object = detail::VkHandle<struct VkTag##object, ::object>;    \
    static_assert(sizeof(object) == sizeof(uint64_t));                  \
    static_assert(alignof(object) == detail::kUint64Alignment);         \
    static_assert(sizeof(object) == sizeof(::object));                  \
    static_assert(alignof(object) == detail::kNativeVkHandleAlignment); \
    }  // namespace dawn::native::vulkan

// Import additional parts of Vulkan that are supported on our architecture and preemptively include
// headers that vulkan.h includes that we have "undefs" for. Note that some of the VK_USE_PLATFORM_*
// defines are defined already in the Vulkan-Header BUILD.gn, but are needed when building with
// CMake, hence they cannot be removed at the moment.
#if DAWN_PLATFORM_IS(WINDOWS)
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "dawn/common/windows_with_undefs.h"
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if defined(DAWN_USE_X11)
#define VK_USE_PLATFORM_XLIB_KHR
#ifndef VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include "dawn/common/xlib_with_undefs.h"
#endif  // defined(DAWN_USE_X11)

#if defined(DAWN_USE_WAYLAND)
#ifndef VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#endif
#endif  // defined(DAWN_USE_WAYLAND)

#if defined(DAWN_ENABLE_BACKEND_METAL)
#ifndef VK_USE_PLATFORM_METAL_EXT
#define VK_USE_PLATFORM_METAL_EXT
#endif
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if DAWN_PLATFORM_IS(ANDROID)
#ifndef VK_USE_PLATFORM_ANDROID_KHR
#define VK_USE_PLATFORM_ANDROID_KHR
#endif
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if DAWN_PLATFORM_IS(FUCHSIA)
#ifndef VK_USE_PLATFORM_FUCHSIA
#define VK_USE_PLATFORM_FUCHSIA
#endif
#endif  // DAWN_PLATFORM_IS(FUCHSIA)

// The actual inclusion of vulkan.h!
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

// Redefine VK_NULL_HANDLE for better type safety where possible.
#undef VK_NULL_HANDLE
static constexpr std::nullptr_t VK_NULL_HANDLE = nullptr;

#endif  // SRC_DAWN_COMMON_VULKAN_PLATFORM_H_
