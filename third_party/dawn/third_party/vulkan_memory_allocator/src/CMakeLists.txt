if (NOT WIN32)
    message(STATUS "VmaSample application is only supported on Windows")
    return()
endif()

option(VMA_STATIC_VULKAN_FUNCTIONS "Link statically with Vulkan API" ON)
option(VMA_DYNAMIC_VULKAN_FUNCTIONS "Fetch pointers to Vulkan functions internally (no static linking)" OFF)
option(VMA_DEBUG_ALWAYS_DEDICATED_MEMORY "Every allocation will have its own memory block" OFF)
option(VMA_DEBUG_INITIALIZE_ALLOCATIONS "Automatically fill new allocations and destroyed allocations with some bit pattern" OFF)
option(VMA_DEBUG_GLOBAL_MUTEX "Enable single mutex protecting all entry calls to the library" OFF)
option(VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT "Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error" OFF)

message(STATUS "VMA_STATIC_VULKAN_FUNCTIONS = ${VMA_STATIC_VULKAN_FUNCTIONS}")
message(STATUS "VMA_DYNAMIC_VULKAN_FUNCTIONS = ${VMA_DYNAMIC_VULKAN_FUNCTIONS}")
message(STATUS "VMA_DEBUG_ALWAYS_DEDICATED_MEMORY = ${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}")
message(STATUS "VMA_DEBUG_INITIALIZE_ALLOCATIONS = ${VMA_DEBUG_INITIALIZE_ALLOCATIONS}")
message(STATUS "VMA_DEBUG_GLOBAL_MUTEX = ${VMA_DEBUG_GLOBAL_MUTEX}")
message(STATUS "VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT = ${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)

add_executable(VmaSample)
target_sources(VmaSample PRIVATE
    Common.cpp
    Common.h
    SparseBindingTest.cpp
    SparseBindingTest.h
    Tests.cpp
    Tests.h
    VmaUsage.cpp
    VmaUsage.h
    VulkanSample.cpp
    ../include/vk_mem_alloc.h
)

# Only link to Vulkan library if static linking is used, but always add Vulkan headers directory
if(VMA_STATIC_VULKAN_FUNCTIONS)
    target_link_libraries(VmaSample PUBLIC Vulkan::Vulkan)
else()
    target_link_libraries(VmaSample PUBLIC Vulkan::Headers)
endif()

target_link_libraries(VmaSample PRIVATE GPUOpen::VulkanMemoryAllocator)

target_compile_definitions(VmaSample PUBLIC
    VMA_STATIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_STATIC_VULKAN_FUNCTIONS}>
    VMA_DYNAMIC_VULKAN_FUNCTIONS=$<BOOL:${VMA_DYNAMIC_VULKAN_FUNCTIONS}>
    VMA_DEBUG_ALWAYS_DEDICATED_MEMORY=$<BOOL:${VMA_DEBUG_ALWAYS_DEDICATED_MEMORY}>
    VMA_DEBUG_INITIALIZE_ALLOCATIONS=$<BOOL:${VMA_DEBUG_INITIALIZE_ALLOCATIONS}>
    VMA_DEBUG_GLOBAL_MUTEX=$<BOOL:${VMA_DEBUG_GLOBAL_MUTEX}>
    VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=$<BOOL:${VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT}>
)

# Provides MSVC users nicer debugging support
target_sources(VmaSample PRIVATE vk_mem_alloc.natvis)

add_subdirectory(Shaders)
add_dependencies(VmaSample VmaSampleShaders)

# Use Unicode instead of multibyte set
add_compile_definitions(UNICODE _UNICODE)

# Add C++ warnings and security checks
add_compile_options(/permissive- /sdl /W3)

# Set VmaSample as startup project
set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "VmaSample")

set_target_properties(VmaSample PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
