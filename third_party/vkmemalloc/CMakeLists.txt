cmake_minimum_required(VERSION 3.9)

project(VulkanMemoryAllocator)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})

# VulkanMemoryAllocator contains an sample application which is not built by default
option(VMA_BUILD_SAMPLE "Build VulkanMemoryAllocator sample application" OFF)
option(VMA_BUILD_SAMPLE_SHADERS "Build VulkanMemoryAllocator sample application's shaders" OFF)

message(STATUS "VMA_BUILD_SAMPLE = ${VMA_BUILD_SAMPLE}")
message(STATUS "VMA_BUILD_SAMPLE_SHADERS = ${VMA_BUILD_SAMPLE_SHADERS}")

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

if(VMA_BUILD_SAMPLE)
    set(VMA_BUILD_SAMPLE_SHADERS ON)
endif(VMA_BUILD_SAMPLE)

find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

if(BUILD_DOCUMENTATION)
    if(DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else()
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif()
endif()

add_subdirectory(src)
