# Copyright 2020 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

DawnGenerator(
    SCRIPT "${Dawn_SOURCE_DIR}/generator/dawn_version_generator.py"
    PRINT_NAME "Dawn version based utilities"
    OUTPUT_HEADERS DAWN_VERSION_AUTOGEN_HEADERS
    EXTRA_PARAMETERS "--dawn-dir"
         "${Dawn_SOURCE_DIR}"
)

DawnGenerator(
    SCRIPT "${Dawn_SOURCE_DIR}/generator/dawn_gpu_info_generator.py"
    PRINT_NAME "Dawn GPU info utilities"
    OUTPUT_HEADERS DAWN_GPU_INFO_AUTOGEN_HEADERS
    OUTPUT_SOURCES DAWN_GPU_INFO_AUTOGEN_SOURCES
    EXTRA_PARAMETERS "--gpu-info-json"
         "${Dawn_SOURCE_DIR}/src/dawn/gpu_info.json"
)

set(private_headers
    "${DAWN_VERSION_AUTOGEN_HEADERS}"
    "${DAWN_GPU_INFO_AUTOGEN_HEADERS}"
    "AlignedAlloc.h"
    "Alloc.h"
    "Assert.h"
    "BitSetIterator.h"
    "Compiler.h"
    "Constants.h"
    "ContentLessObjectCache.h"
    "ContentLessObjectCacheable.h"
    "CoreFoundationRef.h"
    "DynamicLib.h"
    "egl_platform.h"
    "Enumerator.h"
    "FutureUtils.h"
    "GPUInfo.h"
    "HashUtils.h"
    "IOKitRef.h"
    "ityp_array.h"
    "ityp_bitset.h"
    "ityp_span.h"
    "ityp_stack_vec.h"
    "ityp_vector.h"
    "LinkedList.h"
    "Log.h"
    "MatchVariant.h"
    "Math.h"
    "Mutex.h"
    "MutexProtected.h"
    "NonCopyable.h"
    "NonMovable.h"
    "NSRef.h"
    "Numeric.h"
    "PlacementAllocated.h"
    "Platform.h"
    "Preprocessor.h"
    "Range.h"
    "Ref.h"
    "RefBase.h"
    "RefCounted.h"
    "RefCountedWithExternalCount.h"
    "Result.h"
    "Sha3.h"
    "SerialMap.h"
    "SerialQueue.h"
    "SerialStorage.h"
    "SlabAllocator.h"
    "StackAllocated.h"
    "StringViewUtils.h"
    "SystemUtils.h"
    "ThreadLocal.h"
    "TypedInteger.h"
    "TypeTraits.h"
    "UnderlyingType.h"
    "vulkan_platform.h"
    "WeakRef.h"
    "WeakRefSupport.h"
    "WGSLFeatureMapping.h"
    "xlib_with_undefs.h"
)

set(sources
    "${DAWN_GPU_INFO_AUTOGEN_SOURCES}"
    "AlignedAlloc.cpp"
    "Assert.cpp"
    "DynamicLib.cpp"
    "FutureUtils.cpp"
    "GPUInfo.cpp"
    "Log.cpp"
    "Math.cpp"
    "Mutex.cpp"
    "RefCounted.cpp"
    "Result.cpp"
    "Sha3.cpp"
    "SlabAllocator.cpp"
    "StringViewUtils.cpp"
    "SystemUtils.cpp"
    "ThreadLocal.cpp"
    "WeakRefSupport.cpp"
)

set(conditional_private_depends)

if (WIN32)
    list(APPEND headers
        "windows_with_undefs.h"
        "WindowsUtils.h"
    )
    list(APPEND sources
        "WindowsUtils.cpp"
    )
elseif(APPLE)
    list(APPEND headers
        "IOSurfaceUtils.h"
    )
    list(APPEND sources
        "IOSurfaceUtils.cpp"
        "SystemUtils_mac.mm"
    )
    list(APPEND conditional_private_depends
        "-framework Foundation"
        "-framework IOSurface"
    )
endif()


if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    find_library(log_lib log)
    list(APPEND conditional_private_depends
        ${log_lib}
        android)
endif ()

if (DAWN_ENABLE_VULKAN)
    list(APPEND conditional_private_depends
        Vulkan-Headers)
endif ()

list(APPEND conditional_private_depends
    ${CMAKE_DL_LIBS})

dawn_add_library(
    dawn_common
    ENABLE_EMSCRIPTEN
    UTILITY_TARGET dawn_internal_config
    PRIVATE_HEADERS
        ${private_headers}
    SOURCES
        ${sources}
    DEPENDS
        webgpu_c
        webgpu_cpp
    PRIVATE_DEPENDS
        absl::inlined_vector
        absl::no_destructor
        dawn::partition_alloc
        dawn_shared_utils
        ${conditional_private_depends}
)
